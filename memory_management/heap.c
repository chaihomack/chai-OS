#include"heap.h"

#include<stdint.h>
#include"../mylibs/my_stdtypes.h"

extern uint32_t* get_address_after_kernel();

int find_free_bit_and_turn_on(BYTE *bitmap_block_index, uint32_t from, uint32_t to);

#define RECS_PER_BITMAP 263173
BYTE bitmap_for_kmalloc[RECS_PER_BITMAP][32] = {0}; //255 records in one block so 256/8(bits) = 32, one block is 32KiB
//128x32B
//64x64B
//32x128B
//16x256B
//8x512B
//4x1024B
//2x2048B
//1x4096B

// 4096B MAX!!!
void* kmalloc(uint16_t bytes_to_alloc) {
        if (bytes_to_alloc == 0 || bytes_to_alloc > 4096) 
                return NULL;

        uint8_t aligned_to_power_of_two = 5;
        uint16_t current_size = 32;
        
        while (aligned_to_power_of_two < 12 && bytes_to_alloc > current_size) {
                aligned_to_power_of_two++;
                current_size <<= 1;
        }

        uint32_t start_bit = 256 - (1 << (((12 - aligned_to_power_of_two) + 1)));
        uint32_t end_bit = 256 - (1 << (12 - aligned_to_power_of_two));
        uint16_t chunk_size = 1 << aligned_to_power_of_two;

        uint32_t *current_rec_ptr = get_address_after_kernel();
                                       
        for (int i = 0; i < RECS_PER_BITMAP; i++) {
                int free_bit = find_free_bit_and_turn_on(bitmap_for_kmalloc[i], start_bit, end_bit);
                
                if (free_bit != -1 && free_bit != 256) {
                uint32_t section_offset = 0x1000 * (aligned_to_power_of_two - 5);
                return (void*) current_rec_ptr + (section_offset + (1 << aligned_to_power_of_two)*(free_bit-(256 - (1 << ((12 - aligned_to_power_of_two)+1)))));
                }
                current_rec_ptr += 0x8000;
        }
        
        return NULL;
}
// from >= and < to
int find_free_bit_and_turn_on(BYTE *bitmap_block_index,
                                uint32_t from, uint32_t to) 
{
        for (uint32_t bit_idx = from; bit_idx < to; bit_idx++) {
                uint32_t byte_idx = bit_idx / 8;
                uint8_t bit_in_byte = bit_idx % 8;
                BYTE mask = (BYTE)(1 << bit_in_byte);

                if ((bitmap_block_index[byte_idx] & mask) == 0) {
                bitmap_block_index[byte_idx] |= mask;
                return bit_idx;
                }
        }

        return -1;
}