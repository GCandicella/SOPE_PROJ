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
    st_flags->dereference = false;
    st_flags->separate_dirs = false;
    st_flags->max_depth = -1;
    st_flags->block_size = -1;
    strcpy(st_flags->path , "./");
    return st_flags;
}

int get_blocks_bytes(const char* str)
{
    char num_str[MAX_FILE_NAME];
    int size = 0;
    for(int i = 0; str[i] != '\0'; i++)
    {
     
            if(str[i] == '\t')
                break;
            num_str[size] = str[i];
            size++;

    }
    return atoi(num_str);
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

void build_args(char* arg[], char* path, flags* st_flags)
{
    arg[0] = "./simpledu";
    arg[1] = path;
    int i = 2;
    if(st_flags->all)
    {
        arg[i] = "-a";
        i++;
    }
    if(st_flags->bytes)
    {
        arg[i] = "-b";
        i++;
    }
    if(st_flags->dereference)
    {
        arg[i] = "-L";
        i++;
    }
    if(st_flags->separate_dirs)
    {
        arg[i] = "-S";
        i++;
    }
    if(st_flags->block_size != -1)
    {
        char str[12];
        sprintf(str, "%d", st_flags->block_size);
        arg[i] = "-B ";
        strcat(arg[i],str);
        i++;
    }
    if(st_flags->max_depth != -1)
    {
        
        char str[12];
        sprintf(str, "%d", st_flags->max_depth);
        arg[i] = "--max-depth=";
        strcat(arg[i],str);
        i++;
    }
    arg[i] = NULL;
}

int process_dir(int argc, char *argv[]){
    flags* st_flags = createFlags();
    if(parseFlags(argc, argv, st_flags) != OK)
    {
        printf("Parameter error\n");
        return !OK;
    }

    int blocos = (st_flags->block_size == -1) ? 512 : st_flags->block_size; // Padrao STAT
    struct stat s;
    int somatorio = 0;

    if (stat(st_flags->path, &s) != 0){
        fprintf(stderr, "ERRO ao tentar obter stat de %s\n", st_flags->path);    
        return !OK;
    }    
    if(S_ISDIR(s.st_mode)){ // é um diretório
        somatorio += st_flags->bytes ? s.st_size : s.st_blocks*(blocos/BLOCOS_DU);
        int n = nArquivos(st_flags->path);
        char listadir[n][MAX_FILE_NAME];
        DIR* directory = opendir(st_flags->path);
        struct dirent *dir;
        for (int i = 0; i < n-2; i++) // n-2 compensa os casos ignorados
        {
            
            dir = readdir(directory);
            if( (strcmp(dir->d_name, "..")==0) || (strcmp(dir->d_name, ".")==0) ){
                i--;
                continue;
            }
            char new_item[MAX_FILE_NAME];
            strcpy(new_item, st_flags->path);
            strcat(new_item, dir->d_name); 
            strcpy(listadir[i], new_item); // vetor com os paths dos items
        }
        closedir (directory);
 
        // SOMAR BLOCOS DE CADA FICHEIRO DENTRO DE NOSSO DIRETORIO
        struct stat s_item;
        for (int i = 0; i < n-2; i++) // n-2 compensa os casos ignorados
        {
            if (stat(listadir[i], &s_item) != 0){
                fprintf(stderr, "ERRO ao tentar obter stat de %s\n", listadir[i]);   
                return !OK;
            }
            if( !S_ISDIR(s_item.st_mode) ){ // Arquivo Simples
                somatorio += st_flags->bytes ? s_item.st_size : s_item.st_blocks*(blocos/BLOCOS_DU);
                if(st_flags->all)
                {
                    int b = st_flags->bytes ? s_item.st_size : s_item.st_blocks*(blocos/BLOCOS_DU);
                    fprintf(stderr,"%d\t%s\n", b, listadir[i]);
                
                }
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
                    char *args[10];
                    build_args(args,new_path,st_flags); 
                    free(st_flags);
                    execvp(args[0],args); 
                    fprintf(stderr, "Erro no exec\n");
                }
                else
                { // Pai printa filho que esta no pipe
                    wait(&status);
                    close(filepipe[WRITE]);
                    dup2(filepipe[READ], STDIN_FILENO);
                    char buffer;
                    char msg[MAX_FILE_NAME];
                    int i = 0;
                    while (read(filepipe[0], &buffer, sizeof(buffer)) != 0)
                    {
                        msg[i] = buffer;
                        i++;
                        fprintf(stderr,"%c", buffer);
                    }
                    msg[i] = '\0';
                    if(!st_flags->separate_dirs){
                        somatorio   += get_blocks_bytes(msg);
                    }
                }
            }
        }
    }
    else{ // Arquivo individual
        somatorio = st_flags->bytes ? s.st_size : s.st_blocks*(blocos/BLOCOS_DU);
    }
    
        printf("%d\t%s\n", somatorio, st_flags->path );
    
    //printf("Size: %d\t%s\n", somarbytes, path);
    
    free(st_flags);
    return OK;
}

int main (int argc, char *argv[])
{
    process_dir(argc,argv);
}