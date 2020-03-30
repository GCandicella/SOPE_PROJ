#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <wait.h>
#ifndef SOPE_PROJ_MAIN_H
#define SOPE_PROJ_MAIN_H

#define MAX_FILE_NAME 1000
#define BLOCOS_DU 1024.0
#define OK 0
#define N_FLAGS 6
#define MAX_SIZE_FLAG 15

// PIPE defines
#define READ  0
#define WRITE 1


// Funcoes Auxiliares
int nArquivos(const char* name);
//int process_dir(char path[]);


#endif //SOPE_PROJ_MAIN_H
