#include "fs_def.h"
#include "my_ls.h"

static void trim_copy_name(const char src[8], char out[9]) {
    memcpy(out, src, 8);
    out[8] = '\0';
    int len = 8;
    while (len > 0 && out[len - 1] == ' ')
        len--;
    out[len] = '\0';
}

static void trim_copy_ext(const char src[3], char out[4]) {
    memcpy(out, src, 3);
    out[3] = '\0';
    int len = 3;
    while (len > 0 && out[len - 1] == ' ')
        len--;
    out[len] = '\0';
}

void my_ls(void) {
    uint8_t *curr = GET_BLOCK_ADDR(current_dir_block);

    for (int i = 0; i < DIR_ENTRY_MAX; i++) {
        DirEntry *e = (DirEntry *)(curr + i * sizeof(DirEntry));
        if (e->filename[0] == '\0')
            continue;

        char name[9], ext[4];
        trim_copy_name(e->filename, name);
        trim_copy_ext(e->ext, ext);

        if (ext[0] != '\0')
            printf("%s.%s", name, ext);
        else
            printf("%s", name);

        if (e->attribute == FILE_ATTR_DIR)
            printf("  [目录]");
        else if (e->attribute == FILE_ATTR_HIDE)
            printf("  [隐藏]");
        else
            printf("  %u 字节", (unsigned)e->file_size);

        printf("\n");
    }
    fflush(stdout);
}
