#include "fs_def.h"
#include "my_open.h"

void my_open(char* filename) {
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
    
    // 2. 在当前目录中查找文件
    uint8_t *curr_dir_ptr = GET_BLOCK_ADDR(current_dir_block);
    DirEntry *target_entry = NULL;
    
    for (int i = 0; i < DIR_ENTRY_MAX; i++) {
        DirEntry *entry = (DirEntry *)(curr_dir_ptr + i * sizeof(DirEntry));
        
        // 跳过空项
        if (entry->filename[0] == 0) {
            continue;
        }
        
        // 比较文件名和扩展名
        if (strncmp(entry->filename, name, 8) == 0 && 
            strncmp(entry->ext, ext, 3) == 0) {
            target_entry = entry;
            break;
        }
    }
    
    // 3. 检查文件是否存在
    if (target_entry == NULL) {
        printf("错误: 文件 '%s' 不存在\n", filename);
        return;
    }
    
    // 4. 检查是否是文件（不是目录）
    if (target_entry->attribute == FILE_ATTR_DIR) {
        printf("错误: '%s' 是目录，不能作为文件打开\n", filename);
        return;
    }
    
    // 5. 在打开文件表中查找空闲位置
    int fd = -1;
    for (int i = 0; i < OPEN_FILE_MAX; i++) {
        if (open_file_table[i].dir_entry == NULL) {
            fd = i;
            break;
        }
    }
    
    // 6. 检查打开文件表是否已满
    if (fd == -1) {
        printf("错误: 打开文件表已满，无法打开更多文件\n");
        return;
    }
    
    // 7. 检查文件是否已经打开（可选：防止重复打开）
    for (int i = 0; i < OPEN_FILE_MAX; i++) {
        if (open_file_table[i].dir_entry != NULL &&
            open_file_table[i].dir_entry->first_block == target_entry->first_block) {
            printf("错误: 文件 '%s' 已经打开（文件描述符: %d）\n", filename, i);
            return;
        }
    }
    
    // 8. 填充打开文件表项
    open_file_table[fd].dir_entry = target_entry;
    open_file_table[fd].current_pos = 0;    // 读写指针指向文件开头
    open_file_table[fd].is_write = 1;       // 默认可读写
    open_file_table[fd].fd = fd;            // 文件描述符
    
    printf("成功打开文件: %s（文件描述符: %d）\n", filename, fd);
    
    // ===== 调试用：显示打开文件表信息 =====
    printf("  [调试] 文件起始块: %d\n", open_file_table[fd].dir_entry->first_block);
    printf("  [调试] 当前读写位置: %d\n", open_file_table[fd].current_pos);
    printf("  [调试] 打开模式: %s\n", 
           open_file_table[fd].is_write ? "读写" : "只读");
}
