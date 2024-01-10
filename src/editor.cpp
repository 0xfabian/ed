#include "editor.h"

Editor::Editor(char* _filename)
{
    filename = _filename;
    file = fopen(filename, "r+");

    line = 0;
    col = 0;
    target_col = 0;
    vertical_move = false;

    is_selection = false;
    sel_line = 0;
    sel_col = 0;

    text.push_back("");

    if (file)
    {
        char c;

        while (fread(&c, 1, 1, file))
            insert(c);
    }

    dirty = !file;
}

Editor::~Editor()
{
    if (file)
        fclose(file);
}

bool Editor::write()
{
    if (!file)
        file = fopen(filename, "w+");
    else
        file = freopen(nullptr, "w+", file);

    if (!file)
        return false;

    for (int line = 0; line < text.size(); line++)
    {
        fwrite(text[line].c_str(), 1, text[line].size(), file);

        if (line != text.size() - 1)
            fputc('\n', file);
    }

    dirty = false;

    return true;
}

void Editor::insert(char c)
{
    dirty = true;

    erase_selection();

    if (c == '\t')
    {
        int spaces = 4 - col % 4;

        for (int i = 0; i < spaces; i++)
        {
            text[line].insert(text[line].begin() + col, ' ');
            col++;
        }

    }
    else if (c == '\n')
    {
        std::string s1 = text[line].substr(0, col);
        std::string s2 = text[line].substr(col);

        text[line] = s1;

        line++;
        col = 0;

        text.insert(text.begin() + line, "");

        text[line] = s2;
    }
    else
    {
        text[line].insert(text[line].begin() + col, c);
        col++;
    }
}

void Editor::erase()
{
    if (is_selection)
    {
        erase_selection();
        return;
    }

    if (col == text[line].size())
    {
        if (line != text.size() - 1)
        {
            text[line] += text[line + 1];
            text.erase(text.begin() + line + 1);

            dirty = true;
        }
    }
    else
    {
        text[line].erase(text[line].begin() + col);

        dirty = true;
    }
}

void Editor::backspace()
{
    if (is_selection)
    {
        erase_selection();
        return;
    }

    if (col != 0 || line != 0)
    {
        move_left();
        erase();
    }
}

void Editor::move_right(bool shift)
{
    bool was_selection = is_selection;

    if (shift)
        start_selection();
    else
        is_selection = false;

    vertical_move = false;

    if (col == text[line].size())
    {
        if (line != text.size() - 1)
        {
            line++;
            col = 0;
        }
        else
        {
            if (!was_selection)
                is_selection = false;
        }
    }
    else
        col++;
}

void Editor::move_left(bool shift)
{
    bool was_selection = is_selection;

    if (shift)
        start_selection();
    else
        is_selection = false;

    vertical_move = false;

    if (col == 0)
    {
        if (line > 0)
        {
            line--;
            col = text[line].size();
        }
        else
        {
            if (!was_selection)
                is_selection = false;
        }
    }
    else
        col--;
}

void Editor::move_up(bool shift)
{
    bool was_selection = is_selection;

    if (shift)
        start_selection();
    else
        is_selection = false;

    if (!vertical_move)
    {
        vertical_move = true;
        target_col = col;
    }

    if (line > 0)
    {
        line--;
        col = target_col;

        if (col > text[line].size())
            col = text[line].size();
    }
    else
    {
        if (!was_selection)
            is_selection = false;
    }
}

void Editor::move_down(bool shift)
{
    bool was_selection = is_selection;

    if (shift)
        start_selection();
    else
        is_selection = false;

    if (!vertical_move)
    {
        vertical_move = true;
        target_col = col;
    }

    if (line != text.size() - 1)
    {
        line++;
        col = target_col;

        if (col > text[line].size())
            col = text[line].size();
    }
    else
    {
        if (!was_selection)
            is_selection = false;
    }
}

void Editor::move_to_line_start(bool shift)
{
    if (col > 0)
    {
        if (shift)
            start_selection();
        else
            is_selection = false;

        col = 0;
    }
}

void Editor::move_to_line_end(bool shift)
{
    if (col < text[line].size())
    {
        if (shift)
            start_selection();
        else
            is_selection = false;

        col = text[line].size();
    }
}

void Editor::start_selection()
{
    if (!is_selection)
    {
        is_selection = true;
        sel_line = line;
        sel_col = col;
    }
}

void Editor::select_all()
{
    if (text.size() == 1 && text[0].size() == 0)
        return;

    is_selection = true;
    sel_line = 0;
    sel_col = 0;

    line = text.size() - 1;
    col = text[line].size();
}

void Editor::erase_selection()
{
    if (!is_selection)
        return;

    int start_line = line;
    int start_col = col;

    int end_line = sel_line;
    int end_col = sel_col;

    if (start_line > end_line || (start_line == end_line && start_col > end_col))
    {
        int t = start_line;
        start_line = end_line;
        end_line = t;

        t = start_col;
        start_col = end_col;
        end_col = t;
    }

    std::string res = text[start_line].substr(0, start_col) + text[end_line].substr(end_col);

    text.erase(text.begin() + start_line, text.begin() + end_line + 1);
    text.insert(text.begin() + start_line, res);

    is_selection = false;

    line = start_line;
    col = start_col;

    dirty = true;
}

std::string Editor::get_selection()
{
    std::string res = "";

    if (is_selection)
    {
        int start_line = line;
        int start_col = col;

        int end_line = sel_line;
        int end_col = sel_col;

        if (start_line > end_line || (start_line == end_line && start_col > end_col))
        {
            int t = start_line;
            start_line = end_line;
            end_line = t;

            t = start_col;
            start_col = end_col;
            end_col = t;
        }

        int line = start_line;
        int col = start_col;

        while (line != end_line || col != end_col)
        {
            if (col == text[line].size())
            {
                if (line != text.size() - 1)
                {
                    res += '\n';
                    line++;
                    col = 0;
                }
            }
            else
            {
                res += text[line][col];
                col++;
            }
        }
    }

    return res;
}

void Editor::cut()
{
    if (is_selection)
    {
        copied_text = get_selection();

        erase_selection();
    }
    else
    {
        copied_text = text[line];

        if (text.size() == 1)
        {
            text[0] = "";
            col = 0;
        }
        else
        {
            text.erase(text.begin() + line);

            if (line == text.size())
            {
                line--;
                col = text[line].size();
            }
            else
                col = 0;
        }

        dirty = true;
    }
}

void Editor::copy()
{
    if (is_selection)
        copied_text = get_selection();
    else
        copied_text = text[line];
}

void Editor::paste()
{
    for (char c : copied_text)
        insert(c);
}

size_t Editor::get_size()
{
    size_t ret = 0;

    for (int line = 0; line < text.size(); line++)
    {
        ret += text[line].size();

        if (line != text.size() - 1)
            ret++;
    }

    return ret;
}
