#include "main.h"

// Simular comando 'du'
// Cada processo um diretório (subdiretórios => processos filhos)
// Usar pipes para comunicação entre processos

/**
 * TODO
 * Diretorio com o simpledu sem path corre forever
 * ./.git/logs/HEAD com erro na funcao STAT, o que fazer?
 * Erro em alguns pipe (observar print no processo pai)
 * FLAGSSSSS
*/

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

int process_dir(char path[]){
    int blocos = 512; // Padrao STAT
    struct stat s;
    int somarblocos = 0;
    int somarbytes = 0;
    

    //printf("Path: %s\n", path);

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
            char new_item[MAX_FILE_NAME];
            strcpy(new_item, path);
            strcat(new_item, dir->d_name); 
            strcpy(listadir[i], new_item); // vetor com os paths dos items
        }
        closedir (directory);

        // SOMAR BLOCOS 
        struct stat s_item;
        for (int i = 0; i < n-2; i++) // n-2 compensa os casos ignorados
        {
            if (stat(listadir[i], &s_item)){
                fprintf(stderr, "ERRO ao tentar obter stat de %s\n", listadir[i]);    
                return !OK;
            }
            if( !S_ISDIR(s_item.st_mode) ){ // Arquivo Simples

                somarblocos += s_item.st_blocks*(blocos/BLOCOS_DU);
                somarbytes  += s_item.st_size;
            }
            else{ // E' um subdiretorio
                int fd[2];
                if(pipe(fd) < 0){
                    fprintf(stderr, "Pipe Error");
                    return !OK;
                }
                pid_t pid = fork();
                if(pid == 0){ //Filho investiga subdir
                    // CHECK MAX DEPTH (SE PROCESSAR, DECREMENTAR)
                    close(fd[READ]);
                    char new_path[MAX_FILE_NAME*n];
                    strcpy(new_path, listadir[i]);
                    strcat(new_path, "/");
                    int r = process_dir(new_path);
                    write(fd[WRITE], &r, sizeof(r));
                    close(fd[WRITE]);
                    return r + somarblocos;
                }
                else
                { // Pai aguarda filho printar
                    close(fd[WRITE]);
                    wait(NULL);
                    int r;
                    read(fd[READ], &r, sizeof(r));
                    close(fd[READ]);
                    //printf("Pai recebe %d\n", r);
                    somarblocos += r;
                }
            }
        }
    }
    else{ // Arquivo individual
        somarblocos = s.st_blocks*(blocos/BLOCOS_DU);
        somarbytes  = s.st_size;
    }
    printf("Blocos: %d\t%s\n", somarblocos, path);
    //printf("Size: %d\t%s\n", somarbytes, path);
    
    return somarblocos;
}

int main (int argc, char *argv[])
{
    char path[MAX_FILE_NAME];
    if (argc < 2){
        strcpy( path , "./");
    }
    else{
        strcpy( path , argv[1] );
        if(path[strlen(path)-1] != '/')
                strcat(path, "/"); // Add barra no final path/...
    }
    char flags[N_FLAGS][MAX_SIZE_FLAG] = {0};
    int pos = 0;
    //limpar o argv (flags, path, etc)
    for (int i = 0; i < argc; i++)
    {
        if(argv[i][0] == '-'){ // Caracateriza uma flag
            strcpy(flags[pos] , argv[i]);
            pos++;
        }
    }
    
    process_dir(path);
}