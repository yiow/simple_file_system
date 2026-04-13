#include "fs_def.h"
#include "my_rm.h"

void my_rm(char *arg) {
    if (arg == NULL || arg[0] == '\0') {
        printf("用法: rm <文件名>\n");
        return;
    }
    if (strlen(arg) > 8) {
        printf("错误: 文件名不能超过8个字符\n");
        return;
    }

    uint8_t *curr = GET_BLOCK_ADDR(current_dir_block);

    for (int i = 0; i < DIR_ENTRY_MAX; i++) {
        DirEntry *entry = (DirEntry *)(curr + i * sizeof(DirEntry));
        if (entry->filename[0] == '\0')
            continue;
        if (strncmp(entry->filename, arg, 8) != 0)
            continue;

        if (entry->attribute == FILE_ATTR_DIR) {
            printf("错误: '%s' 是目录，请使用 rmdir 删除\n", arg);
            return;
        }

        uint16_t blk = entry->first_block;
        int steps = 0;
        while (blk != 0 && blk < super_block->total_blocks && steps < (int)super_block->total_blocks) {
            steps++;
            if (blk < super_block->data_start) {
                printf("错误: 文件数据块号异常\n");
                return;
            }
            uint16_t next = fat_table[blk];
            if (fat_table[blk] == FAT_FREE)
                break;
            fat_table[blk] = FAT_FREE;
            super_block->free_blocks++;
            if (next == FAT_END)
                break;
            blk = next;
        }

        for (int j = 0; j < OPEN_FILE_MAX; j++) {
            if (open_file_table[j].dir_entry == entry)
                memset(&open_file_table[j], 0, sizeof(OpenFileItem));
        }

        memset(entry, 0, sizeof(DirEntry));
        printf("成功删除文件: %s\n", arg);
        return;
    }

    printf("错误: 找不到文件 '%s'\n", arg);
}
