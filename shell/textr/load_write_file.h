#pragma once 

void save_file(const char *filename, char **vid_memory, uint16_t allocated_lines);
void load_file(const char *filename, char **vid_memory);