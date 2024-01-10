#pragma once

#include <cstdio>
#include <string>
#include <vector>

struct Editor
{
    size_t line;
    size_t col;
    size_t target_col;
    bool vertical_move;

    bool is_selection;
    size_t sel_line;
    size_t sel_col;

    char* filename;
    FILE* file;
    bool dirty;

    std::string copied_text;
    std::vector<std::string> text;

    Editor(char* _filename);
    ~Editor();

    bool write();

    void insert(char c);
    void erase();
    void backspace();

    void move_right(bool shift = false);
    void move_left(bool shift = false);
    void move_up(bool shift = false);
    void move_down(bool shift = false);
    void move_to_line_start(bool shift = false);
    void move_to_line_end(bool shift = false);

    void start_selection();
    void select_all();
    void erase_selection();
    std::string get_selection();

    void cut();
    void copy();
    void paste();

    size_t get_size();
};
