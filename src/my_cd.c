#include "fs_def.h"

static void copy_fixed_str(char *dst, size_t dst_size, const char *src, size_t src_len)
{
    size_t i;
    if (dst_size == 0) return;
    for (i = 0; i < src_len && i + 1 < dst_size; ++i) {
        if (src[i] == '\0') break;
        dst[i] = src[i];
    }
    dst[i] = '\0';
}

static int dir_name_equals(const DirEntry *entry, const char *name)
{
    char entry_name[9];
    if (entry == NULL || name == NULL) return 0;
    copy_fixed_str(entry_name, sizeof(entry_name), entry->filename, sizeof(entry->filename));
    return strcmp(entry_name, name) == 0;
}

static int is_valid_entry(const DirEntry *entry)
{
    return entry != NULL && entry->filename[0] != '\0';
}

void my_cd(char *filename)
{
    int i;
    DirEntry *entry;

    if (virtual_disk == NULL || fat_table == NULL || super_block == NULL) {
        printf("my_cd: filesystem is not initialized\n");
        return;
    }

    if (filename == NULL || filename[0] == '\0') {
        printf("my_cd: missing directory name\n");
        return;
    }

    if (strcmp(filename, ".") == 0) {
        return;
    }

    if (strcmp(filename, "/") == 0) {
        current_dir_block = super_block->root_dir_block;
        return;
    }

    if (strcmp(filename, "..") == 0) {
        if (current_dir_block == super_block->root_dir_block) {
            return;
        }

        for (i = 0; i < DIR_ENTRY_MAX; ++i) {
            entry = get_dir_entry_by_index(current_dir_block, i);
            if (!is_valid_entry(entry)) continue;
            if ((entry->attribute & FILE_ATTR_DIR) && dir_name_equals(entry, "..")) {
                current_dir_block = entry->first_block;
                return;
            }
        }

        printf("my_cd: parent directory entry not found\n");
        return;
    }

    for (i = 0; i < DIR_ENTRY_MAX; ++i) {
        entry = get_dir_entry_by_index(current_dir_block, i);
        if (!is_valid_entry(entry)) continue;

        if (dir_name_equals(entry, filename)) {
            if ((entry->attribute & FILE_ATTR_DIR) == 0) {
                printf("my_cd: %s is not a directory\n", filename);
                return;
            }
            current_dir_block = entry->first_block;
            return;
        }
    }

    printf("my_cd: no such directory: %s\n", filename);
}