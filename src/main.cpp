#include <ncurses.h>

#include "editor.h"

#define ctrl(x)           ((x) & 0x1f)

bool quit = false;
int view_start_line = 0;

void process_keys(Editor* ed)
{
    int key = getch();

    if (isprint(key) || key == '\t' || key == '\n')
        ed->insert(key);
    else
    {
        switch (key)
        {
        case ctrl('q'):         quit = true;                return;
        case ctrl('s'):         ed->write();                break;
        case ctrl('x'):         ed->cut();                  break;
        case ctrl('c'):         ed->copy();                 break;
        case ctrl('v'):         ed->paste();                break;
        case ctrl('a'):         ed->select_all();           break;

        case KEY_DC:            ed->erase();                break;
        case KEY_BACKSPACE:     ed->backspace();            break;

        case KEY_RIGHT:         ed->move_right();           break;
        case KEY_LEFT:          ed->move_left();            break;
        case KEY_UP:            ed->move_up();              break;
        case KEY_DOWN:          ed->move_down();            break;

        case KEY_HOME:          ed->move_to_line_start();   break;
        case KEY_END:           ed->move_to_line_end();     break;

        case KEY_SRIGHT:        ed->move_right(true);       break;
        case KEY_SLEFT:         ed->move_left(true);        break;
        case KEY_SR:            ed->move_up(true);          break;
        case KEY_SF:            ed->move_down(true);        break;

        case KEY_SHOME:        ed->move_to_line_start(true);    break;
        case KEY_SEND:         ed->move_to_line_end(true);      break;
        }
    }
}

bool in_selection(int line, int col, Editor* ed)
{
    if (!ed->is_selection)
        return false;

    int start_line = ed->line;
    int start_col = ed->col;

    int end_line = ed->sel_line;
    int end_col = ed->sel_col;

    if (start_line > end_line || (start_line == end_line && start_col > end_col))
    {
        int t = start_line;
        start_line = end_line;
        end_line = t;

        t = start_col;
        start_col = end_col;
        end_col = t;
    }

    if (line < start_line || line > end_line)
        return false;

    if (line == start_line && col < start_col)
        return false;

    if (line == end_line && col >= end_col)
        return false;

    return true;
}

std::string line_number(int i, int min_size)
{
    std::string num = std::to_string(i);

    int padding = min_size - num.size();

    if (padding > 0)
        return std::string(padding, ' ') + num + "  ";

    return num + "  ";
}

void draw(Editor* ed)
{
    curs_set(0);

    int height = getmaxy(stdscr) - 1;
    int width = getmaxx(stdscr);

    int top_margin = ed->line - 4;
    int bottom_margin = ed->line + 4;

    if (top_margin < 0)
        top_margin = 0;

    if (bottom_margin > ed->text.size() - 1)
        bottom_margin = ed->text.size() - 1;

    if (view_start_line > top_margin)
        view_start_line = top_margin;
    else if (view_start_line < bottom_margin - height + 1)
        view_start_line = bottom_margin - height + 1;

    int min_size = std::to_string(ed->text.size()).size();

    if (min_size < 4)
        min_size = 4;

    for (int i = 0; i < height; i++)
    {
        move(i, 0);

        int line = view_start_line + i;

        if (line < ed->text.size())
        {
            std::string prefix = line_number(line + 1, min_size);

            attr_t attr = COLOR_PAIR(line == ed->line ? 2 : 1) | A_BOLD;

            attron(attr);
            printw("%s", prefix.c_str());
            attroff(attr);

            int col;

            for (col = 0; col < ed->text[line].size(); col++)
            {
                attr = in_selection(line, col, ed) ? A_REVERSE : 0;

                attron(attr);
                printw("%c", ed->text[line][col]);
                attroff(attr);
            }

            if (line != ed->text.size() - 1)
            {
                attr = in_selection(line, col, ed) ? A_REVERSE : 0;

                attron(attr);
                printw(" ");
                attroff(attr);
            }
        }
        else
        {            
            std::string prefix = std::string(min_size - 1, ' ') + "~  ";
            
            attron(COLOR_PAIR(1));
            printw("%s", prefix.c_str());
            attroff(COLOR_PAIR(1));
        }

        clrtoeol();
    }

    move(height, 0);

    attron(COLOR_PAIR(3));

    for (int i = 0; i < width; i++)
        printw(" ");

    mvprintw(height, 4, "%s", ed->filename);

    if (ed->dirty)
        printw("*");

    printw("    %llu bytes", ed->get_size());

    mvprintw(height, width - 16, "%d,%d", ed->line + 1, ed->col + 1);

    attroff(COLOR_PAIR(3));

    move(ed->line - view_start_line, ed->col + min_size + 2);

    refresh();

    curs_set(1);
}

int main(int argc, char** argv)
{
    if (argc != 2)
    {
        fprintf(stderr, "%s: no input file\n", argv[0]);
        return 1;
    }

    Editor ed(argv[1]);

    initscr();
    cbreak();
    raw();
    noecho();
    keypad(stdscr, true);

    start_color();
    use_default_colors();
    init_pair(1, COLOR_BLUE, -1);
    init_pair(2, COLOR_CYAN, -1);
    init_pair(3, -1, COLOR_BLUE);

    while (!quit)
    {
        draw(&ed);
        process_keys(&ed);
    }

    endwin();

    return 0;
}
