#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <assert.h>
#include <ctype.h>
#include <string.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <dirent.h>
#include <sys/mman.h>
#include <pwd.h>
#include <sys/wait.h>


#define MAX_COMM_LEN 10000
#define TOK_BUFSIZE 64
#define TOK_DELIM " \v\t\r\n\f "



// prints subdirectories within current directory
void printSubDirs() {
    DIR *dir;
    struct dirent *entry;
    char cwd[MAX_COMM_LEN];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if ((dir = opendir(cwd)) == NULL) {
            perror("opendir() error");
        }
        else {
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_DIR) {
                    write(1, entry->d_name, strlen(entry->d_name));
                    write(1, "\n", strlen("\n"));
                }
            }
            closedir(dir);
        }
    }
}


// prints files within current directory
void printFiles() {
    DIR *dir;
    struct dirent *entry;
    char cwd[MAX_COMM_LEN];

    if (getcwd(cwd, sizeof(cwd)) != NULL) {
        if ((dir = opendir(cwd)) == NULL) {
            perror("opendir() error");
        }
        else {
            while ((entry = readdir(dir)) != NULL) {
                if (entry->d_type == DT_REG) {
                    write(1, entry->d_name, strlen(entry->d_name));
                    write(1, "\n", strlen("\n"));
                }
            }
            closedir(dir);
        }
    }
}



// prints files within current directory
void clearShell() {
    int i;
    for (i = 0; i < 300; i++) {
        write(1, "\n", strlen("\n"));
    }
}


// the one and only error message
void errorMessage() {
    char error_message[30] = "An error has occured\n";
    write(STDERR_FILENO, error_message, strlen(error_message));
}




// check if the command is a built-in shell command
// if so, return 1, else return 0
int isBuiltIn( char *comm ) {

    // command against to determine action
    char pwd[MAX_COMM_LEN];
    char cd[MAX_COMM_LEN];
    char showDirs[MAX_COMM_LEN];
    char showFiles[MAX_COMM_LEN];
    char mkdirect[MAX_COMM_LEN];
    char touch[MAX_COMM_LEN];
    char clear[MAX_COMM_LEN];
    char exit[MAX_COMM_LEN];
    char cwd[MAX_COMM_LEN];

    // enter strings into commands
    strcpy(pwd, "pwd");
    strcpy(cd, "cd");
    strcpy(showDirs, "show-dirs");
    strcpy(showFiles, "show-files");
    strcpy(mkdirect, "mkdir");
    strcpy(clear, "clear");
    strcpy(exit, "exit");
    strcpy(touch, "touch");

    // check if built-in shell command
    if ((strcmp(comm, pwd) == 0) ||
        (strcmp(comm, cd) == 0) ||
        (strcmp(comm, showDirs) == 0) ||
        (strcmp(comm, showFiles) == 0) ||
        (strcmp(comm, mkdirect) == 0) ||
        (strcmp(comm, touch) == 0) ||
        (strcmp(comm, clear) == 0) ||
        (strcmp(comm, exit) == 0)) {
            return 1;
        } else {
            return 0;
        }
}


