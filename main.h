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


/**
 *
 * @param name - string to check if it is a dir or just a file
 * @return - number of files if it is a dir, -1 otherwise
 */
int isDir(const char* name);

#endif //SOPE_PROJ_MAIN_H
