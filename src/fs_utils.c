#include "fs_def.h"

/*
 * 根据逻辑索引获取目录项的内存地址
 * 原理：目录也是文件，其数据块通过 FAT 链连接。
 * 我们需要计算逻辑索引对应的物理块号。
 */
DirEntry* get_dir_entry_by_index(uint32_t first_block, int logic_index) {
    // 1. 计算一个块能存多少个目录项
    // 假设 BLOCK_SIZE = 1024, sizeof(DirEntry) = 16 -> 64个
    int items_per_block = BLOCK_SIZE / sizeof(DirEntry);

    // 2. 计算目标目录项位于第几个块（相对于起始块的偏移）
    // 例如：index=65，则在第 1 个块（0号块存0-63，1号块存64-127）
    int block_offset = logic_index / items_per_block;

    // 3. 计算目标目录项在块内的下标
    int index_in_block = logic_index % items_per_block;

    // 4. 遍历 FAT 链，找到目标物理块号
    uint32_t current_fat_block = first_block;
    
    // 如果目标在第 0 个块，直接用起始块
    if (block_offset == 0) {
        // 无需遍历
    } 
    else {
        // 顺着 FAT 链往后找 block_offset 次
        for (int i = 0; i < block_offset; i++) {
            uint16_t next_block = fat_table[current_fat_block];
            
            // 如果链表提前结束（文件损坏或索引越界）
            if (next_block == FAT_END || next_block == FAT_FREE) {
                return NULL; 
            }
            current_fat_block = next_block;
        }
    }

    // 5. 计算最终内存地址
    // 找到物理块后，像之前一样计算块内偏移
    DirEntry* block_start_addr = (DirEntry*)GET_BLOCK_ADDR(current_fat_block);
    return block_start_addr + index_in_block;
}