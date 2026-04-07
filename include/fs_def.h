#ifndef FS_DEFS_H
#define FS_DEFS_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// ================= 1. 基础常量定义 =================

#define DISK_SIZE       1024 * 1024 * 10  // 虚拟磁盘总大小：10MB
#define BLOCK_SIZE      1024              // 物理块大小：1KB
#define BLOCK_COUNT     DISK_SIZE / BLOCK_SIZE // 总块数

// 文件属性常量
#define FILE_ATTR_FILE  0x00              // 普通文件
#define FILE_ATTR_DIR   0x01              // 目录
#define FILE_ATTR_HIDE  0x02              // 隐藏文件

// 系统状态
#define OPEN_FILE_MAX   10                // 系统允许同时打开的最大文件数
#define DIR_ENTRY_MAX   64                // 一个目录块中最大容纳的目录项数 (1KB / 16B = 64)
#define FAT_END         0xFFFF            // FAT 链结束标志
#define FAT_FREE        0x0000            // 空闲块标志
#define FAT_BAD         0xFFF7            // 坏块标志

// 虚拟磁盘布局规划 (假设)
// 第 0 块: 超级块 (SuperBlock) - 存放文件系统基本信息
// 第 1 块: FAT 表位示图/或FAT表起始 (为了简单，我们假设 FAT 表存放在紧接着的块中)
// 剩余块: 数据区

// ================= 2. 数据结构定义 =================

/* 
 * 目录项结构 (Directory Entry)
 * 相当于文件控制块 (FCB)
 * 大小设计为 16 字节
 */
typedef struct {
    char     filename[8];   // 文件名 (8.3格式简化，只用8字节)
    char     ext[3];        // 扩展名 (3字节)
    uint8_t  attribute;     // 属性 (0x00:文件, 0x01:目录)
    uint8_t  protection;    // 保护位 (简单的读写保护)
    uint16_t first_block;   // 起始物理块号 (指向 FAT 的索引)
    uint32_t file_size;     // 文件长度 (字节)
} DirEntry;

/*
 * 文件分配表项 (FAT Entry)
 * 每个表项 2 字节 (uint16_t)，可寻址 65536 个块
 */
typedef uint16_t FatItem;

/*
 * 超级块 (SuperBlock)
 * 存放在虚拟磁盘的第 0 块，用于描述文件系统整体信息
 */
typedef struct {
    uint32_t disk_size;     // 磁盘总大小
    uint32_t block_size;    // 块大小
    uint32_t total_blocks;  // 总块数
    uint32_t fat_start;     // FAT 表起始块号
    uint32_t fat_size;      // FAT 表占用块数
    uint32_t data_start;    // 数据区起始块号
    uint32_t root_dir_block;// 根目录所在的块号
    uint32_t free_blocks;   // 当前空闲块数量
} SuperBlock;

/*
 * 打开文件表项 (Open File Table)
 * 用于记录当前打开的文件状态，支持多文件操作
 */
typedef struct {
    DirEntry *dir_entry;    // 指向目录项的指针 (内存中的副本)
    uint32_t current_pos;   // 文件读写指针位置
    uint8_t  is_write;      // 打开模式 (0:读, 1:写)
    int8_t   fd;            // 文件描述符 (索引)
} OpenFileItem;

// ================= 3. 全局变量声明 (在 main.c 或 global.c 中定义) =================

// 虚拟磁盘内存指针
extern uint8_t *virtual_disk;

// 内存中的 FAT 表指针 (映射到 virtual_disk 的特定区域)
extern FatItem *fat_table;

// 内存中的超级块指针
extern SuperBlock *super_block;

// 打开文件表
extern OpenFileItem open_file_table[OPEN_FILE_MAX];

// 当前工作目录的块号
extern uint32_t current_dir_block;

// ================= 4. 辅助宏定义 =================

// 获取块在内存中的起始地址
#define GET_BLOCK_ADDR(block_num) (virtual_disk + (block_num) * BLOCK_SIZE)

// 声明：根据逻辑索引获取目录项地址
// current_block: 目录文件的起始块号（从 DirEntry 中获取）
// logic_index: 目录项的逻辑序号（0, 1, 2...）
DirEntry* get_dir_entry_by_index(uint32_t current_block, int logic_index);

// 文件系统持久化函数
void save_fs(const char* filename);
int load_fs(const char* filename);

#endif
