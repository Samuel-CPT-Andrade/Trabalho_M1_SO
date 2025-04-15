#ifndef BANCO_H
#define BANCO_H

#define MAX_NOME 50
#define MAX_REQUISICAO 100

typedef struct {
    int id;
    char nome[MAX_NOME];
} Registro;

#endif
