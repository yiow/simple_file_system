#include "fs_def.h"

void my_close(int fd) {
    if (fd < 0 || fd >= OPEN_FILE_MAX || open_file_table[fd].is_open == 0) {
        printf("错误：文件描述符无效或文件未打开。\n");
        return;
    }

    OpenFileItem *file = &open_file_table[fd];

    // 检查 fcbstate
    if (file->fcbstate == 1) {
        printf("正在将 FCB 更新回磁盘...\n");
        
        // 1. 定位父目录块
        uint8_t *dir_block_ptr = GET_BLOCK_ADDR(file->dir_block_no);
        
        // 2. 定位目录项
        DirEntry *dir_entry = (DirEntry *)dir_block_ptr;
        dir_entry += file->dir_offset;

        // 3. 写回数据
        memcpy(dir_entry->filename, file->filename, 8);
        memcpy(dir_entry->ext, file->ext, 3);
        dir_entry->attribute = file->attribute;
        dir_entry->first_block = file->first_block;
        dir_entry->file_size = file->file_size; // 关键：更新长度

        printf("FCB 已更新。\n");
    } else {
        printf("文件未修改，无需写回磁盘。\n");
    }

    // 4. 回收表项
    memset(file, 0, sizeof(OpenFileItem));
    printf("文件已关闭。\n");
}