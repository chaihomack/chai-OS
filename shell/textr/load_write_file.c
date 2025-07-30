#include "../../fs/file_system.h"
#include "../../mylibs/kernelio.h"

void save_file(const char *filename, char **vid_memory, uint16_t allocated_lines)
{
        BYTE *buffer = kcalloc(2048, 0);
        for(size_t i = 0; i < allocated_lines; i++) {
                if (vid_memory[i] == NULL) continue; 
                
                memcpy(buffer + i * COLUMNS_IN_LINE, vid_memory[i], COLUMNS_IN_LINE);
        }
        uint32_t data_size = allocated_lines * COLUMNS_IN_LINE;
        write_in_file(filename, buffer, 2048, 0);
}

void load_file(const char *filename, char **vid_memory)
{
        uint32_t size = 0;
        char* data = read_from_file(filename, &size, 0);
        if (data == NULL) {
                kprint_str("Error reading file: ");
                kprint_str(filename);
                kprint_newline();
                return;
        }
        
        for (uint32_t i = 0; i < size && i < LINES * COLUMNS_IN_LINE; i++) {
                memcpy(vid_memory[i], data + i*COLUMNS_IN_LINE, COLUMNS_IN_LINE);
        }
        
        // kfree(data);
}