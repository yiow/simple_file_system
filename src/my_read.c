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

static void split_filename_83(const char *input, char *name, size_t name_size, char *ext, size_t ext_size)
{
    const char *dot;
    size_t name_len;

    if (name_size > 0) name[0] = '\0';
    if (ext_size > 0) ext[0] = '\0';
    if (input == NULL) return;

    dot = strrchr(input, '.');
    if (dot == NULL || dot == input) {
        strncpy(name, input, name_size - 1);
        name[name_size - 1] = '\0';
        return;
    }

    name_len = (size_t)(dot - input);
    if (name_len >= name_size) name_len = name_size - 1;
    memcpy(name, input, name_len);
    name[name_len] = '\0';

    strncpy(ext, dot + 1, ext_size - 1);
    ext[ext_size - 1] = '\0';
}

static int is_valid_entry(const DirEntry *entry)
{
    return entry != NULL && entry->filename[0] != '\0';
}

static int file_name_equals(const DirEntry *entry, const char *name, const char *ext)
{
    char entry_name[9];
    char entry_ext[4];

    if (entry == NULL) return 0;

    copy_fixed_str(entry_name, sizeof(entry_name), entry->filename, sizeof(entry->filename));
    copy_fixed_str(entry_ext, sizeof(entry_ext), entry->ext, sizeof(entry->ext));

    return strcmp(entry_name, name) == 0 && strcmp(entry_ext, ext) == 0;
}

void my_read(char *filename)
{
    int i;
    DirEntry *entry = NULL;
    char name[9];
    char ext[4];
    uint32_t remain;
    uint16_t block;

    if (virtual_disk == NULL || fat_table == NULL || super_block == NULL) {
        printf("my_read: filesystem is not initialized\n");
        return;
    }

    if (filename == NULL || filename[0] == '\0') {
        printf("my_read: missing file name\n");
        return;
    }

    split_filename_83(filename, name, sizeof(name), ext, sizeof(ext));

    for (i = 0; i < DIR_ENTRY_MAX; ++i) {
        DirEntry *cur = get_dir_entry_by_index(current_dir_block, i);
        if (!is_valid_entry(cur)) continue;
        if (file_name_equals(cur, name, ext)) {
            entry = cur;
            break;
        }
    }

    if (entry == NULL) {
        printf("my_read: no such file: %s\n", filename);
        return;
    }

    if (entry->attribute & FILE_ATTR_DIR) {
        printf("my_read: %s is a directory\n", filename);
        return;
    }

    if (entry->file_size == 0) {
        printf("[empty file]\n");
        return;
    }

    block = entry->first_block;
    remain = entry->file_size;

    while (remain > 0) {
        size_t chunk;
        uint16_t next_block;

        if (block == FAT_FREE || block == FAT_END || block >= BLOCK_COUNT) {
            printf("\nmy_read: broken FAT chain\n");
            return;
        }

        chunk = (remain > BLOCK_SIZE) ? BLOCK_SIZE : (size_t)remain;
        fwrite(GET_BLOCK_ADDR(block), 1, chunk, stdout);

        remain -= (uint32_t)chunk;
        if (remain == 0) {
            break;
        }

        next_block = fat_table[block];
        if (next_block == block) {
            printf("\nmy_read: FAT loop detected\n");
            return;
        }
        block = next_block;
    }

    putchar('\n');
}