// runs the built-in commands
// interactive, batch, and main functions will detect 
// whether we have a built-in command and call this function if necessary
int runBuiltIn( char *comm, char *dirFile ) {
    int dirFilelen = strlen(dirFile);
    int exitMode = 0;

    // command against to determine action
    char pwd[MAX_COMM_LEN];
    char cd[MAX_COMM_LEN];
    char showDirs[MAX_COMM_LEN];
    char showFiles[MAX_COMM_LEN];
    char mkdirect[MAX_COMM_LEN];
    char touch[MAX_COMM_LEN];
    char clear[MAX_COMM_LEN];
    char exit[MAX_COMM_LEN];
    char cwd[MAX_COMM_LEN];

    // enter strings into commands
    strcpy(pwd, "pwd");
    strcpy(cd, "cd");
    strcpy(showDirs, "show-dirs");
    strcpy(showFiles, "show-files");
    strcpy(mkdirect, "mkdir");
    strcpy(clear, "clear");
    strcpy(exit, "exit");
    strcpy(touch, "touch");

    // process built-in shell commands
    if (strcmp(comm, pwd) == 0) { // pwd command
        if  (dirFilelen > 0) {
            errorMessage();
            exitMode = 1;
        }
        else if (getcwd(cwd, sizeof(cwd)) != NULL) {
            write(1, cwd, strlen(cwd));
            write(1, "\n", strlen("\n"));
        }
        else {
            errorMessage();
        }
    }
    else if (strcmp(comm, cd) == 0) { // cd command
        if  (dirFilelen == 0) {
            chdir(getenv("HOME"));
        }
        else {
            chdir(dirFile); 
        }
    }
    else if (strcmp(comm, showDirs) == 0) { // show-dirs command
        if  (dirFilelen > 0) {
            errorMessage();
            exitMode = 1;
        }
        else {
            printSubDirs(); 
        } 
    } 
    else if (strcmp(comm, showFiles) == 0) { // show-files command
        if  (dirFilelen > 0) {
            errorMessage();
            exitMode = 1;
        }
        else {
            printFiles();
        }
    } 
    else if (strcmp(comm, mkdirect) == 0) { // mkdir command
        if (dirFilelen < 1) {
            errorMessage();
            exitMode = 1;
        }
        else {
            struct stat st = {0};
            if (stat(dirFile, &st) == -1) {
                mkdir(dirFile, 0700);
            }
            else {
                errorMessage();
            }
        }
    } 
    else if (strcmp(comm, touch) == 0) { // touch command
        if (dirFilelen < 1) {
            errorMessage();
            exitMode = 1;
        }
        else {
            mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH;
            creat(dirFile, mode);
        }
    } 
    else if (strcmp(comm, clear) == 0) { // clear command
        if (dirFilelen > 0) {
            errorMessage();
            exitMode = 1;
        }
        else {
            clearShell();
        }
    } 
    else if (strcmp(comm, exit) == 0) { // exit command
        if (dirFilelen > 0) {
            errorMessage();
        }
        exitMode = 1;
    } 
    else {
        errorMessage();
    }

    return exitMode;
}


// get the number of tokens in a command
int getTokenCount ( char **tokens ) {
    int count = 0;
    while ( tokens[count] != NULL ) {
        count++;
    }
    return count;
}


// split the tokens into an array
char **splitTokens(char *commline) {
    int bufsize = TOK_BUFSIZE;
    int position = 0;

    // allocate iniital space
    char **tokens = malloc(bufsize * sizeof(char*));
    char *token;

    token = strtok(commline, TOK_DELIM);
    
    while (token != NULL) {
        
        tokens[position] = token;
        position++;

        // if out of space, reallocate memory
        if (position >= bufsize) {
          bufsize += TOK_BUFSIZE;
          tokens = realloc(tokens, bufsize * sizeof(char*));
        }

        token = strtok(NULL, TOK_DELIM);
    }
    tokens[position] = NULL;

    return tokens;
}


// split the tokens into an array
char **cleanTokens( char **tokens ) {
    int bufsize = TOK_BUFSIZE;
    char **newTokens = malloc(bufsize * sizeof(char*));
    int count = getTokenCount( tokens );
    // separate counters for new and old arrays
    int i = 0;
    int j = i;

    // set symbol for redirect
    char re[10];
    strcpy(re, ">");
    
    // symbol for background
    char back[10];
    strcpy(back, "&");


    while (i < count) {
        if ( strcmp(tokens[i], re) == 0 ) {
            i++;
            j++;
            break;
        }
        if (( strcmp(tokens[i], re) != 0 ) && ( strcmp(tokens[i], back) != 0 )) {
            newTokens[j] = tokens[i];
            // increment in both arrays
            i++;
            j++;

            // if out of space, reallocate memory
            if (j >= bufsize) {
              bufsize += TOK_BUFSIZE;
              newTokens = realloc(newTokens, bufsize * sizeof(char*));
            }
        }
        else {
            // only increment in the tokens array
            i++;
        }
        
    }
    newTokens[j] = NULL;
    return newTokens;
}


