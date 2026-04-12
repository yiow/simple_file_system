#include "fs_def.h"
#include <stdio.h>
#include <string.h>

void save_fs(const char* filename) {
    if (filename == NULL || strlen(filename) == 0) {
        printf("错误: 请指定保存文件名\n");
        return;
    }

    if (virtual_disk == NULL || super_block == NULL) {
        printf("错误: 文件系统未初始化，无法保存\n");
        return;
    }

    printf("正在保存文件系统到 %s...\n", filename);

    FILE* file = fopen(filename, "wb");
    if (file == NULL) {
        printf("错误: 无法打开文件 %s 进行写入\n", filename);
        return;
    }

    size_t written = fwrite(virtual_disk, 1, DISK_SIZE, file);
    if (written != DISK_SIZE) {
        printf("错误: 写入文件不完整，只写入了 %zu/%d 字节\n", written, DISK_SIZE);
        fclose(file);
        return;
    }

    fclose(file);

    printf("文件系统保存成功！\n");
    printf("保存位置: %s\n", filename);
    printf("总大小: %d MB\n", DISK_SIZE / 1024 / 1024);
}

int load_fs(const char* filename) {
    if (filename == NULL || strlen(filename) == 0) {
        printf("错误: 请指定加载文件名\n");
        return 0;
    }

    printf("正在从 %s 加载文件系统...\n", filename);

    FILE* file = fopen(filename, "rb");
    if (file == NULL) {
        printf("注意: 文件 %s 不存在，将创建新的文件系统\n", filename);
        return 0;
    }

    fseek(file, 0, SEEK_END);
    long file_size = ftell(file);
    fseek(file, 0, SEEK_SET);

    if (file_size != DISK_SIZE) {
        printf("错误: 文件大小不匹配，期望 %d 字节，实际 %ld 字节\n", DISK_SIZE, file_size);
        fclose(file);
        return 0;
    }

    uint8_t* temp_disk = (uint8_t*)malloc(DISK_SIZE);
    if (temp_disk == NULL) {
        printf("错误: 内存分配失败\n");
        fclose(file);
        return 0;
    }

    size_t read = fread(temp_disk, 1, DISK_SIZE, file);
    if (read != DISK_SIZE) {
        printf("错误: 读取文件不完整，只读取了 %zu/%d 字节\n", read, DISK_SIZE);
        free(temp_disk);
        fclose(file);
        return 0;
    }

    fclose(file);

    if (virtual_disk != NULL) {
        free(virtual_disk);
    }

    virtual_disk = temp_disk;

    super_block = (SuperBlock*)GET_BLOCK_ADDR(0);
    fat_table = (FatItem*)GET_BLOCK_ADDR(super_block->fat_start);

    current_dir_block = super_block->root_dir_block;
    memset(open_file_table, 0, sizeof(open_file_table));

    printf("文件系统加载成功！\n");
    printf("总大小: %d MB\n", super_block->disk_size / 1024 / 1024);
    printf("块大小: %d B\n", super_block->block_size);
    printf("空闲块数: %d\n", super_block->free_blocks);

    return 1;
}