#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <readline/readline.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include "queue.h"

#define MAX 1000

char **separ(char *input, char *separator) //分割
{
    char **command = malloc(8 * sizeof(char *));
    char *parsed;
    int index = 0;

    parsed = strtok(input, separator);
    while (parsed != NULL) {
        command[index++] = parsed;
        parsed = strtok(NULL, separator);
    }

    command[index] = NULL;
    return command;
}

int pipe_count(char *input)
{
    int count = 0;
    int i;

    for(i = 0; input[i]; i++)
        if(input[i] == '|') count++;

    return count;
}

void shell_cmd(char *str)
{
    if(strstr(str, "exit"))
        exit(0);
}

void creat_proc(char **command, int in_fd, int out_fd, int *close_fd, int pipe_max, int (*pipe_fd)[2])
//依據輸入的command載入程序和重新設定STDIN&STDOUT
{
    pid_t pid;

    if((pid = fork()) < 0)
        exit(0);
    else if(pid == 0)
    {
        if (in_fd != STDIN_FILENO) dup2(in_fd, STDIN_FILENO);
        if (out_fd != STDOUT_FILENO) dup2(out_fd, STDOUT_FILENO);

        close(close_fd[0]);
        close(close_fd[1]);
        for(int i = 0; i < pipe_max; i++)
        {
            close(pipe_fd[i][0]);
            close(pipe_fd[i][1]);
        }
        if(execvp(command[0], command) == -1)
        {
            fprintf(stderr,
                    "Error: Unable to load the executable %s.\n",
                    command[0]);
            exit(EXIT_FAILURE); 
        }
    }
}

int isredir(char *in)
{
    while(*in)
        if(*in++ == '>') return 1;
    return 0;
}

char *off(char *str)
{
    while(*str++ == ' ');

    return str-1;
}

int ispipe_out(char *str)
{
    return atoi(str);
}

int main() 
{
    char **list;
    char *input;
    int pipe_max;
    int pipe_fd[MAX][2];
    struct queue *que = queue_new();
    struct data in = {};

    while ((input = readline("% ")) != NULL) 
    {
        shell_cmd(input);
        pipe_max = pipe_count(input);
        list = separ(input, "|");       //用|分段 A | B | C ,list[0] = A ,list[1] = B

        for(int i = 0; i < pipe_max; i++)  //建立pipe
            pipe(pipe_fd[i]);

        int stdin_fileno = STDIN_FILENO;
        in = que->read(que, 0);
        if(in.pip[0] || in.pip[1])
            stdin_fileno = in.pip[0];
        for(int i = 0; list[i] != NULL; i++)
        {
            char **command;
            int in_fd  = (i == 0)?(stdin_fileno):(pipe_fd[i - 1][0]);   //第一個的輸入為STDIN
            int out_fd = (list[i + 1] == NULL)?(STDOUT_FILENO):(pipe_fd[i][1]); //最後一個的輸出為STDOUT
            int close_fd[2] = {-1, -1};

            if(in_fd != STDIN_FILENO)
                close_fd[0] = pipe_fd[i - 1][1];
            if(out_fd != STDOUT_FILENO)
                close_fd[1] = pipe_fd[i][0];

            if(isredir(list[i]))    //有重導向符號'>'
            {
                char **str = separ(list[i], ">");   //A > B 分成 str[0] = A, str[1] = B
                char *reg = off(str[1]);
                int out_fd;

                command = separ(str[0], " ");
                if((out_fd = open(reg, (O_RDWR | O_CREAT), 0666)) == -1)
                    printf("檔案開啟失敗\n");
                creat_proc(command, in_fd, out_fd, close_fd, pipe_max, pipe_fd);  

                close(out_fd);
                free(str);
            }
            else
            {
                int jumpp;

                if(list[i + 1] != NULL && list[i + 2] == NULL && (jumpp = ispipe_out(list[i + 1])))
                {
                    struct data n = que->read(que, jumpp);

                    if(!n.pip[0] && !n.pip[1])
                        pipe(n.pip);
                    out_fd = n.pip[1];
                    que->wirte(que, n, jumpp);
                    command = separ(list[i], " ");
                    i++;
                }
                /*
                else if(list[i + 1] == NULL && (jumpp = ispipe_err(list[i])))
                {
                }
                */
                else 
                    command = separ(list[i], " ");
//                printf("%s\n in = %d out = %d c[0]%d c[1]%d\n"\
//                        , command[0], in_fd, out_fd, close_fd[0], close_fd[1]);
                creat_proc(command, in_fd, out_fd, close_fd, pipe_max, pipe_fd);
            }
           
            free(command);
        }
        if(stdin_fileno != STDIN_FILENO)
        {
            close(in.pip[0]);
            close(in.pip[1]);
            in.pip[0] = in.pip[1] = 0;
            que->wirte(que, in, 0);
        }
        for(int i = 0; i < pipe_max; i++)
        {
            close(pipe_fd[i][0]);
            close(pipe_fd[i][1]);
        }

        for(int i = 0; list[i] != NULL; i++)
        {
            int status;
            wait(&status);
        }

        free(list);
        free(input);
        que->addsp(que);
    }
    
    queue_del(&que);
    return 0;
}