// split the tokens into an array
char *getFileName( char **tokens ) {
    char *filename;
    int fileFound = 0;
    int count = getTokenCount( tokens );
    // separate counters for new and old arrays
    int i = 0;

    // set symbol for redirect
    char re[10];
    strcpy(re, ">");

    // find file
    while (i < count) {
        if ( strcmp(tokens[i], re) == 0 ) {
            filename = tokens[i + 1];
            fileFound = 1;
        }  
        i++;
    }
    if (fileFound == 1 ) {
        return filename;
    }
    else {
        filename = "";
        return filename;
    }
}



// determines if the command is a background job
int isBackground( char **tokens ) {
    int tokenCount = 0;
    int isBack = 0;
    char *lastComm;
    char back[10];

    // get number of tokens
    tokenCount = getTokenCount( tokens );

    // symbol for background
    strcpy(back, "&");

    // check redirect and background
    if ( tokenCount > 1 ) {
        lastComm = tokens[tokenCount - 1];

        // check if background job
        if ( strcmp(lastComm, back) == 0 ) {
            isBack = 1;
        } 
    }
    return isBack;
}


// determines if the command has a redirect in place
int isRedirect ( char **tokens ) {
    int tokenCount = 0;
    int isRedir = 0;
    char *twoLastComm;
    char *threeLastComm;
    char re[10];

    // get number of tokens
    tokenCount = getTokenCount( tokens );
    
    // set symbol for redirect
    strcpy(re, ">");

    // if more than two tokens and no background token
    // check second to last token
    if ( tokenCount > 2 && (isBackground( tokens ) == 0)) {
        twoLastComm = tokens[tokenCount - 2];

        // check if redirect without background
        if ( strcmp(twoLastComm, re) == 0 ) {
            isRedir = 1;
        }   
    }  // if more than 3 tokens and a background token, check third to last token
    else if ( (tokenCount > 3) && (isBackground( tokens ) == 1) ) {
        threeLastComm = tokens[tokenCount - 3];
        // check if redirect and also background
        if ( strcmp(threeLastComm, re) == 0 ) {
            isRedir = 1;
        }
    }
    return isRedir; 
}



// check if there is a false redirect
// if there is a > symbol in the last position
int isFalseRedirect ( char **tokens ) {
    int tokenCount = 0;
    int isFalseRedir = 0;
    char *lastComm;
    char re[10];
    int reCount = 0;
    int i = 0;

    // get number of tokens
    tokenCount = getTokenCount( tokens );

    // set symbol for redirect
    strcpy(re, ">");

    // count the number of redirect symbols
    while ( i < tokenCount ) {
        if ( strcmp(tokens[i], re) == 0) {
            reCount++;
        }
        i++;
    }

    // if there is more than one redirect symbol
    // or if the last token is a redirect symbol
    // then we have a false redirect
    if ( reCount > 1 ) {
        isFalseRedir = 1;
    }
    else if ( strcmp(tokens[tokenCount - 1], re) == 0) {
        isFalseRedir = 1;
    } 

    return isFalseRedir; 
}


// checks whether first command is a python file
// returns 1 if so, 0 otherwise
int isPython ( char *firstcomm ) {
    int commLen = strlen(firstcomm);
    int dotIndex = commLen - 3;
    if ( ( commLen > 3 ) && ( strcmp(&firstcomm[dotIndex], ".py") == 0 ) )  {
        return 1;
    }
    else {
      return 0;  
    }
}


// split the tokens into an array
char **buildPyArgs( char **tokens ) {
    int bufsize = TOK_BUFSIZE;
    char **pyArgs = malloc(bufsize * sizeof(char*));
    int count = getTokenCount( tokens );
    // separate counters for new and old arrays
    int i = 0;
    pyArgs[i] = "python3";

    while (i < count) {
        pyArgs[i + 1] = tokens[i];

        // if out of space, reallocate memory
        if (i >= bufsize) {
          bufsize += TOK_BUFSIZE;
          pyArgs = realloc(pyArgs, bufsize * sizeof(char*));
        }
        i++;
    }
    pyArgs[i + 1] = NULL;

    return pyArgs;
}


