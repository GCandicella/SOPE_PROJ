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
    int n = 0;
    if(errno == ENOTDIR)
        return !OK;
    if(directory != NULL)
    {
        struct dirent *dir;
        while ((dir = readdir (directory)) != NULL){
            n++;
        }
        closedir (directory);
        return n;
    }
    return !OK;
}

int main (int argc, char *argv[])
{
    int blocos = 512; // Padrao STAT
    struct stat s;
    char path[MAX_FILE_NAME];
    int somarblocos = 0;
    int somarbytes = 0;
    
    if (argc < 2){
        strcpy(path,".");
    }
    else{
        strcpy( path , argv[1] );
    }

    if (stat(path, &s)){
        fprintf(stderr, "ERRO ao tentar obter stat de %s\n", path);    
        return !OK;
    }    
    if(S_ISDIR(s.st_mode)){ // é um diretório
        somarblocos += s.st_blocks*(blocos/BLOCOS_DU);
        somarbytes  += s.st_size;
        int n = nArquivos(path);
        char listadir[n][MAX_FILE_NAME];
        DIR* directory = opendir(path);
        struct dirent *dir;
        for (int i = 0; i < n-2; i++) // n-2 compensa os casos ignorados
        {
            dir = readdir(directory);
            if( (strcmp(dir->d_name, "..")==0) || (strcmp(dir->d_name, ".")==0) ){
                i--;
                continue;
            }
            strcpy(listadir[i], dir->d_name);
        }
        closedir (directory);

        // SOMAR BLOCOS
        struct stat s_item;
        for (int i = 0; i < n-2; i++) // n-2 compensa os casos ignorados
        {
            if (stat(listadir[i], &s_item)){
                fprintf(stderr, "ERRO ao tentar obter stat de %s\n", path);    
                return !OK;
            }
            if( !S_ISDIR(s_item.st_mode) ){ // Arquivo Simples
                somarblocos += s_item.st_blocks*(blocos/BLOCOS_DU);
                somarbytes  += s_item.st_size;
            }
            else{ // E' um subdiretorio
                if(fork() > 0){
                    char *argv_sub[n];
                    argv_sub[0] = "./simpledu";
                    argv_sub[1] = listadir[i];
                    main(argc, argv_sub);
                }
                else{
                    wait(NULL);
                }
            }
        }
    }
    else{ // Arquivo individual
        somarblocos = s.st_blocks*(blocos/BLOCOS_DU);
        somarbytes  = s.st_size;
    }
    printf("Blocos: %d\t%s\n", somarblocos, path);
    printf("Size: %d\t%s\n", somarbytes, path);
    
    return OK;
}
