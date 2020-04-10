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

flags* createFlags()
{
    flags* st_flags;
    st_flags = malloc(sizeof(flags));
    st_flags->all = false;
    st_flags->bytes = false;
    st_flags->dereference = false;
    st_flags->separate_dirs = false;
    st_flags->max_depth = UNDEFINED_FLAG;
    st_flags->block_size = UNDEFINED_FLAG;
    strcpy(st_flags->path , "./");
    return st_flags;
}

bool numStr(char* str)
{
    for(int i = 0; str[i] != '\0'; i++)
    {
        if(!isdigit(str[i]) && str[i] != '-') return false;
    }
    return true;
}

bool logfile_write(const char *evento, const char * info){
    struct timespec requestTime;
    clock_gettime(CLOCK_MONOTONIC_RAW, &requestTime);

    char mensagem[MAX_FILE_NAME];
    //instant – pid – action – info
    sprintf(mensagem, "%.2f - %d - %s - %s\n", (requestTime.tv_nsec - atoi(getenv(STARTTIME)))/1000000.0 , getpid(), evento, info);
    write(atoi(getenv(LOG_DESC)), mensagem, strlen(mensagem));
    return EXIT_SUCCESS;
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
            else return EXIT_FAILURE;
            i++;
        }
        else if(strncmp(argv[i],"--block-size=",13) == 0)
        {
            if(numStr(argv[i]+13))
            {
                st_flags->block_size = atoi(argv[i]+13);
            }
            else return EXIT_FAILURE;
        }
        else if(strncmp(argv[i],"--max-depth=",12) == 0)
        {
            if(numStr(argv[i]+12))
            {
                st_flags->max_depth = atoi(argv[i]+12);
            }
            else return EXIT_FAILURE;
        }
        else
        {
            strcpy(st_flags->path,argv[i]);
        }
    }
    return EXIT_SUCCESS;
}

int nArquivos(const char* name)
{
    DIR* directory = opendir(name);
    int n = 0;
    if(errno == ENOTDIR)
        return EXIT_SUCCESS;
    if(directory != NULL)
    {
        struct dirent *dir;
        while ((dir = readdir (directory)) != NULL){
            n++;
        }
        closedir (directory);
        return n;
    }
    return EXIT_SUCCESS;
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
    if(st_flags->block_size != UNDEFINED_FLAG)
    {
        arg[i] = "-B";
        i++;
        int length = snprintf( NULL, 0, "%d", st_flags->block_size);
        arg[i] = malloc(length + 1);
        snprintf( arg[i], length + 1, "%d", st_flags->block_size );
        i++;
    }
    if(st_flags->max_depth != UNDEFINED_FLAG)
    {
        int length = snprintf( NULL, 0, "%d", st_flags->max_depth);
        arg[i] = malloc(length + 13);
        snprintf( arg[i], length + 13, "--max-depth=%d", st_flags->max_depth );    
        i++;
    }
    arg[i] = NULL;
}

void sigint_handler(int signo){
    char c;
    logfile_write("RECV_SIGNAL", "SIGINT");
    if( atoi(getenv(PROCESS_GRP)) != getpid() ){
        char aux[255];
        sprintf(aux, "SIGSTOP (%d)", getpid());
        logfile_write("SEND_SIGNAL", aux);
        raise(SIGSTOP);
    }
    if( atoi(getenv(PROCESS_GRP)) == getpid() ){
        while(1){
            write(atoi(getenv(BACKUPSTDOUT)), "\nDeseja Encerrar(Y/n)? ", 22);
            read(STDIN_FILENO, &c, sizeof(c));
            if(c == 'Y' || c == 'y'){
                char aux[255];
                sprintf(aux, "SIGTERM (%d)", atoi(getenv(PROCESS_GRP)));
                logfile_write("SEND_SIGNAL", aux);
                kill(-atoi(getenv(PROCESS_GRP)), SIGTERM);    
                break;
            }
            else
            {
                char aux[255];
                sprintf(aux, "SIGCONT (%d)", atoi(getenv(PROCESS_GRP)));
                logfile_write("SEND_SIGNAL", aux);
                kill(-atoi(getenv(PROCESS_GRP)), SIGCONT); 
                break;
            }
        }
    }
}

void set_sinal(){
    struct sigaction intAction;
    intAction.sa_handler = sigint_handler;
    sigemptyset(&intAction.sa_mask);
    intAction.sa_flags = 0;

    if(sigaction(SIGINT, &intAction, NULL) < 0){
        fprintf(stderr, "Erro ao inicializar o sinal de interrupcao");
    }
}

