#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/wait.h>
#include <stdbool.h>
#include <ctype.h>
#include <signal.h>
#include <math.h>
#ifndef SOPE_PROJ_MAIN_H
#define SOPE_PROJ_MAIN_H

#define MAX_FILE_NAME 1024
#define BLOCOS_DU 512.0
#define OK 0
#define UNDEFINED_FLAG -5

#define BACKUPSTDOUT "BackupStdout"

// PIPE defines
#define READ  0
#define WRITE 1

typedef struct flags{
	bool all;
	bool bytes;
	int block_size;
	bool dereference;
	bool separate_dirs;
	int max_depth;
	char path[MAX_FILE_NAME];
}flags;

// Funcoes Auxiliares
int nArquivos(const char* name);
//int process_dir(char path[]);

#endif //SOPE_PROJ_MAIN_H
