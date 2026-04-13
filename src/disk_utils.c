#include "fs_def.h"

/**
 * 分配一个空闲磁盘块
 * 返回：块号，如果失败返回 -1
 */
int alloc_block() {
    // 从数据区开始查找 (super_block->data_start)
    for (int i = super_block->data_start; i < super_block->total_blocks; i++) {
        if (fat_table[i] == FAT_FREE) {
            // 找到空闲块，标记为文件结尾 (FAT_END)
            // 简化处理：每次分配都视为新块的结尾，由 do_write 处理链式关系
            fat_table[i] = FAT_END;
            super_block->free_blocks--;
            return i;
        }
    }
    return -1; // 磁盘满
}


int do_write(int fd, char *text, int len, char style) {
    OpenFileItem *file = &open_file_table[fd];
    int bytes_written = 0;
    
    while (bytes_written < len) {
        // 1. 计算当前逻辑块号和块内偏移
        int current_ptr = file->read_write_ptr;
        int logic_block = current_ptr / BLOCK_SIZE;
        int offset = current_ptr % BLOCK_SIZE;

        // 2. 找到当前逻辑块对应的物理块        
        int phy_block = -1;
        int current_fat = file->first_block;
        
        // 如果是文件的第一个块
        if (logic_block == 0) {
            if (file->first_block == 0) {
                phy_block = alloc_block();
                if (phy_block == -1) return -1;
                file->first_block = phy_block; // 更新文件起始块
            } else {
                phy_block = file->first_block;
            }
        } else {
            int count = 0;
            int temp_block = file->first_block;
            while (temp_block != FAT_END && temp_block != FAT_FREE && temp_block != -1) {
                if (count == logic_block) {
                    phy_block = temp_block;
                    break;
                }
                if (fat_table[temp_block] == FAT_END) {
                    int new_block = alloc_block();
                    if (new_block == -1) return -1;
                    fat_table[temp_block] = new_block;
                    phy_block = new_block;
                    break;
                }
                temp_block = fat_table[temp_block];
                count++;
            }
            
        }

        // 3. 写入数据
        uint8_t *block_ptr = GET_BLOCK_ADDR(phy_block);
        
        int space_left = BLOCK_SIZE - offset;
        int to_write = (len - bytes_written) < space_left ? (len - bytes_written) : space_left;

        memcpy(block_ptr + offset, text + bytes_written, to_write);

        // 4. 更新指针
        file->read_write_ptr += to_write;
        bytes_written += to_write;
    }

    return bytes_written;
}