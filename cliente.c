#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "banco.h"

int main() {
    int fd[2];
    if (pipe(fd) == -1) {
        perror("pipe");
        exit(1);
    }

    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        exit(1);
    }

    if (pid == 0) { // processo filho: servidor
        close(fd[1]);
        dup2(fd[0], STDIN_FILENO);
        close(fd[0]);
        execl("./servidor", "servidor", NULL);
        perror("execl");
        exit(1);
    } else { // processo pai: cliente
        close(fd[0]);
        FILE* stream = fdopen(fd[1], "w");
        char linha[MAX_REQUISICAO];
        printf("Digite comandos (INSERT, DELETE, SELECT, UPDATE):\n");
        while (fgets(linha, sizeof(linha), stdin)) {
            fprintf(stream, "%s", linha);
            fflush(stream);
        }
        fclose(stream);
    }
    return 0;
}
