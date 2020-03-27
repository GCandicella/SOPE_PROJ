#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include "main.h"

// Simular comando 'du'
// Cada processo um diretório (subdiretórios => processos filhos)
// Usar pipes para comunicação entre processos

/*
 * Campos obtidos do man2 stat :
        st_dev ID of device containing file
        st_ino inode number
        st_mode protection
        st_nlink number of hard links
        st_uid user ID of owner
        st_gid group ID of owner
        st_rdev device ID (if special file)
        st_size total size, in bytes
        st_blksize blocksize for filesystem I/O
        st_blocks number of blocks allocated
        st_atime time of last access
        st_mtime time of last modification
        st_ctime time of last status change
*/

int nArquivos(const char* name)
{
    DIR* directory = opendir(name);
    if(errno == ENOTDIR)
        return !OK;
    if(directory != NULL)
    {
        struct dirent *dir;
        int nArquivos = 0;
        while ((dir = readdir(directory)) != NULL)
        {
            if( strcmp(dir->d_name, "..") != 0 && strcmp(dir->d_name, ".") != 0)
                nArquivos++;
        }
        closedir(directory);
        return nArquivos;
    }
    return !OK;
}

int main (int argc, char *argv[])
{
    int blocos = 512;
    struct stat s;
    char path[MAX_FILE_NAME];
    
    if (argc < 2){
        strcpy(path,"./");
    }
    else{
        strcpy( path , argv[1] );
    }

    if (stat(path, &s)){
        fprintf(stderr, "ERRO ao tentar obter stat de %s\n", path);    
        return !OK;
    }    
    if(S_ISDIR(s.st_mode)){ // é um diretório
        int n = nArquivos(path) ;
        DIR *d;
        d = opendir(path);
        struct dirent *dir;
        for(int i = 0; i < n ; i++){
            dir = readdir(d);
            if( strcmp(dir->d_name, "..") == 0 || strcmp(dir->d_name, ".") == 0)
                continue;
            printf("File %d: %s\n", i, dir->d_name);
        }
        closedir(d);
    }

    printf("===> %s\n", path);
    printf("\tBlocos = %ld\n",s.st_blocks);
    printf("The file %s a symbolic link\n", (S_ISLNK(s.st_mode)) ? "is" : "is not");

    return OK;
}
