#include "fs_def.h"

int alloc_block(); // 分配一个空闲块
int do_write(int fd, char *text, int len, char style);

int my_write(int fd) {
    // 1. 检查 fd 有效性
    if (fd < 0 || fd >= OPEN_FILE_MAX || open_file_table[fd].is_open == 0) {
        printf("错误：文件描述符无效或文件未打开。\n");
        return -1;
    }

    OpenFileItem *file = &open_file_table[fd];
    char text[1024];
    int total_written = 0;
    char style;

    // 2. 交互：选择写模式
    printf("请选择写入方式:\n");
    printf("1: 截断写 (清空原内容)\n");
    printf("2: 覆盖写 (从当前指针位置修改)\n");
    printf("3: 追加写 (在文件末尾添加)\n");
    printf("请输入选择 (1/2/3): ");
    
    style = getchar();
    getchar(); // 吸收回车符

    // 3. 根据模式调整状态
    if (style == '1') {
        // 截断写：重置长度和指针
        file->file_size = 0;
        file->read_write_ptr = 0;
        printf("已清空文件内容。\n");
    } else if (style == '3') {
        // 追加写：指针移至末尾
        file->read_write_ptr = file->file_size;
    }

    // 4. 循环读取输入
    printf("开始输入内容 (按 Ctrl+D 或 Ctrl+Z 结束):\n");
    
    while (1) {
        int i = 0;
        char c;
        while ((c = getchar()) != '\n') {
            if (c == 0x04 || c == 0x1A) { // Ctrl+D / Ctrl+Z
                text[i] = '\0';
                break;
            }
            text[i++] = c;
            if (i >= 1023) break;
        }
        text[i] = '\0';

        if (i == 0 || (i == 1 && (text[0] == 0x04 || text[0] == 0x1A))) {
            break;
        }

        // 5. 调用底层写函数
        int ret = do_write(fd, text, strlen(text), style);
        if (ret < 0) {
            printf("写入错误：磁盘可能已满。\n");
            break;
        }
        total_written += ret;
    }

    // 6. 更新文件长度和状态
    if (file->read_write_ptr > file->file_size) {
        file->file_size = file->read_write_ptr;
        file->fcbstate = 1; // 标记 FCB 已修改
    }

    printf("写入完成。共写入 %d 字节。\n", total_written);
    return total_written;
}