#include "fs_def.h"
#include "my_rmdir.h"
#include <string.h>

/*
rmdir 的流程就是：先在当前目录找到目标目录
→ 判断是否为空（不为空且没加 -r 就报错）
→ 如果递归则先删除内部所有内容
→ 再通过 FAT 释放该目录占用的所有块
→ 最后把父目录中的那条目录项删除。
*/

static void normalize_filename(char *dst, const char *src) {
    memcpy(dst, src, 8);
    dst[8] = '\0';
    for (int i = 7; i >= 0 && dst[i] == ' '; --i) {
        dst[i] = '\0';
    }
}

static int is_valid_entry(const DirEntry *entry) {
    return entry->filename[0] != 0;
}

int is_directory_empty(uint32_t dir_block) {
    const int items_per_block = BLOCK_SIZE / sizeof(DirEntry);
    uint32_t block = dir_block;

    while (1) {
        DirEntry *entries = (DirEntry *)GET_BLOCK_ADDR(block);

        for (int i = 0; i < items_per_block; ++i) {
            if (!is_valid_entry(&entries[i])) continue;

            char name[9];
            normalize_filename(name, entries[i].filename);

            if (strcmp(name, ".") != 0 && strcmp(name, "..") != 0) {
                return 0;
            }
        }

        uint16_t next = fat_table[block];
        if (next == FAT_END || next == FAT_FREE || next == FAT_BAD) break;

        block = next;
    }

    return 1;
}

void free_directory_blocks(uint32_t dir_block) {
    uint32_t block = dir_block;

    while (1) {
        uint16_t next = fat_table[block];

        fat_table[block] = FAT_FREE;
        super_block->free_blocks++;

        if (next == FAT_END || next == FAT_FREE || next == FAT_BAD) break;

        block = next;
    }
}

int recursive_delete_directory(uint32_t dir_block) {
    const int items_per_block = BLOCK_SIZE / sizeof(DirEntry);
    uint32_t block = dir_block;

    while (1) {
        DirEntry *entries = (DirEntry *)GET_BLOCK_ADDR(block);

        for (int i = 0; i < items_per_block; ++i) {
            DirEntry *entry = &entries[i];
            if (!is_valid_entry(entry)) continue;

            char name[9];
            normalize_filename(name, entry->filename);

            if (strcmp(name, ".") == 0 || strcmp(name, "..") == 0) continue;

            if (entry->attribute == FILE_ATTR_DIR) {
                recursive_delete_directory(entry->first_block);
            }

            free_directory_blocks(entry->first_block);
            memset(entry, 0, sizeof(DirEntry));
        }

        uint16_t next = fat_table[block];
        if (next == FAT_END || next == FAT_FREE || next == FAT_BAD) break;

        block = next;
    }

    return 1;
}

static int parse_r_flag(char *arg, char **dirname) {
    if (strncmp(arg, "-r", 2) != 0) {
        *dirname = arg;
        return 0;
    }

    char *p = arg + 2;
    while (*p == ' ' || *p == '\t') p++;

    if (*p == '\0') return -1;

    *dirname = p;
    return 1;
}

void my_rmdir(char *arg) {
    if (arg == NULL || *arg == '\0') {
        printf("错误: 请指定要删除的目录名\n");
        return;
    }

    char *dirname = NULL;
    int recursive = parse_r_flag(arg, &dirname);

    if (recursive == -1) {
        printf("错误: 使用 -r 时需要指定目录名\n");
        return;
    }

    if (strlen(dirname) > 8) {
        printf("错误: 目录名不能超过8个字符\n");
        return;
    }

    if (!virtual_disk || !super_block) {
        printf("错误: 文件系统未初始化\n");
        return;
    }

    const int items_per_block = BLOCK_SIZE / sizeof(DirEntry);
    uint32_t block = current_dir_block;

    uint32_t target_block = 0;
    uint32_t parent_block = 0;
    int entry_index = -1;

    while (1) {
        DirEntry *entries = (DirEntry *)GET_BLOCK_ADDR(block);

        for (int i = 0; i < items_per_block; ++i) {
            DirEntry *entry = &entries[i];
            if (!is_valid_entry(entry)) continue;

            char name[9];
            normalize_filename(name, entry->filename);

            if (strcmp(name, dirname) == 0) {
                if (entry->attribute != FILE_ATTR_DIR) {
                    printf("错误: '%s' 不是目录\n", dirname);
                    return;
                }

                target_block = entry->first_block;
                parent_block = block;
                entry_index = i;
                break;
            }
        }

        if (entry_index != -1) break;

        uint16_t next = fat_table[block];
        if (next == FAT_END || next == FAT_FREE || next == FAT_BAD) break;

        block = next;
    }

    if (entry_index == -1) {
        printf("错误: 目录 '%s' 不存在\n", dirname);
        return;
    }

    if (!is_directory_empty(target_block)) {
        if (!recursive) {
            printf("错误: 目录 '%s' 不为空\n", dirname);
            return;
        }
        recursive_delete_directory(target_block);
    }

    free_directory_blocks(target_block);

    DirEntry *entries = (DirEntry *)GET_BLOCK_ADDR(parent_block);
    memset(&entries[entry_index], 0, sizeof(DirEntry));

    printf("成功删除目录: %s\n", dirname);
}