#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>  // Biblioteca principal do Windows para threads e mutex
#include "banco.h"

Registro banco[100];
int total = 0;
CRITICAL_SECTION cs;  // Substitui o pthread_mutex_t

void salvarBanco() {
    EnterCriticalSection(&cs);
    FILE* f = fopen("./banco.txt", "w");
    if (!f) {
        printf("[ERRO] Falha ao abrir arquivo para escrita\n");
        LeaveCriticalSection(&cs);
        return;
    }
    for (int i = 0; i < total; i++) {
        fprintf(f, "%d %s\n", banco[i].id, banco[i].nome);
    }
    fclose(f);
    LeaveCriticalSection(&cs);
}

void carregarBanco() {
    EnterCriticalSection(&cs);
    FILE* f = fopen("banco.txt", "r");
    if (!f) {
        LeaveCriticalSection(&cs);
        return;
    }
    while (fscanf(f, "%d %s", &banco[total].id, banco[total].nome) == 2) {
        total++;
    }
    fclose(f);
    LeaveCriticalSection(&cs);
}

DWORD WINAPI tratarRequisicao(LPVOID arg) {
    char* req = (char*)arg;
    char comando[10];
    int id;
    char nome[MAX_NOME];

    if (strncmp(req, "INSERT", 6) == 0) {
        sscanf(req, "%s %d %s", comando, &id, nome);
        EnterCriticalSection(&cs);
        
        // Verificar se o ID já existe no banco
        int existe = 0;
        for (int i = 0; i < total; i++) {
            if (banco[i].id == id) {
                existe = 1;
                break;
            }
        }
        
        if (!existe) {
            banco[total].id = id;
            strncpy(banco[total].nome, nome, MAX_NOME);
            total++;
            salvarBanco();
            printf("[INSERIDO] id=%d nome=%s\n", id, nome);
        } else {
            printf("[ERRO] ID %d ja existe. Insercao nao realizada.\n", id);
        }
        
        LeaveCriticalSection(&cs);
    }
    else if (strncmp(req, "DELETE", 6) == 0) {
        sscanf(req, "%s %d", comando, &id);
        EnterCriticalSection(&cs);
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
            printf("[ERRO] ID %d nao encontrado\n", id);
        }
        LeaveCriticalSection(&cs);
    }
    else if (strncmp(req, "SELECT", 6) == 0) {
        sscanf(req, "%s %d", comando, &id);
        EnterCriticalSection(&cs);
        int encontrado = 0;
        for (int i = 0; i < total; i++) {
            if (banco[i].id == id) {
                printf("[SELECT] id=%d nome=%s\n", banco[i].id, banco[i].nome);
                encontrado = 1;
                break;
            }
        }
        if (!encontrado) {
            printf("[ERRO] ID %d nao encontrado\n", id);
        }
        LeaveCriticalSection(&cs);
    }
    else if (strncmp(req, "UPDATE", 6) == 0) {
        sscanf(req, "%s %d %s", comando, &id, nome);
        EnterCriticalSection(&cs);
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
            printf("[ERRO] ID %d nao encontrado para atualizacao\n", id);
        }
        LeaveCriticalSection(&cs);
    }
    else {
        printf("[ERRO] Comando invalido: %s", req);
    }

    free(req);
    return 0;
}

int main() {
    InitializeCriticalSection(&cs);  // Inicializa a seção crítica (equivalente ao mutex)
    carregarBanco();

    char linha[MAX_REQUISICAO];
    while (fgets(linha, sizeof(linha), stdin)) {
        char* req = _strdup(linha);  // strdup no Windows pode exigir _strdup
        HANDLE hThread = CreateThread(
            NULL,                   // Atributos de segurança padrão
            0,                      // Tamanho padrão da pilha
            tratarRequisicao,       // Função da thread
            (LPVOID)req,            // Argumento (requisição)
            0,                      // Flags de criação (0 = execução imediata)
            NULL                    // ID da thread (não necessário)
        );

        if (hThread == NULL) {
            fprintf(stderr, "[ERRO] Falha ao criar thread\n");
            free(req);
        } else {
            CloseHandle(hThread);  // Fecha o handle (a thread continua executando)
        }
    }

    DeleteCriticalSection(&cs);  // Libera a seção crítica
    return 0;
}