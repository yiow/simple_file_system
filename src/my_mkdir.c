#include "fs_def.h"
#include "my_mkdir.h"
void my_mkdir(char *name) {
    // 1. 参数合法性检查
    if (strlen(name) > 8) {
        printf("错误: 目录名不能超过8个字符\n");
        return;
    }

    // 2. 在 FAT 中寻找一个空闲块来存放新目录的内容
    int new_block_idx = -1;
    for (int i = 0; i < super_block->total_blocks; i++) {
        if (fat_table[i] == FAT_FREE) {
            new_block_idx = i;
            fat_table[i] = FAT_END; // 占用该块，设置为链表结尾
            break;
        }
    }

    if (new_block_idx == -1) {
        printf("错误: 磁盘空间已满，无法创建目录\n");
        return;
    }

    // 3. 检查当前目录是否已有同名文件/目录，并寻找空闲的目录项位置
    // 当前目录数据区的起始地址
    uint8_t *curr_dir_ptr = GET_BLOCK_ADDR(current_dir_block);
    int empty_entry_offset = -1; // 记录空闲目录项相对于数据区起始的偏移量

    // 一个块能存 DIR_ENTRY_MAX 个目录项
    for (int i = 0; i < DIR_ENTRY_MAX; i++) {
        DirEntry *entry = (DirEntry *)(curr_dir_ptr + i * sizeof(DirEntry));
        
        // 检查是否是空项 (通常通过判断第一个字符是否为0或特殊值，这里假设未使用项文件名为空)
        // 注意：刚格式化的磁盘内存可能是0，所以检查 filename[0] == 0 是常用做法
        if (entry->filename[0] == 0) {
            if (empty_entry_offset == -1) {
                empty_entry_offset = i * sizeof(DirEntry);
            }
            continue; 
        }

        // 检查重名 (简单比较前8个字符)
        if (strncmp(entry->filename, name, 8) == 0) {
            // 找到了同名文件，释放刚才申请的块
            fat_table[new_block_idx] = FAT_FREE;
            printf("错误: 目录或文件 '%s' 已存在\n", name);
            return;
        }
    }

    if (empty_entry_offset == -1) {
        // 当前目录满了
        fat_table[new_block_idx] = FAT_FREE;
        printf("错误: 当前目录已满，无法创建更多目录项\n");
        return;
    }

    // 4. 写入新目录的目录项 (DirEntry)
    DirEntry *new_dir_entry = (DirEntry *)(curr_dir_ptr + empty_entry_offset);
    
    // 复制文件名
    memset(new_dir_entry->filename, 0, 8);
    strncpy(new_dir_entry->filename, name, 8);
    
    // 清空扩展名
    memset(new_dir_entry->ext, 0, 3);
    
    // 设置属性为目录
    new_dir_entry->attribute = FILE_ATTR_DIR;
    
    // 设置保护位 (假设默认可读写)
    new_dir_entry->protection = 0;
    
    // 设置起始块号
    new_dir_entry->first_block = (uint16_t)new_block_idx;
    
    // 目录初始大小为 0 (或者你可以设为 BLOCK_SIZE，取决于你的设计，这里设为0表示空目录)
    new_dir_entry->file_size = 0;

    // 5. 初始化新目录的内容 (创建 . 和 ..)
    uint8_t *new_dir_ptr = GET_BLOCK_ADDR(new_block_idx);
    memset(new_dir_ptr, 0, BLOCK_SIZE); // 清空新块

    // 创建 "." (指向自己)
    DirEntry *self_entry = (DirEntry *)new_dir_ptr;
    strcpy(self_entry->filename, ".");
    self_entry->attribute = FILE_ATTR_DIR;
    self_entry->first_block = (uint16_t)new_block_idx;
    self_entry->file_size = 0;

    // 创建 ".." (指向父目录)
    DirEntry *parent_entry = (DirEntry *)(new_dir_ptr + sizeof(DirEntry));
    strcpy(parent_entry->filename, "..");
    parent_entry->attribute = FILE_ATTR_DIR;
    parent_entry->first_block = (uint16_t)current_dir_block;
    parent_entry->file_size = 0;

    printf("成功创建目录: %s\n", name);
}