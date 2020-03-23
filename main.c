#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <dirent.h>
#include <errno.h>
#include <stdio.h>

// Simular comando 'du'
// Cada processo um diretório (subdiretórios => processos filhos)
// Usar pipes para comunicação entre processos

int isDir(const char* name)
{
    DIR* directory = opendir(name);
    if(directory != NULL)
    {
        closedir(directory);
        return 0; // É um diretório
    }
    if(errno == ENOTDIR)
    {
        return 1;
    }
    return -1;
}

int main (int argc, char *argv[])
{
    int i;
    struct stat s;

    if (argc < 2){
        fprintf(stderr, "ERRO: faltou parametro\n");
        return(1);
    }

    for (i = 1; i < argc; i++){
        if (stat(argv[i], &s)){
            fprintf(stderr, "ERRO ao tentar obter stat de %s\n", argv[i]);
            continue;
        }
        printf("===> %s\n", argv[i]);
        printf("\tBlocos = %lld\n",s.st_blocks);
        printf("\tTamanho = %lli bytes\n", s.st_size);
        printf("The file %s a symbolic link\n", (S_ISLNK(s.st_mode)) ? "is" : "is not");
        /* Outros campos, obtidos do man2 stat :
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
    }
}
