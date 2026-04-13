#include "fs_def.h"

void my_close(char* filename) {
    // 1. 解析文件名和扩展名
    char name[9] = {0};
    char ext[4] = {0};
    
    char *dot = strchr(filename, '.');
    if (dot != NULL) {
        int name_len = dot - filename;
        int ext_len = strlen(dot + 1);
        
        if (name_len > 8 || ext_len > 3) {
            printf("错误: 文件名格式不正确\n");
            return;
        }
        
        strncpy(name, filename, name_len);
        strncpy(ext, dot + 1, ext_len);
    } else {
        if (strlen(filename) > 8) {
            printf("错误: 文件名不能超过8个字符\n");
            return;
        }
        strcpy(name, filename);
    }
    
    // 2. 在打开文件表中查找文件
    int fd = -1;
    for (int i = 0; i < OPEN_FILE_MAX; i++) {
        if (open_file_table[i].dir_entry != NULL) {
            DirEntry *entry = open_file_table[i].dir_entry;
            if (strncmp(entry->filename, name, 8) == 0 && 
                strncmp(entry->ext, ext, 3) == 0) {
                fd = i;
                break;
            }
        }
    }
    
    // 3. 检查文件是否打开
    if (fd == -1) {
        printf("错误: 文件 '%s' 未打开\n", filename);
        return;
    }

    OpenFileItem *file = &open_file_table[fd];

    // 4. 回收表项
    memset(file, 0, sizeof(OpenFileItem));
    printf("文件已关闭。\n");
}