// determines if the command has only "wait" in it
// returns 1 if so, 0 otherwise
int isWaitComm( char **tokens ) {
    char *firstcomm = tokens[0];
    int tokenCount = getTokenCount( tokens );

    // built-in wait command
    char waitCommand[MAX_COMM_LEN];
    strcpy(waitCommand, "wait");

    // if only one command and it equals "wait" return 1, otherwise 0
    if ((strcmp(waitCommand, firstcomm) == 0) && (tokenCount == 1)) {
        return 1;
    }
    else {
        return 0;
    }
}


// determines if first command is wait but there
// is more than one argument, which is an incorrect
// command
int isFalseWait( char **tokens ) {
    char *firstcomm = tokens[0];
    int tokenCount = getTokenCount( tokens );

    // built-in wait command
    char waitCommand[MAX_COMM_LEN];
    strcpy(waitCommand, "wait");

    // if only one command and it equals "wait" return 1, otherwise 0
    if ((strcmp(waitCommand, firstcomm) == 0) && (tokenCount > 1)) {
        return 1;
    }
    else {
        return 0;
    }
}



// check for commands with only white space
int isEmpty(const char *s) {
  while (*s != '\0') {
    if (!isspace((unsigned char)*s))
      return 0;
    s++;
  }
  return 1;
}


// checks if command is mysh
// returns 1 if so, 0 otherwise
int isMyshComm( char **tokens ) {
    char *firstcomm = tokens[0];
    int tokenCount = getTokenCount( tokens );

    // built-in wait command
    char mysh[MAX_COMM_LEN];
    strcpy(mysh, "mysh");

    // if only one command and it equals "wait" return 1, otherwise 0
    if ((strcmp(mysh, firstcomm) == 0) && (tokenCount == 1)) {
        return 1;
    }
    else {
        return 0;
    }
}


// checks if first command is mysh, but there is more than one command
// returns 1 if this is the case, 0 otherwise
int isFalseMysh( char **tokens ) {
    char *firstcomm = tokens[0];
    int tokenCount = getTokenCount( tokens );

    // built-in wait command
    char mysh[MAX_COMM_LEN];
    strcpy(mysh, "mysh");

    // if only one command and it equals "wait" return 1, otherwise 0
    if ((strcmp(mysh, firstcomm) == 0) && (tokenCount > 1)) {
        return 1;
    }
    else {
        return 0;
    }
}


// write the user name to standard output
void writeUserName() {
    struct passwd *p = getpwuid(getuid());
    write (1, p->pw_name, strlen(p->pw_name));
    write (1, "\n", strlen("\n"));
}


