#include "fs_def.h"      // 你的定义
#include "my_format.h"   // 你的格式化/初始化函数
#include "my_shell.h"    // 下面我们要写的头文件

#define MAX_CMD_LEN 256

int main() {
    char cmd_buffer[MAX_CMD_LEN];

    // 1. 初始化文件系统 (挂载虚拟磁盘)
    printf("正在初始化文件系统...\n");
    my_format();

    // 2. Shell 主循环
    while (1) {
        // 打印提示符
        printf("myfs> ");
        
        // 读取用户输入
        if (!fgets(cmd_buffer, MAX_CMD_LEN, stdin)) {
            break; // 处理 Ctrl+D 或读取错误
        }

        // 去掉末尾的换行符
        cmd_buffer[strcspn(cmd_buffer, "\n")] = 0;

        // 解析并执行命令
        if (execute_command(cmd_buffer) == 0) {
            break; // 如果返回0，表示需要退出
        }
    }

    printf("再见！\n");
    return 0;
}