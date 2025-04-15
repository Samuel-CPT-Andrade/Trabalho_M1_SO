#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include "banco.h"

Registro banco[100];
int total = 0;
pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

void salvarBanco() {
    FILE* f = fopen("banco.txt", "w");
    for (int i = 0; i < total; i++) {
        fprintf(f, "%d %s\n", banco[i].id, banco[i].nome);
    }
    fclose(f);
}

void carregarBanco() {
    FILE* f = fopen("banco.txt", "r");
    if (!f) return;
    while (fscanf(f, "%d %s", &banco[total].id, banco[total].nome) == 2) {
        total++;
    }
    fclose(f);
}

void* tratarRequisicao(void* arg) {
    char* req = (char*) arg;
    char comando[10];
    int id;
    char nome[MAX_NOME];

    if (strncmp(req, "INSERT", 6) == 0) {
        sscanf(req, "%s %d %s", comando, &id, nome);
        pthread_mutex_lock(&mutex);
        banco[total].id = id;
        strncpy(banco[total].nome, nome, MAX_NOME);
        total++;
        salvarBanco();
        pthread_mutex_unlock(&mutex);
        printf("[INSERIDO] id=%d nome=%s\n", id, nome);
    }
    else if (strncmp(req, "DELETE", 6) == 0) {
        sscanf(req, "%s %d", comando, &id);
        pthread_mutex_lock(&mutex);
        int i;
        for (i = 0; i < total; i++) {
            if (banco[i].id == id) {
                for (int j = i; j < total - 1; j++) {
                    banco[j] = banco[j + 1];
                }
                total--;
                salvarBanco();
                printf("[REMOVIDO] id=%d\n", id);
                break;
            }
        }
        if (i == total) {
            printf("[ERRO] ID %d não encontrado\n", id);
        }
        pthread_mutex_unlock(&mutex);
    }
    else if (strncmp(req, "SELECT", 6) == 0) {
        sscanf(req, "%s %d", comando, &id);
        pthread_mutex_lock(&mutex);
        int encontrado = 0;
        for (int i = 0; i < total; i++) {
            if (banco[i].id == id) {
                printf("[SELECT] id=%d nome=%s\n", banco[i].id, banco[i].nome);
                encontrado = 1;
                break;
            }
        }
        if (!encontrado) {
            printf("[ERRO] ID %d não encontrado\n", id);
        }
        pthread_mutex_unlock(&mutex);
    }
    else if (strncmp(req, "UPDATE", 6) == 0) {
        sscanf(req, "%s %d %s", comando, &id, nome);
        pthread_mutex_lock(&mutex);
        int encontrado = 0;
        for (int i = 0; i < total; i++) {
            if (banco[i].id == id) {
                strncpy(banco[i].nome, nome, MAX_NOME);
                salvarBanco();
                printf("[ATUALIZADO] id=%d novo_nome=%s\n", id, nome);
                encontrado = 1;
                break;
            }
        }
        if (!encontrado) {
            printf("[ERRO] ID %d não encontrado para atualização\n", id);
        }
        pthread_mutex_unlock(&mutex);
    }
    else {
        printf("[ERRO] Comando inválido: %s", req);
    }

    free(req);
    return NULL;
}

int main() {
    carregarBanco();
    char linha[MAX_REQUISICAO];
    while (fgets(linha, sizeof(linha), stdin)) {
        char* req = strdup(linha);
        pthread_t thread;
        pthread_create(&thread, NULL, tratarRequisicao, req);
        pthread_detach(thread);
    }
    return 0;
}