// runs interactive mode
void runInteractive() {
    char commline[MAX_COMM_LEN];
    char **tokens;
    char **modTokens;
    char **pyArgs;
    char *firstComm;
    char *dirfile;
    char *refile;
    int tokenCount = 0;
    int exitStatus = 0;
    int redirect = 0;
    int background = 0;
    int isPy = 0;
    int isWait = 0;
    int falseWait = 0;
    int isMysh = 0;
    int falseMysh = 0;

    // linked list for background process data
    struct node {
        pid_t data;
        struct node *next;
    };
    struct node *head = NULL;


    // loop through commands until exit
    while(1) {

        // set prompt text
        write (1, "mysh> ", strlen("mysh> ")); 

        // get command from user
        fgets(commline, MAX_COMM_LEN, stdin);

        // through error if command is too long
        // continue to next iteration
        if ( strlen(commline) > 513 ) {
            errorMessage();
            continue;
        }

        // if empty string, continue to next iteration
        if (( strcmp(commline, "\n") == 0 ) || ( isEmpty( commline ))) {
            continue;
        }

        // parse command into an array of tokens
        tokens = splitTokens( commline ); // all tokens
        modTokens = cleanTokens( tokens ); // tokens minus >, filename, and &

        // get number of tokens
        tokenCount = getTokenCount( tokens );

        // check for redirect
        redirect = isRedirect( tokens );

        // check for background
        background = isBackground( tokens );

        // get first command to check if built-in
        firstComm = tokens[0];

        // check is python
        isPy = isPython( firstComm );

        // check if we have a false redirect
        if ( isFalseRedirect( tokens ) ) {
            errorMessage();
            continue;
        }

        // check if wait called
        // and whether it has too many arguments
        isWait = isWaitComm( tokens );
        falseWait = isFalseWait( tokens );

        // check if false wait
        // continue to next iteration if so
        if ( falseWait == 1 ) {
            errorMessage();
            continue;
        }

        // check if mysh is called
        // and whether it has too many arguments
        isMysh = isMyshComm( tokens );
        falseMysh = isFalseMysh( tokens );

        // check if false wait
        // continue to next iteration if so
        if ( falseMysh == 1 ) {
            errorMessage();
            continue;
        }

        // if mysh, write the current username
        if ( isMysh == 1 ) {
            writeUserName();
        }
        else if ( isWait == 1 ) { // if wait is called

            // call waitpid on each process with children in the background
            if ( head != NULL) {
                struct node *ptr = head;
                while(ptr != NULL) {
                  waitpid(ptr->data, NULL, 0);
                  ptr = ptr->next;
                } 

                // free linked list
                struct node *temp = head;
                while(temp->next) {
                    temp = head->next;
                    free(head);
                    head = temp;
                }
                head = NULL;
            }
        }
        else if ( isBuiltIn( firstComm ) == 1 ) { // built-in commands
            // if there are more than two tokens for built-in
            // functions, print error and exit gracefully
            if ( tokenCount > 2 ) {
                errorMessage();
                return;
            }

            // get directory name
            if ( tokens[1] != NULL ) {
                dirfile = tokens[1];
            }
            else {
                dirfile = "";
            }

            // run built-in command, get exit status
            exitStatus = runBuiltIn( firstComm, dirfile );
            // end interactive mode if exit was entered as command
            if ( exitStatus == 1 ) {
                return;
            }
        }
        else { // non-built-in command
            int rc = fork();
            if ( rc < 0 ) {
                errorMessage();
            }
            else if ( rc == 0 ) {

                // redirection if necessary
                if ( redirect == 1 ) {
                    refile = getFileName( tokens );
                    close(STDOUT_FILENO);
                    open(refile, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                }

                // track failure of command
                int ex = 0;

                if ( isPy == 1 ) { // run python file
                    pyArgs = buildPyArgs( tokens );
                    ex = execvp(pyArgs[0], pyArgs);
                }
                else { // run other non-built in commands
                    ex = execvp( modTokens[0], modTokens );
                }

                // if not a command, print error message
                if ( ex == -1) {
                    errorMessage();
                }

            }
            else {
                // if a background job, don't call wait, add pid to list
                if ( background == 1 ) {
                    pid_t pid = getpid();
                    struct node *link = (struct node*) malloc(sizeof(struct node));
                    link->data = pid;
                    link->next = head; //point it to old first node
                    head = link; //point first to new first node
                }
                else { // if it is not a background job, call wait
                    int wc = wait(NULL);
                }
            }
        }
    }
}


// function to run batch mode
// takes in filename to read from as argument
void runBatch( char batchfile[MAX_COMM_LEN] ) {
    char commline[MAX_COMM_LEN];
    FILE *fp;
    char **tokens;
    char **modTokens;
    char **pyArgs;
    char *firstComm;
    char *dirfile;
    char *refile;
    int tokenCount = 0;
    int exitStatus = 0;
    int redirect = 0;
    int background = 0;
    int isPy = 0;
    int isWait = 0;
    int falseWait = 0;
    int isMysh = 0;
    int falseMysh = 0;


    // linked list for background process data
    struct node {
        pid_t data;
        struct node *next;
    };
    struct node *head = NULL;

    // open file
    fp = fopen(batchfile,"r");

    if (fp == NULL) {
        errorMessage();
        return;
    }

    // loop through commands until exit
    while( fgets(commline, 10000, fp) ) {

        // through error if command is too long
        // continue to next iteration
        if ( strlen(commline) > 513 ) {
            errorMessage();
            continue;
        }

        // if empty string, continue to next iteration
        if (( strcmp(commline, "\n") == 0 ) || ( isEmpty( commline ))) {
            continue;
        }

        // write the command back out
        write (1, commline, strlen(commline));

        // parse command into an array of tokens
        tokens = splitTokens( commline ); // all tokens
        modTokens = cleanTokens( tokens ); // tokens minus >, filename, and &

        // get number of tokens
        tokenCount = getTokenCount( tokens );

        // check for redirect
        redirect = isRedirect( tokens );

        // check for background
        background = isBackground( tokens );

        // get first command to check if built-in
        firstComm = tokens[0];

        // check is python
        isPy = isPython( firstComm );

        // check if we have a false redirect
        if ( isFalseRedirect( tokens ) == 1 ) {
            errorMessage();
            continue;
        }

        // check if wait called
        // and whether it has too many arguments
        isWait = isWaitComm( tokens );
        falseWait = isFalseWait( tokens );

        // check if false wait
        // continue to next iteration if so
        if ( falseWait == 1 ) {
            errorMessage();
            continue;
        }

        // check if mysh is called
        // and whether it has too many arguments
        isMysh = isMyshComm( tokens );
        falseMysh = isFalseMysh( tokens );

        // check if false wait
        // continue to next iteration if so
        if ( falseMysh == 1 ) {
            errorMessage();
            continue;
        }

        // if mysh, write the current username
        if ( isMysh == 1 ) {
            writeUserName();
        }
        else if ( isWait == 1 ) { // if wait is called

            // call waitpid on each process with children in the background
            if ( head != NULL) {
                struct node *ptr = head;
                while(ptr != NULL) {
                    waitpid(ptr->data, NULL, 0);
                    ptr = ptr->next;
                } 

                // free linked list
                struct node *temp = head;
                while(temp->next) {
                    temp = head->next;
                    free(head);
                    head = temp;
                }
                head = NULL;
            }
        }
        else if ( isBuiltIn( firstComm ) == 1 ) { // built-in commands
            // if there are more than two tokens for built-in
            // functions, print error and exit gracefully
            if ( tokenCount > 2 ) {
                errorMessage();
                return;
            }

            // get directory name
            if ( tokens[1] != NULL ) {
                dirfile = tokens[1];
            }
            else {
                dirfile = "";
            }

            // run built-in command and determine
            exitStatus = runBuiltIn( firstComm, dirfile );
            // end interactive mode if exit was entered as command
            if ( exitStatus == 1 ) {
                return;
            }
        }
        else { // non-built-in command
            int rc = fork();
            if ( rc < 0 ) {
                errorMessage();
            }
            else if ( rc == 0 ) {

                // redirection if necessary
                if ( redirect == 1 ) {
                    refile = getFileName( tokens );
                    close(STDOUT_FILENO);
                    open(refile, O_CREAT|O_WRONLY|O_TRUNC, S_IRWXU);
                }

                // track failure of command
                int ex = 0;

                if ( isPy == 1 ) { // run python file
                    pyArgs = buildPyArgs( tokens );
                    ex = execvp(pyArgs[0], pyArgs);
                }
                else { // run other non-built in commands
                    ex = execvp( modTokens[0], modTokens );
                }

                // if not a command, print error message
                if ( ex == -1) {
                    errorMessage();
                }

            }
            else {
                // if a background job, don't call wait, add pid to list
                if ( background == 1 ) {
                    pid_t pid = getpid();
                    struct node *link = (struct node*) malloc(sizeof(struct node));
                    link->data = pid;
                    link->next = head; //point it to old first node
                    head = link; //point first to new first node
                }
                else { // if it is not a background job, call wait
                    int wc = wait(NULL);
                }
            }
        }
    }
}




int main(int argc, char *argv[]) {

    if ( argc == 2 ) { // batch mode
        runBatch( argv[1] );
    }
    else if ( argc > 2 ) { // too many arguments
        errorMessage();
    }
    else { // interactive mode
        runInteractive();
    }

    return 0;
}

