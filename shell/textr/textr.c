#include "textr.h"

#include "../../mylibs/kernelio.h"
#include "../../fs/file_system.h"
#include "../../memory_management/heap.h"
#include "load_write_file.h"

char ** vid_memory;
uint16_t allocated_lines;

uint16_t line_pos;
uint16_t column_pos;

uint16_t top_line;

void textr_main(const char *filename);
void textr_init();

void textr_start(const char *filename)
{       
        if (filename == NULL || filename[0] == '\0') {
                kprint_str("Filename cannot be empty");
                return;
        }
        textr_init();
        load_file(filename, vid_memory);
        textr_main(filename);
}
void textr_init()
{
        line_pos = 0;
        column_pos = 0;
        top_line = 0;

        clear_screen();
        set_cursor_pos(0, 0);

        vid_memory = kcalloc(2048, 0);     //must be fine. in future it will be 1 block
        if (vid_memory == NULL) {
                kprint_str("Memory allocation failed");
                kgetc();
                return;
        }

        //lines*2 just for cool moving down and up
        for (uint32_t i = 0; i < LINES*2; i++) {
                vid_memory[i] = kcalloc(COLUMNS_IN_LINE * sizeof(char), 0);
        }
        allocated_lines = LINES-1;
}
void* create_new_line()
{
        vid_memory[allocated_lines++] = kcalloc(COLUMNS_IN_LINE * sizeof(char), 0);
        return vid_memory[allocated_lines-1];
}

void add_char_in_vid_mem(const char character)
{
        if (line_pos < LINES && column_pos < COLUMNS_IN_LINE) {
                vid_memory[line_pos][column_pos] = character;
        }
}

void print_line(uint16_t screen_line_pos, uint16_t vid_mem_line_pos)
{
        set_cursor_pos_not_visible(screen_line_pos, 0);
        if (screen_line_pos >= LINES) return;
        
        for (size_t i = 0; i < COLUMNS_IN_LINE; i++) {
                if (vid_memory[vid_mem_line_pos][i] == '\n') break;
                kprint_char_no_ref_cursor(vid_memory[vid_mem_line_pos][i]);
        }
}

uint16_t find_pos_of_last_symbol_in_line(uint16_t input_line_pos, uint16_t max_index);
void make_real_line(uint16_t line);
void delete_line(uint16_t line);
void back_space()
{
        if (column_pos == 0 && line_pos == 0) {
                return;
        }
        if(column_pos == 0 && vid_memory[line_pos-1][0] == '\n'){
                delete_line(line_pos-1);
                line_pos--;
                set_cursor_pos(line_pos-top_line, column_pos);
                return;
        }
        if(column_pos == 0 && vid_memory[line_pos][1] == 0){
                delete_line(line_pos);
        }

        if (column_pos == 0) {
                line_pos--;
                column_pos = find_pos_of_last_symbol_in_line(line_pos, COLUMNS_IN_LINE-1);
                column_pos++;
        }
        column_pos--;

        vid_memory[line_pos][column_pos] = 0;
        set_cursor_pos(line_pos-top_line, column_pos);
        kprint_char_no_ref_cursor(' ');
        set_cursor_pos(line_pos-top_line, column_pos);
        make_real_line(line_pos);
}

void clear_line(uint16_t line);

//actually, we cant delete line in memory cuz i didnt make free func yet ;)
//we just clearing it and teleport that line to the end
void delete_line(uint16_t line)
{
        char* line_ptr = vid_memory[line];
        clear_line(line);

        for (size_t i = line; i < allocated_lines; i++){
                vid_memory[i] = vid_memory[i+1];
        }
        vid_memory[allocated_lines] = line_ptr;
}

void clear_line(uint16_t line)
{
        for (uint32_t i = 0; i < COLUMNS_IN_LINE; i++){
                vid_memory[line][i] = 0;
        }
        
}
// real line means that from first to last char we dont have chars that = 0
void make_real_line(uint16_t line) {
    uint16_t j = 0;
    for (uint16_t i = 0; i < COLUMNS_IN_LINE; i++) {
        if (vid_memory[line][i] != 0)
            vid_memory[line][j++] = vid_memory[line][i];
    }
    while (j < COLUMNS_IN_LINE)
        vid_memory[line][j++] = 0;
}


//last symbol is '\n' everytime 
uint16_t find_pos_of_last_symbol_in_line(uint16_t input_line_pos, uint16_t max_index)
{       
        uint32_t out_column_pos = 0;
                
        while (out_column_pos < max_index && vid_memory[input_line_pos][out_column_pos+1] != 0 ) {
                out_column_pos++;
        }
        return out_column_pos;
}
void move_lines_down(uint16_t from_line);
void new_line()
{
    // Ensure next line exists
    if (line_pos + 1 >= allocated_lines) {
        create_new_line();
    }

    // Shift lines down if next line isn't empty
    if (vid_memory[line_pos+1][0] != 0) {
        move_lines_down(line_pos);
    }

    // Check if content exists after cursor
    bool hasContent = false;
    for (uint16_t i = column_pos; i < COLUMNS_IN_LINE; i++) {
        if (vid_memory[line_pos][i] != 0) {
            hasContent = true;
            break;
        }
    }

    // Split content if needed
    if (hasContent) {
        uint16_t j = 0;
        for (uint16_t i = column_pos; i < COLUMNS_IN_LINE; i++) {
            if (j < COLUMNS_IN_LINE) {
                vid_memory[line_pos+1][j] = vid_memory[line_pos][i];
            }
            vid_memory[line_pos][i] = 0;
            j++;
        }
    }

    // Terminate current line
    vid_memory[line_pos][column_pos] = '\n';
    
    // Move to next line
    line_pos++;
    column_pos = 0;
    set_cursor_pos(line_pos - top_line, column_pos);
}
void move_lines_down(uint16_t from_line)
{
    void* new_line_ptr = create_new_line();  // Creates at end of array

    // Shift lines down (starting from the end)
    for (int i = allocated_lines - 1; i > from_line + 1; i--) {
        vid_memory[i] = vid_memory[i-1];
    }
    
    // Insert new line
    vid_memory[from_line + 1] = new_line_ptr;
}

