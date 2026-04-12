#include "fs_def.h"
#include "my_create.h"

void my_create(char* filename) {
    // 1. 参数合法性检查 - 解析文件名和扩展名
    char name[9] = {0};
    char ext[4] = {0};
    
    // 查找扩展名分隔符
    char *dot = strchr(filename, '.');
    if (dot != NULL) {
        // 有扩展名的情况
        int name_len = dot - filename;
        int ext_len = strlen(dot + 1);
        
        if (name_len > 8) {
            printf("错误: 文件名不能超过8个字符\n");
            return;
        }
        if (ext_len > 3) {
            printf("错误: 扩展名不能超过3个字符\n");
            return;
        }
        
        strncpy(name, filename, name_len);
        strncpy(ext, dot + 1, ext_len);
    } else {
        // 无扩展名的情况
        if (strlen(filename) > 8) {
            printf("错误: 文件名不能超过8个字符\n");
            return;
        }
        strcpy(name, filename);
    }
    
    // 2. 在 FAT 中寻找一个空闲块来存放文件内容
    int new_block_idx = -1;
    for (int i = 0; i < super_block->total_blocks; i++) {
        if (fat_table[i] == FAT_FREE) {
            new_block_idx = i;
            fat_table[i] = FAT_END; // 占用该块，设置为链表结尾
            break;
        }
    }
    
    if (new_block_idx == -1) {
        printf("错误: 磁盘空间已满，无法创建文件\n");
        return;
    }
    
    // 3. 检查当前目录是否已有同名文件/目录，并寻找空闲的目录项位置
    uint8_t *curr_dir_ptr = GET_BLOCK_ADDR(current_dir_block);
    int empty_entry_offset = -1;
    
    for (int i = 0; i < DIR_ENTRY_MAX; i++) {
        DirEntry *entry = (DirEntry *)(curr_dir_ptr + i * sizeof(DirEntry));
        
        // 检查是否是空项
        if (entry->filename[0] == 0) {
            if (empty_entry_offset == -1) {
                empty_entry_offset = i * sizeof(DirEntry);
            }
            continue;
        }
        
        // 检查重名 - 比较文件名和扩展名
        if (strncmp(entry->filename, name, 8) == 0 && 
            strncmp(entry->ext, ext, 3) == 0) {
            // 找到了同名文件，释放刚才申请的块
            fat_table[new_block_idx] = FAT_FREE;
            printf("错误: 文件 '%s' 已存在\n", filename);
            return;
        }
    }
    
    if (empty_entry_offset == -1) {
        // 当前目录满了
        fat_table[new_block_idx] = FAT_FREE;
        printf("错误: 当前目录已满，无法创建更多文件\n");
        return;
    }
    
    // 4. 写入新文件的目录项 (DirEntry)
    DirEntry *new_file_entry = (DirEntry *)(curr_dir_ptr + empty_entry_offset);
    
    // 复制文件名
    memset(new_file_entry->filename, 0, 8);
    strncpy(new_file_entry->filename, name, 8);
    
    // 复制扩展名
    memset(new_file_entry->ext, 0, 3);
    strncpy(new_file_entry->ext, ext, 3);
    
    // 设置属性为普通文件
    new_file_entry->attribute = FILE_ATTR_FILE;
    
    // 设置保护位（默认可读写）
    new_file_entry->protection = 0;
    
    // 设置起始块号
    new_file_entry->first_block = (uint16_t)new_block_idx;
    
    // 文件初始大小为 0
    new_file_entry->file_size = 0;
    
    printf("成功创建文件: %s\n", filename);
    
    // ===== 调试用：显示创建的文件信息 =====
    printf("  [调试] 文件名: %.8s\n", new_file_entry->filename);
    printf("  [调试] 扩展名: %.3s\n", new_file_entry->ext);
    printf("  [调试] 属性: %s\n", 
           new_file_entry->attribute == FILE_ATTR_FILE ? "文件" : "目录");
    printf("  [调试] 起始块: %d\n", new_file_entry->first_block);
    printf("  [调试] 文件大小: %d 字节\n", new_file_entry->file_size);
}
