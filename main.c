#include <stdio.h>

// Simular comando 'du'
// Cada processo um diretório (subdiretórios => processos filhos)
// Usar pipes para comunicação entre processos

#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>

int main (int argc, char *argv[])
{
    int i;
    struct stat s;

    if (argc < 2){
        fprintf(stderr, "ERRO: faltou parametro\n");
        return(1);
    }

    for (i = 1; i< argc; i++){
        if (stat(argv[i], &s)){
            fprintf(stderr, "ERRO ao tentar obter stat de %s\n", argv[i]);
            continue;
        }
        printf("===> %s\n", argv[i]);
        printf("\tTamanho = %lli\n", s.st_size);
        printf("\tUltimo acesso = %li\n", s.st_atime);
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