void move_to_right_from_index(uint16_t line, uint16_t column_index);
void print_char(char character)
{
        if (column_pos >= COLUMNS_IN_LINE) {
                new_line();
        }
        if(vid_memory[line_pos][column_pos] != 0)
                move_to_right_from_index(line_pos, column_pos);
        
                vid_memory[line_pos][column_pos] = character;
        column_pos++;
        set_cursor_pos(line_pos-top_line, column_pos);
}

void move_to_right_from_index(uint16_t line, uint16_t column_index)
{
    if (column_index >= COLUMNS_IN_LINE - 1) return;

    for (int i = COLUMNS_IN_LINE - 2; i >= column_index; i--) {
        vid_memory[line][i + 1] = vid_memory[line][i];
    }
    vid_memory[line][column_index] = 0;
}


void refresh_video()
{
        clear_screen();
        for (size_t i = top_line, j = 0; i < LINES+top_line; i++, j++){
                print_line(j, i);
        }
        set_cursor_pos(line_pos-top_line, column_pos);
}

void arrow_up();
void arrow_down();
void arrow_right();
void arrow_left();
void show_help();
void move_up();
void move_down();

void textr_main(const char *filename)
{
        uint8_t scancode = 0;
        uint8_t was_ctrl_pressed = 0;
        while(1) {
                refresh_video();
                char char_in = kgetc_with_hotkey_support(CTRL_KEY_CODE, &was_ctrl_pressed, &scancode);

                if(was_ctrl_pressed == 1 && char_in == 'h'){
                        show_help();
                        set_cursor_pos(line_pos, column_pos);
                        continue;
                }
                if(was_ctrl_pressed == 1 && char_in == 's'){
                        save_file(filename, vid_memory, allocated_lines);
                        continue;
                }
                if(was_ctrl_pressed == 1 && char_in == 'x'){
                        return;
                }
                if(char_in >= 32 && char_in <= 126) {
                        print_char(char_in);
                        continue;
                }
                if(char_in == '\n') {
                        new_line();
                        continue;
                }
                if(char_in == '\b') {
                        back_space();
                        continue;
                }

                if(scancode == ARROW_UP_KEY_CODE && was_ctrl_pressed == 0) arrow_up();
                if(scancode == ARROW_DOWN_KEY_CODE && was_ctrl_pressed == 0) arrow_down();
                if(scancode == ARROW_RIGHT_KEY_CODE && was_ctrl_pressed == 0) arrow_right();
                if(scancode == ARROW_LEFT_KEY_CODE && was_ctrl_pressed == 0) arrow_left();

                if(scancode == ARROW_UP_KEY_CODE && was_ctrl_pressed == 1) move_up();
                if(scancode == ARROW_DOWN_KEY_CODE && was_ctrl_pressed == 1) move_down();

        }
}

void arrow_up()
{
        if(line_pos == 0) return; //fuck you
        
        line_pos--;
        column_pos = find_pos_of_last_symbol_in_line(line_pos, column_pos);
        set_cursor_pos(line_pos, column_pos);
}

void arrow_down(){
        uint16_t new_column_pos = find_pos_of_last_symbol_in_line(line_pos+1, column_pos);
        if (vid_memory[line_pos+1][new_column_pos] == 0)
                return;
        
        line_pos++;
        column_pos = new_column_pos;
        set_cursor_pos(line_pos, column_pos);
}

void arrow_right()
{
        if(vid_memory[line_pos][column_pos] != '\n' && vid_memory[line_pos][column_pos] != 0){
                column_pos++;
                set_cursor_pos(line_pos, column_pos);
                return;
        }
        if(vid_memory[line_pos + 1][0] == 0)
                return;

        line_pos++;
        column_pos = 0;
        set_cursor_pos(line_pos, column_pos);
}

void arrow_left()
{
        if(column_pos == 0 && line_pos == 0) return; //fuckyou
        
        if(column_pos != 0){
                column_pos--;
                set_cursor_pos(line_pos, column_pos);
                return;
        }
        line_pos--;
        column_pos = find_pos_of_last_symbol_in_line(line_pos, COLUMNS_IN_LINE - 1);
        set_cursor_pos(line_pos, column_pos);
}

void show_help()
{
        clear_screen();
        set_cursor_pos(0,0);
        kprint_str("CTRL + h - shows this help");
        kprint_newline();
        kprint_str("CTRL + up/down arrow - moves screen up and down");
        kprint_newline();
        kprint_str("CTRL + s - saves file");
        kprint_newline();
        kprint_str("press any key to exit help");
        kgetc();
}

void move_down()
{
        if(top_line+LINES >= allocated_lines+LINES) return;
        top_line++;
}
void move_up()
{
        if(top_line == 0) return;
        top_line--;
}