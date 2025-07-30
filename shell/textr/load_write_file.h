#pragma once 

void save_file(const char *filename, char **vid_memory, uint16_t allocated_lines);
uint32_t load_file(const char *filename, char ***vid_memory);