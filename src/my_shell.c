#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "my_shell.h" 
#include "my_ls.h"
#include "my_mkdir.h"
#include "my_rmdir.h"
#include "my_cd.h"
#include "my_create.h"
#include "my_rm.h"
#include "my_open.h"
#include "my_close.h"
#include "my_write.h"
#include "my_read.h"



// 简单的命令分发器
int execute_command(char *cmd_line) {
    // 复制一份字符串，因为 strtok 会修改原字符串
    char buffer[256];
    strcpy(buffer, cmd_line);

    // 获取第一个词（命令）
    char *cmd = strtok(buffer, " \t\n"); // 增加 \t 和 \n 的处理，更稳健

    if (cmd == NULL) return 1; // 空行，继续循环

    // --- 命令匹配逻辑 ---

    // 1. 退出命令
    if (strcmp(cmd, "exit") == 0) {
        return 0; 
    }

    // 2. ls 命令 (无参数)
    else if (strcmp(cmd, "ls") == 0) {
        my_ls();
    }

    // 3. mkdir 命令
    else if (strcmp(cmd, "mkdir") == 0) {
        char *arg = strtok(NULL, " \t\n");
        if (arg != NULL) {
            my_mkdir(arg);
        } else {
            printf("用法: mkdir <目录名>\n");
        }
    }

    // 4. rmdir 命令（支持 -r 选项）
    else if (strcmp(cmd, "rmdir") == 0) {
        char *arg1 = strtok(NULL, " \t\n");
        char *arg2 = strtok(NULL, " \t\n");

        if (arg1 != NULL) {
            // 检查是否有第二个参数
            if (arg2 != NULL) {
                // 格式：rmdir -r dirname
                if (strcmp(arg1, "-r") == 0) {
                    char full_arg[16];
                    strcpy(full_arg, arg1);
                    strcat(full_arg, " ");
                    strcat(full_arg, arg2);
                    my_rmdir(full_arg);
                } else {
                    printf("错误: 无效的选项 '%s'\n", arg1);
                    printf("用法: rmdir [-r] <目录名>\n");
                }
            } else {
                // 只有一个参数：rmdir dirname 或 rmdir -rdirname
                my_rmdir(arg1);
            }
        } else {
            printf("用法: rmdir [-r] <目录名>\n");
        }
    }

    // 5. cd 命令
    else if (strcmp(cmd, "cd") == 0) {
        char *arg = strtok(NULL, " \t\n");
        if (arg != NULL) {
            my_cd(arg);
        } else {
            printf("用法: cd <目录名>\n");
        }
    }

    // 6. create 命令 (创建文件)
    else if (strcmp(cmd, "create") == 0) {
        char *arg = strtok(NULL, " \t\n");
        if (arg != NULL) {
            my_create(arg);
        } else {
            printf("用法: create <文件名>\n");
        }
    }

    // 7. rm 命令 (删除文件)
    else if (strcmp(cmd, "rm") == 0) {
        char *arg = strtok(NULL, " \t\n");
        if (arg != NULL) {
            my_rm(arg);
        } else {
            printf("用法: rm <文件名>\n");
        }
    }

    // 8. open 命令
    else if (strcmp(cmd, "open") == 0) {
        char *arg = strtok(NULL, " \t\n");
        if (arg != NULL) {
            my_open(arg);
        } else {
            printf("用法: open <文件名>\n");
        }
    }

    // 9. close 命令
    else if (strcmp(cmd, "close") == 0) {
        char *arg = strtok(NULL, " \t\n");
        if (arg != NULL) {
            // 注意：如果你的 my_close 接受的是文件描述符(int)，这里可能需要 atoi(arg)
            // 如果你的 my_close 接受的是文件名，则保持原样
            my_close(arg); 
        } else {
            printf("用法: close <文件描述符或文件名>\n");
        }
    }

    // 10. write 命令 (写文件)
    // 用法示例: write filename "Hello World"
    else if (strcmp(cmd, "write") == 0) {
        char *filename = strtok(NULL, " \t\n");
        char *content = strtok(NULL, "\n"); // 剩下的所有内容都作为写入内容（支持带空格的内容）

        if (filename != NULL && content != NULL) {
            // 去除内容前后的引号（可选，看你的实现是否需要）
            // 这里为了简单，直接传指针
            my_write(filename, content);
        } else {
            printf("用法: write <文件名> <内容>\n");
        }
    }

    // 11. read 命令
    else if (strcmp(cmd, "read") == 0) {
        char *filename = strtok(NULL, " \t\n");
        if (filename != NULL) {
            my_read(filename);
        } else {
            printf("用法: read <文件名>\n");
        }
    }

    // 12. 未知命令
    else {
        printf("未知命令: %s\n", cmd);
        printf("支持的命令: ls, mkdir, rmdir, cd, create, rm, open, close, write, read, exit\n");
    }

    return 1; // 继续循环
}