int process_dir(int argc, char *argv[]){
    flags* st_flags = createFlags();
    if(parseFlags(argc, argv, st_flags) != EXIT_SUCCESS)
    {
        write(STDERR_FILENO, "Parameter error\n", 16);
        return EXIT_FAILURE;
    }
    float blocos = (st_flags->block_size == UNDEFINED_FLAG) ? 1024 : st_flags->block_size; // Padrao STAT
    struct stat s;
    float somatorio = 0;

    if (lstat(st_flags->path, &s) != 0){
        fprintf(stderr, "ERRO ao tentar obter stat de %s\n", st_flags->path);    
        return EXIT_FAILURE;
    }
    char path_copy[MAX_FILE_NAME]; strcpy(path_copy, st_flags->path );  
    if(S_ISBLK(s.st_mode) && st_flags->dereference){}
    else if(S_ISDIR(s.st_mode)){ // é um diretório simples
        if(st_flags->path[strlen(st_flags->path)-1] != '/'){
            strcat(st_flags->path, "/"); // Add barra no final path/...
        }
        somatorio += st_flags->bytes ? s.st_size : s.st_blocks*(BLOCOS_DU/blocos);
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
            
            if (lstat(listadir[i], &s_item) != 0){
                fprintf(stderr, "ERRO ao tentar obter stat de %s\n", listadir[i]);   
                return EXIT_FAILURE;
            }
            if(S_ISBLK(s_item.st_mode) && st_flags->dereference) continue;
            else if( !S_ISDIR(s_item.st_mode) ){ // Arquivo Simples
                somatorio += st_flags->bytes ? s_item.st_size : s_item.st_blocks*(BLOCOS_DU/blocos);
                if(st_flags->all)
                {
                    int b = st_flags->bytes ? s_item.st_size : s_item.st_blocks*(BLOCOS_DU/blocos);
                    fprintf(stderr,"%.0f\t%s\n", ceil(b), listadir[i]);
                
                }
            }
            else{ // E' um subdiretorio
                pid_t pid;
                int status;
                int filepipe[2];
                if(pipe(filepipe)  == -1)   {perror("Pipe Error"); exit(1);}
                if ((pid = fork()) == -1)   {perror("Fork Error"); exit(1);}
                else if(pid == 0){ //Filho investiga subdir
                    close(filepipe[READ]);
                    dup2(filepipe[WRITE], STDOUT_FILENO);
                    setpgid(getpid(), atoi(getenv(PROCESS_GRP)));
                    char new_path[MAX_FILE_NAME*n];
                    strcpy(new_path, listadir[i]);
                    strcat(new_path, "/");
                    char* args[11];
                    if(st_flags->max_depth >= 0)st_flags->max_depth--;
                    build_args(args,new_path,st_flags); 
                    free(st_flags);
                    execvp(args[0],args); 
                    fprintf(stderr, "Erro no exec\n");
                }
                else
                { // Pai printa filho que esta no pipe
                    wait(&status);
                    close(filepipe[WRITE]);
                    float buffer;
                    read(filepipe[READ], &buffer, sizeof(buffer));
                    char aux[255];
                    sprintf(aux, "%.2f", buffer);
                    logfile_write("RECV_PIPE", aux);
                    if(!st_flags->separate_dirs){
                        somatorio += buffer;
                    }
                }
            }
        }
    }
    else{ // Arquivo individual
        somatorio = st_flags->bytes ? s.st_size : s.st_blocks*(BLOCOS_DU/blocos);
    }

    if(getpid() == atoi(getenv(PROCESS_GRP))){ // Escreve paizao 
            char msgem[MAX_FILE_NAME];
            strcpy(msgem, path_copy);
            if(msgem[strlen(msgem)-1] == '/'){
                msgem[strlen(msgem)-1] = '\0';
            }
            printf("%.0f\t%s\n", ceil(somatorio), path_copy);
    }
    else{ //  Escreve filho (pipe e tela)
        write(STDOUT_FILENO, &somatorio, sizeof(somatorio)); // Escreve no pipe
        char aux[255];
        sprintf(aux, "%.2f", somatorio);
        logfile_write("SEND_PIPE", aux);
        if(st_flags->max_depth == UNDEFINED_FLAG || st_flags->max_depth >= 0){ // Escreve filho na tela     
            char msgem[MAX_FILE_NAME];
            sprintf(msgem, "%.0f", ceil(somatorio)); 
            strcat(msgem, "\t");
            strcat(msgem, st_flags->path);
            strcat(msgem, "\n");
            if(msgem[strlen(msgem)-2] == '/'){
                msgem[strlen(msgem)-2] = '\n';
                msgem[strlen(msgem)-1] = '\0';
            }
            write(atoi(getenv(BACKUPSTDOUT)), msgem, strlen(msgem));
        }
    }
    
    free(st_flags);
    return EXIT_SUCCESS;
}

int main (int argc, char *argv[])
{
    
    char pg[255];
    sprintf(pg, "%d", getpid());  
    setenv(PROCESS_GRP, pg, 0);
    set_sinal();

    int stdoutbackup = dup(STDOUT_FILENO);
    char stdoutbackupaux[2];
    sprintf(stdoutbackupaux, "%d", stdoutbackup);
    setenv(BACKUPSTDOUT, stdoutbackupaux, 0 );

    int fd;
    setenv(LOGFILE, "output.log", 0 ); // define output.log como env para logfile se nao existir
    if(getpid() == atoi(getenv(PROCESS_GRP)))
        fd = open(getenv(LOGFILE), O_WRONLY | O_CREAT | O_TRUNC | O_APPEND , S_IWUSR | S_IRUSR);
    else
        fd = open(getenv(LOGFILE), O_WRONLY | O_CREAT | O_APPEND , S_IWUSR | S_IRUSR);
    char fdchar[2];
    sprintf(fdchar, "%d", fd);
    setenv(LOG_DESC, fdchar, 0);
    //logfile_write("CREATE", "oi");

    struct timespec requireTime;
    char time_nsec[255];
    clock_gettime(CLOCK_MONOTONIC_RAW, &requireTime);
    sprintf(time_nsec, "%ld", requireTime.tv_nsec);
    setenv(STARTTIME, time_nsec, 0);

    process_dir(argc,argv);

    return EXIT_SUCCESS;
}