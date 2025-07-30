#include "../../fs/file_system.h"
#include "../../mylibs/kernelio.h"

void save_file(const char *filename, char **vid_memory, uint16_t allocated_lines)
{
        uint32_t data_size = allocated_lines * COLUMNS_IN_LINE;
        BYTE *buffer = kcalloc(data_size);

        for (size_t i = 0; i < allocated_lines; i++) {
                if (vid_memory[i] == NULL) break;
                memcpy(buffer + i * COLUMNS_IN_LINE, vid_memory[i], COLUMNS_IN_LINE);
        }

        write_in_file(filename, buffer, data_size);
        //kfree(buffer);
}

char** create_new_vid_memory();

// returns size bytes // 0 if error
uint32_t load_file(const char *filename, char ***vid_memory)
{
        uint32_t size = 4096;
        char* data = read_from_file(filename, 0); // returns 4096 bytes (1 cluster)

        if (data == NULL) {
                return 0;
        }

        if (*data == 0) {
                *vid_memory = create_new_vid_memory();
                return size;
        }

        *vid_memory = create_new_vid_memory();

        for (uint32_t i = 0; i < size / COLUMNS_IN_LINE && i < LINES * 3; i++) {
                memcpy((*vid_memory)[i], data + i * COLUMNS_IN_LINE, COLUMNS_IN_LINE);
        }

        // kfree(data); // якщо read_from_file malloc-ить — розкоментуй

        return size;
}


char** create_new_vid_memory()
{
        char **new_vid_memory = kcalloc(LINES*3 * sizeof(char*));       // * 3 for correct listing without painting memory with garbage

        for (uint16_t i = 0; i < LINES*3; i++) {
                new_vid_memory[i] = kcalloc(COLUMNS_IN_LINE * sizeof(char));
        }

        return new_vid_memory;
}