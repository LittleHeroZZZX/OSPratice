/*
 * @Author: zhouxin zzzx@hdu.edu.cn
 * @Date: 2022-10-25 12:21:57
 * @LastEditTime: 2022-10-28 13:33:03
 * @Software: VSCode
 */
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define EXIT_SUCCESS 0
#define EXIT_FAILURE 1

#define BUFFER_SIZE 1024
#define DELIMETERS " \t\n"


void print_prompt(void)
{
    printf("myshell> ");
}

char *read_line(void)
{
    char *line = NULL;
    size_t bufsize = 0;
    if (-1 == getline(&line, &bufsize, stdin))
    {
        free(line);
        perror("readline");
        exit(EXIT_FAILURE);
    }
    return line;
}

char **parse_line(char *line)
{
    int bufsize = BUFFER_SIZE, position = 0;
    char *token;
    char **tokens = (char **)malloc(BUFFER_SIZE * sizeof(char *));
    if (!tokens)
    {
        perror("allocation error");
        exit(EXIT_FAILURE);
    }
    token = strtok(line, DELIMETERS);
    while (token != NULL)
    {
        tokens[position] = token;
        position++;
        token = strtok(NULL, DELIMETERS);
        if (position >= BUFFER_SIZE)
        {
            tokens = (char **)realloc(tokens, bufsize * 2 * sizeof(char *));
            bufsize *= 2;
            if (!tokens)
            {
                perror("allocation error");
                exit(EXIT_FAILURE);
            }
        }
    }
    tokens[position] = NULL;
    return tokens;
}

int execute(char **args)
{
    pid_t pid, wpid;
    int status;
    if (args[0] == NULL)
        return 1;
    else if (strcmp(args[0], "exit") == 0)
        exit(EXIT_SUCCESS);
    else
    {
        pid = fork();
        if (pid == 0)
        {
            if (execvp(args[0], args) == -1)
            {
                perror("myshell");
            }
            exit(EXIT_FAILURE);
        }
        else if (pid < 0)
        {
            perror("myshell");
        }
        else
        {
            do
            {
                wpid = waitpid(pid, &status, WUNTRACED);
            } while (!WIFEXITED(status) && !WIFSIGNALED(status));
        }
    }
    
}

void sh_loop(void)
{
    char *line;
    char **args;
    while (1) {
        print_prompt();
        line = read_line();
        args = parse_line(line);
        execute(args);
        free(line);
        free(args);
    }

}



int main(){
    sh_loop();

}