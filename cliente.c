#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <windows.h>  // API do Windows para pipes e processos
#include "banco.h"

int main() {
    HANDLE hReadPipe, hWritePipe;
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    // 1. Criar um pipe anônimo para comunicação
    if (!CreatePipe(&hReadPipe, &hWritePipe, &sa, 0)) {
        fprintf(stderr, "[ERRO] Falha ao criar pipe\n");
        return 1;
    }

    // 2. Configurar o processo filho (servidor)
    STARTUPINFO si = { sizeof(STARTUPINFO) };
    PROCESS_INFORMATION pi;

    // Redirecionar a entrada padrão (STDIN) do filho para o pipe
    si.hStdInput = hReadPipe;
    si.hStdOutput = GetStdHandle(STD_OUTPUT_HANDLE);
    si.hStdError = GetStdHandle(STD_ERROR_HANDLE);
    si.dwFlags = STARTF_USESTDHANDLES;

    // 3. Criar o processo filho (servidor)
    if (!CreateProcess(
        NULL,                   // Nome do executável (NULL = usar linha de comando)
        "servidor.exe",         // Comando (servidor.exe deve estar no mesmo diretório)
        NULL,                   // Atributos de segurança do processo
        NULL,                   // Atributos de segurança da thread
        TRUE,                   // Herdar handles (incluindo o pipe)
        0,                      // Flags de criação (0 = padrão)
        NULL,                   // Ambiente (herdar do pai)
        NULL,                   // Diretório de trabalho (herdar do pai)
        &si,                   // STARTUPINFO
        &pi                    // PROCESS_INFORMATION
    )) {
        fprintf(stderr, "[ERRO] Falha ao criar processo servidor\n");
        CloseHandle(hReadPipe);
        CloseHandle(hWritePipe);
        return 1;
    }

    // 4. Processo pai (cliente) envia comandos pelo pipe
    CloseHandle(hReadPipe);  // O pai não usa a extremidade de leitura

    printf("Digite comandos (INSERT, DELETE, SELECT, UPDATE):\n");

    char linha[MAX_REQUISICAO];
    DWORD bytesEscritos;

    while (fgets(linha, sizeof(linha), stdin)) {
        if (!WriteFile(hWritePipe, linha, strlen(linha), &bytesEscritos, NULL)) {
            fprintf(stderr, "[ERRO] Falha ao escrever no pipe\n");
            break;
        }
    }

    // 5. Fechar tudo
    CloseHandle(hWritePipe);
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    return 0;
}