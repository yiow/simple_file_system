#include "my_format.h"

// 1. 虚拟磁盘内存指针
uint8_t *virtual_disk = NULL;

// 2. 映射指针（指向虚拟磁盘的不同区域）
SuperBlock *super_block = NULL;
FatItem *fat_table = NULL;

// 3. 系统状态
OpenFileItem open_file_table[OPEN_FILE_MAX];
uint32_t current_dir_block = 0; // 当前目录块号



/**
 * my_format: 格式化虚拟磁盘
 * 功能：初始化内存布局，创建根目录，初始化FAT表
 */
void my_format() {
    printf("正在格式化虚拟磁盘 (%d MB)...\n", DISK_SIZE / 1024 / 1024);

    // 1. 如果已经存在虚拟磁盘，先释放（重新格式化）
    if (virtual_disk != NULL) {
        free(virtual_disk);
    }

    // 2. 开辟虚拟磁盘内存
    virtual_disk = (uint8_t *)malloc(DISK_SIZE);
    if (virtual_disk == NULL) {
        printf("错误：内存分配失败！\n");
        return;
    }

    // 3. 清空内存 (相当于全盘填0)
    memset(virtual_disk, 0, DISK_SIZE);

    // ==========================================
    // 4. 规划磁盘布局
    // ==========================================
    // 布局方案：
    // 块 0: 超级块 (SuperBlock)
    // 块 1: FAT 表 (假设 FAT 表只占 1 个块，实际应根据磁盘大小计算，这里简化处理)
    // 块 2: 根目录 (Root Directory) - 根目录也是文件，占用第 2 块
    // 块 3 ~ End: 数据区

    int fat_blocks = 1; // 简化：FAT表占用1个块

    // 4.1 设置超级块指针
    super_block = (SuperBlock *)GET_BLOCK_ADDR(0);

    // 4.2 填充超级块信息
    super_block->disk_size = DISK_SIZE;
    super_block->block_size = BLOCK_SIZE;
    super_block->total_blocks = BLOCK_COUNT;
    super_block->fat_start = 1;             // FAT从第1块开始
    super_block->fat_size = fat_blocks;     // FAT大小
    super_block->data_start = 2 + fat_blocks; // 数据区从第3块开始 (0:SB, 1:FAT, 2:Root)
    super_block->root_dir_block = 2;        // 根目录在第2块
    super_block->free_blocks = BLOCK_COUNT - super_block->data_start; // 计算空闲块数

    // 4.3 设置 FAT 表指针
    // 指向第 1 块的起始位置，并强转为 FatItem 数组
    fat_table = (FatItem *)GET_BLOCK_ADDR(super_block->fat_start);

    // ==========================================
    // 5. 初始化 FAT 表
    // ==========================================
    // 我们需要管理的总块数是 BLOCK_COUNT
    // 但是 FAT 表本身只需要记录数据区的块吗？
    // 通常 FAT 表记录所有块的状态。
    
    // 初始化所有表项为 0 (空闲)
    // 注意：这里只初始化我们需要的部分，防止越界
    int total_fat_items = BLOCK_COUNT; 
    for (int i = 0; i < total_fat_items; i++) {
        fat_table[i] = FAT_FREE;
    }

    // 标记系统占用的块 (不可分配给用户文件)
    // 块 0 (超级块) - 标记为坏块或特殊占用
    fat_table[0] = FAT_BAD; 
    
    // 块 1 (FAT表自身) - 标记为占用 (这里简单用非0非EOF表示占用，或者直接用FAT_BAD)
    fat_table[1] = FAT_BAD;

    // 块 2 (根目录) - 标记为占用，且是文件结尾
    fat_table[2] = FAT_END;

    // 更新超级块中的空闲块计数 (总块数 - 3个系统块)
    super_block->free_blocks = BLOCK_COUNT - 3;

    // ==========================================
    // 6. 初始化根目录
    // ==========================================
    // 根目录在物理块 2。我们需要在里面写入两个特殊的目录项：
    // "." (当前目录) 和 ".." (父目录，根目录的父目录是自己)
    
    DirEntry *root_dir = (DirEntry *)GET_BLOCK_ADDR(super_block->root_dir_block);

    // --- 创建 "." 目录项 ---
    strcpy(root_dir[0].filename, ".       "); // 补齐空格，或者直接存 "."
    strcpy(root_dir[0].ext, "   ");
    root_dir[0].attribute = FILE_ATTR_DIR;
    root_dir[0].first_block = super_block->root_dir_block; // 指向自己 (块2)
    root_dir[0].file_size = 0;

    // --- 创建 ".." 目录项 ---
    strcpy(root_dir[1].filename, "..      ");
    strcpy(root_dir[1].ext, "   ");
    root_dir[1].attribute = FILE_ATTR_DIR;
    root_dir[1].first_block = super_block->root_dir_block; // 指向自己 (块2)
    root_dir[1].file_size = 0;

    // 7. 初始化系统状态
    current_dir_block = super_block->root_dir_block; // 当前目录设为根目录
    memset(open_file_table, 0, sizeof(open_file_table)); // 清空打开文件表

    printf("格式化完成！\n");
    printf("总空间: %d MB\n", DISK_SIZE / 1024 / 1024);
    printf("块大小: %d B\n", BLOCK_SIZE);
    printf("根目录起始块: %d\n", super_block->root_dir_block);
}