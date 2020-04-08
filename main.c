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

flags* createFlags()
{
    flags* st_flags;
    st_flags = malloc(sizeof(flags));
    st_flags->all = false;
    st_flags->bytes = false;
    st_flags->count_links = false;
    st_flags->dereference = false;
    st_flags->separate_dirs = false;
    st_flags->max_depth = -1;
    strcpy(st_flags->path , "./");
    return st_flags;
}

bool numStr(char* str)
{
    for(int i = 0; str[i] != '\0'; i++)
    {
        if(!isdigit(str[i])) return false;
    }
    return true;
}

int parseFlags(int argc, char *argv[], flags* st_flags)
{
    for(int i = 1; i < argc; i++)
    {
        if((strcmp(argv[i],"-a") == 0) || (strcmp(argv[i],"--all") == 0))
        {
            st_flags->all = true;
        }
        else if((strcmp(argv[i],"-b") == 0) || (strcmp(argv[i],"--bytes") == 0))
        {
            st_flags->bytes = true;
        }
        else if((strcmp(argv[i],"-l") == 0) || (strcmp(argv[i],"--count-links") == 0))
        {
            st_flags->count_links = true;
        }
        else if((strcmp(argv[i],"-L") == 0) || (strcmp(argv[i],"--dereference") == 0))
        {
            st_flags->dereference = true;
        }
        else if((strcmp(argv[i],"-S") == 0) || (strcmp(argv[i],"--separate-dirs") == 0))
        {
            st_flags->separate_dirs = true;
        }
        else if(strcmp(argv[i],"-B") == 0)
        {
            if(numStr(argv[i+1]))
            {
                st_flags->block_size = atoi(argv[i+1]);
            }
            else return !OK;
            i++;
        }
        else if(strncmp(argv[i],"--block-size=",13) == 0)
        {
            if(numStr(argv[i]+13))
            {
                st_flags->block_size = atoi(argv[i]+13);
            }
            else return !OK;
        }
        else if(strncmp(argv[i],"--max-depth=",12) == 0)
        {
            if(numStr(argv[i]+12))
            {
                st_flags->max_depth = atoi(argv[i]+12);
            }
            else return !OK;
        }
        else
        {
            strcpy(st_flags->path,argv[i]);
            struct stat s;
            stat(argv[i], &s);
            if(S_ISDIR(s.st_mode)){
                if(st_flags->path[strlen(st_flags->path)-1] != '/')
                    strcat(st_flags->path, "/"); // Add barra no final path/...
            }
       }
    }
    return OK;
}

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

    if (stat(path, &s) != 0){
        fprintf(stderr, "ERRO ao tentar obter stat de %s\n", path);    
        return !OK;
    }    
    if(S_ISDIR(s.st_mode)){ // é um diretório
        somarblocos += s.st_blocks*(blocos/BLOCOS_DU);
        somarbytes  += s.st_size;

        fprintf(stderr,"Somando dir base -----> %s\tPID %d\n",  path,  getpid());
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
 
        // SOMAR BLOCOS DE CADA FICHEIRO DENTRO DE NOSSO DIRETORIO
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

                fprintf(stderr,"Somando not dir -----> %s\tPID %d\n",  listadir[i], getpid());
            }
            else{ // E' um subdiretorio
                pid_t pid;
                int status;
                int filepipe[2];
                if(pipe(filepipe)  == -1)   {perror("Pipe Error"); exit(1);}
                if ((pid = fork()) == -1)   {perror("Fork Error"); exit(1);}
                else if(pid == 0){ //Filho investiga subdir
                    // CHECK MAX DEPTH (SE > 0, DECREMENTAR)
                    close(filepipe[READ]);
                    dup2(filepipe[WRITE], STDOUT_FILENO);
                    //dup2(filepipe[WRITE], STDERR_FILENO);
                    char new_path[MAX_FILE_NAME*n];
                    strcpy(new_path, listadir[i]);
                    strcat(new_path, "/");
                    char *args[]={"./simpledu", new_path, NULL}; 
                    execvp(args[0],args); 
                    perror("Exec");
                }
                else
                { // Pai printa filho que esta no pipe
                    wait(&status);
                    close(filepipe[WRITE]);
                    dup2(filepipe[READ], STDIN_FILENO);
                    char buffer;
                    int r = 0;
                    read(filepipe[READ], &buffer, sizeof(buffer));
                    r = buffer - '0';
                    somarblocos += r;
                    printf("%d", r);
                    while (read(filepipe[0], &buffer, sizeof(buffer)) != 0)
                        printf("%c", buffer);
                }
            }
        }
    }
    else{ // Arquivo individual
        somarblocos = s.st_blocks*(blocos/BLOCOS_DU);
        somarbytes  = s.st_size;
        fprintf(stderr,"Somando not dir %s\tPID %d\n",  path, getpid());
            
    }
    printf("%d\t%s (PID %d)\n", somarblocos, path, getpid() );
    //printf("Size: %d\t%s\n", somarbytes, path);
    
    return OK;
}

int main (int argc, char *argv[])
{
    flags* st_flags = createFlags();
    if(parseFlags(argc, argv, st_flags) != OK)
    {
        printf("Parameter error\n");
    }
    process_dir(st_flags->path);
    free(st_flags);
}