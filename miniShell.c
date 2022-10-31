/**
 * Author - Samir Khoury
 * Mini shell, with pipe, ampersand and nohup support.
 * This a mini shell for linux, that takes an input from the user and executes it, with support for up to 2 pipes, and nohup and ampersand.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <wait.h>
#include <fcntl.h>

#define MAX_LEN 512

#define TRUE 1
#define FALSE 0

#define WORD 0
#define LETTER 1

int *wordCounter(char *str);
int writeHistory(char *str);
int printHistory();
char **createCommand(char *str, int commandCount);
void prompt();
int containsSpace(char *str);
int getLine(char *str);
char *getContent(int line);
int isHistory(char *str);
int countPipe(char *str);
char **splitPipe(char *str,int pipeCount);
void clearQuotation(char *str);
int isAmpersand(char *str);
int isNoHup(char *str);
void createNoHupFile();

int main() {
    char input[MAX_LEN];

    int numCommand=0;
    int totalWords=0;
    while (1) {
        //reset the input.
        memset(input, '\0', MAX_LEN);
        //print the prompt
        prompt();
        //user input.
        fgets(input, MAX_LEN, stdin);
        //removing the last '\n' character.
        if ((strlen(input) > 0) && (input[strlen(input) - 1] == '\n')) {
            input[strlen(input) - 1] = '\0';
        }
        //remove quotations
        clearQuotation(input);
        //check if input has pipes
        int pipeCount=countPipe(input);
        //check if the process contains ampersand.
        int amp= isAmpersand(input);
        //check if the process contains nohup.
        int hup = isNoHup(input);
        //check if user used a history command.
        if (isHistory(input) == TRUE) {
            int space = containsSpace(input);
            //check if the history command contains a space.
            if (space == TRUE) {
                printf("Illegal command entered.\n");
                memset(input, '\0', MAX_LEN);
            } else {
                if(pipeCount==0){
                    int line = getLine(input);
                    char *historyCommand = getContent(line);
                    memset(input, '\0', MAX_LEN);
                    //check if entered line is in history file.
                    if (historyCommand == NULL) {
                        printf("NOT IN HISTORY.\n");
                    } else {
                        strcpy(input, historyCommand);
                    }
                }
                else if(pipeCount==1){
                    char **pipesArr=splitPipe(input,pipeCount);
                    char *historyCommand=(char*)malloc(sizeof (char)*MAX_LEN);
                    int line;
                    line= getLine(pipesArr[0]);
                    char *content=getContent(line);
                    strcpy(historyCommand, content);
                    strcat(historyCommand,"|");
                    line=-1;
                    free(content);
                    line= getLine(pipesArr[1]);
                    content=getContent(line);
                    strcat(historyCommand, content);

                    memset(input, '\0', MAX_LEN);
                    strcpy(input, historyCommand);

                    free(historyCommand);
                    // free pipe split.
                    int i;
                    for (i=0;i<pipeCount+1;i++){
                        free(pipesArr[i]);
                    }
                    free(pipesArr);
                    free(content);
                }
                else if(pipeCount==2){
                    char **pipesArr=splitPipe(input,pipeCount);
                    char *historyCommand=(char*)malloc(sizeof (char)*MAX_LEN);
                    int line;
                    line= getLine(pipesArr[0]);
                    char *content=getContent(line);
                    strcpy(historyCommand, content);
                    strcat(historyCommand,"|");
                    line=-1;
                    free(content);
                    line= getLine(pipesArr[1]);
                    content=getContent(line);
                    strcat(historyCommand, content);
                    strcat(historyCommand,"|");
                    line=-1;
                    free(content);
                    line= getLine(pipesArr[2]);
                    content=getContent(line);
                    strcat(historyCommand, content);

                    memset(input, '\0', MAX_LEN);
                    strcpy(input, historyCommand);

                    free(historyCommand);
                    // free pipe split.
                    int i;
                    for (i=0;i<pipeCount+1;i++){
                        free(pipesArr[i]);
                    }
                    free(pipesArr);
                    free(content);
                }
            }
        }

        // if 0 pipes
        if(pipeCount==0){
            int *count = wordCounter(input);

            //check if user entered the command "exit" despite spaces.
            if (strcmp("done", input) == 0) {
                numCommand++;
                free(count);
                printf("Num of commands: %d\n", numCommand);
                printf("Total number of words in all commands: %d !\n", totalWords);
                exit(EXIT_SUCCESS);
            }
                //check if user entered the command "history" despite spaces.
            else if (strcmp("history", input) == 0) {
                numCommand++;
                totalWords = totalWords + count[WORD];
                writeHistory(input);
                memset(input, '\0', MAX_LEN);
                free(count);
                printHistory();
            }
                //check if user entered the command "cd".
            else if (strcmp("cd", input) == 0) {
                numCommand++;
                totalWords = totalWords + count[WORD];
                memset(input, '\0', MAX_LEN);
                free(count);
                printf("command not supported (Yet)\n");
            }
                //if no command was entered.
            else if (count[WORD] == 0 && count[LETTER] == 0) {
                free(count);
                continue;
            }
                // space at the beginning and/or end of command
                //// || input[strlen(input) - 1] == ' '
            else if (input[0] == ' ') {
                free(count);
                printf("Illegal command entered.\n");
            }
                // normal command to be handled by execvp without any special handling.
            else {
                writeHistory(input);
                char **command = createCommand(input, count[WORD]);
                pid_t pid;
                pid = fork();

                //if fork failed.
                if (pid < 0) {
                    perror("Fork Failed!\n");
                }
                //child proccess
                if (pid == 0) {
                    if (hup == TRUE) {
                        createNoHupFile();
                        signal(SIGHUP, SIG_IGN);
                        //remove the nohup from command.
                    }

                    int check = execvp(command[0], command);
                    if (check == -1) {
                        perror("Command Not Found");
                        exit(EXIT_FAILURE);
                    }
                }
                    //parent process.
                else {
                    if(amp==FALSE){
                        wait(NULL);
                    }
                    numCommand++;
                    totalWords = totalWords + count[WORD];
                    int i;
                    for (i = 0; i < count[WORD]; i++) {
                        free(command[i]);
                    }
                    free(command);

                    memset(input, '\0', MAX_LEN);
                    free(count);
                }
            }
        }
        // if 1 pipe
        else if(pipeCount==1){
            writeHistory(input);

            char **pipesArr=splitPipe(input,pipeCount);

            int *leftCount=wordCounter(pipesArr[0]);
            char **left= createCommand(pipesArr[0],leftCount[WORD]);

            int *rightCount=wordCounter(pipesArr[1]);
            char **right= createCommand(pipesArr[1], rightCount[WORD]);

            memset(input,'\0',MAX_LEN);

            pid_t pid;
            int fd[2];
            pipe(fd);
            pid=fork();
            // handle first process.
            if(pid==0){
                close(fd[0]);
                dup2(fd[1],STDOUT_FILENO);
                close(fd[1]);
                execvp(left[0],left);
            }
            else {
                pid = fork();
                // handle second process.

                if (pid == 0) {
                    close(fd[1]);
                    dup2(fd[0], STDIN_FILENO);
                    close(fd[0]);
                    execvp(right[0], right);
                }
                else {
                    close(fd[0]);
                    close(fd[1]);
                    // free pipe split.
                    int i;
                    for (i=0;i<pipeCount+1;i++){
                        free(pipesArr[i]);
                    }
                    free(pipesArr);

                    //left free
                    for (i = 0; i < leftCount[WORD]; i++) {
                        free(left[i]);
                    }
                    free(left);

                    //right free
                    for (i = 0; i < rightCount[WORD]; i++) {
                        free(right[i]);
                    }
                    free(right);

                    free(leftCount);
                    free(rightCount);
                    if(amp==FALSE) {
                        wait(NULL);
                        wait(NULL);
                    }
                }
            }
        }
        // if 2 pipes
        else if(pipeCount==2){
            writeHistory(input);

            char **pipesArr=splitPipe(input,pipeCount);

            int *leftCount=wordCounter(pipesArr[0]);
            char **left= createCommand(pipesArr[0], leftCount[WORD]);

            int *middleCount=wordCounter(pipesArr[1]);
            char **middle=createCommand(pipesArr[1], middleCount[WORD]);

            int *rightCount=wordCounter(pipesArr[2]);
            char **right= createCommand(pipesArr[2], rightCount[WORD]);

            memset(input,'\0',MAX_LEN);

            pid_t pid;
            int fd1[2];
            int fd2[2];
            pipe(fd1);
            pipe(fd2);
            pid=fork();
            // handle first process.
            if(pid==0){
                dup2(fd1[1],STDOUT_FILENO);
                close(fd1[0]);
                close(fd1[1]);
                execvp(left[0],left);
            }
            else {
                pid = fork();
                // handle second process.
                if (pid == 0) {
                    close(fd1[1]);
                    dup2(fd1[0],STDIN_FILENO);
                    close(fd1[0]);
                    close(fd2[0]);
                    dup2(fd2[1],STDOUT_FILENO);
                    execvp(middle[0],middle);
                    close(fd2[1]);
                } else {
                    // handle third process.
                    pid=fork();
                    if(pid==0){
                        close(fd1[0]);
                        close(fd1[1]);
                        close(fd2[1]);
                        dup2(fd2[0], STDIN_FILENO);
                        close(fd2[0]);
                        execvp(right[0], right);
                    }
                    else {
                        close(fd1[0]);
                        close(fd1[1]);
                        close(fd2[0]);
                        close(fd2[1]);

                        // free pipe split.
                        int i;
                        for (i=0;i<pipeCount+1;i++){
                            free(pipesArr[i]);
                        }
                        free(pipesArr);
                        //free left
                        for (i = 0; i < leftCount[WORD]; i++) {
                            free(left[i]);
                        }
                        free(left);

                        //free middle
                        for (i = 0; i < middleCount[WORD]; i++) {
                            free(middle[i]);
                        }
                        free(middle);

                        //free right
                        for (i = 0; i < rightCount[WORD]; i++) {
                            free(right[i]);
                        }
                        free(right);

                        free(leftCount);
                        free(middleCount);
                        free(rightCount);

                        if(amp==FALSE) {
                            wait(NULL);
                            wait(NULL);
                            wait(NULL);
                        }
                    }
                }
            }
        }
    }
}

/**
 * Counts how many words in a string, and the sum of letters in every WORD despite spaces.
 * @param str
 * @return int array were count[0] is the WORD count, and count[1] is the sum of all letters in the WORD
 */
int *wordCounter(char *str){
    char countString[MAX_LEN];
    //check if malloc failed.
    if(countString==NULL){
        perror("Error: can not allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    strcpy(countString,str);
    int *count=(int*) malloc(sizeof (int)*2);
    //check if malloc failed.
    if(count==NULL){
        perror("Error: can not allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    count[WORD]=0;
    count[LETTER]=0;
    char * token = strtok(countString, " ");//breaks a string very space character.
    while( token != NULL ) {
        count[LETTER]= (int)count[LETTER] + ((int)strlen(token));
        token = strtok(NULL, " ");
        count[WORD]++;
    }
    return count;
}

/**
 * Writes a strong to a locally stored txt file.
 * @param str
 * @return 1 if successful, 0 otherwise.
 */
int writeHistory(char *str){
    FILE *fp= fopen("file.txt","a");

    if (fp==NULL){
        printf("Error: can not open file!\n");
        return FALSE;
    }
    fprintf(fp,"%s\n", str);
    fclose(fp);
    return TRUE;
}

/**
 * Prints the strings written in the locally stored txt file.
 * @return 1 if successful, 0 otherwise.
 */
int printHistory(){
    char buff[MAX_LEN];
    int i=1;

    FILE *fp= fopen("file.txt","r");
    if (fp==NULL){
        return FALSE;
    }
    while (fgets(buff, sizeof (buff), fp) != NULL){
        printf("%d: %s",i,buff);
        i++;
    }
    fclose(fp);
    return TRUE;
}

/**
 * Takes a string and parsed in a 2D array where ever word is a separate string terminated with Null at the end.
 * @param str
 * @param commandCount
 * @return a 2D array.
 */
char **createCommand(char *str, int commandCount){
    char **command=(char**) malloc(sizeof(char*)*(commandCount+1));
    //check if malloc failed.
    if(command==NULL){
        perror("Error: can not allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    char * token = strtok(str, " ");//breaks a string very space character.
    int i=0;
    while( token != NULL ) {
        command[i]=(char*)malloc(sizeof(char)* strlen(token)+1);
        //check if malloc failed.
        if(command[i]==NULL){
            perror("Error: can not allocate memory!\n");
            exit(EXIT_FAILURE);
        }
        strcpy(command[i],token);
        command[i][strlen(token)]='\0';
        i++;
        token = strtok(NULL, " ");
    }
    command[commandCount]=NULL;
    return command;
}

/**
 * Prints the current directory.
 */
void prompt(){
    char cwd[MAX_LEN];
    getcwd(cwd,MAX_LEN);
    printf("%s",cwd);
    printf(">");
}

/**
 * Checks if a history command contains spaces.
 * @param str
 * @return 1 if found, 0 otherwise.
 */
int containsSpace(char *str){
    int found=FALSE;
    char *space=strstr(str," ");
    if(space!=NULL){//checks the first instance of "exit" int the user input.
        found=TRUE;
    }
    return found;
}

/**
 * Converts a history command number to an int.
 * @param str
 * @return line number.
 */
int getLine(char *str){
    char *lineNumber=(char*) malloc(sizeof(char)* strlen(str));
    int i;
    for(i=0;i< strlen(str)-1;i++){
        lineNumber[i]=str[i+1];
    }
    lineNumber[i]='\0';
    int line = atoi(lineNumber);
    free(lineNumber);
    return line;
}

/**
 * Gets the command stored in the history file according to the received line number.
 * @param line
 * @return string with the content of that line.
 */
char *getContent(int line){
    FILE *fp= fopen("file.txt","r");
    char reader[MAX_LEN];
    char *checkLine;
    int i;
    //iterate over lines
    for(i=0;i<line-1;i++){
        memset(reader,'\0',MAX_LEN);
        fgets(reader, sizeof(reader), fp);
    }

    //get requested line
    checkLine=fgets(reader, MAX_LEN, fp);
    //removing the last '\n' character.
    if ((strlen(reader) > 0) && (reader[strlen(reader) - 1] == '\n')) {
        reader[strlen(reader) - 1] = '\0';
    }
    fclose(fp);

    char *lineContent;
    // check if requested line is not in history
    if(checkLine==NULL){
        lineContent=NULL;
    }
    else {
        lineContent = (char *) malloc(sizeof(char) * (strlen(reader) + 1));
        strcpy(lineContent, reader);
        lineContent[strlen(reader)] = '\0';
    }

    return lineContent;
}

/**
 * Checks if an input a history command.
 * @param str
 * @return 1 if history command, 0 otherwise.
 */
int isHistory(char *str){
    if(str[0]=='!'){
        return TRUE;
    } else
        return FALSE;
}


/**
 * counts how many pipes in the input.
 * @param str
 * @return the number of pipes in the input.
 */
int countPipe(char *str){
    int count=0;
    // parameter backup.
    char input[MAX_LEN];
    strcpy(input,str);

    //Search for the first pipe symbol.
    char *pipe=input;
    while ((pipe=strstr(pipe,"|"))!=NULL){
        count++;
        pipe++;
    }

    return count;
}

/**
 * splites the pipe splitted input into separate commands.
 * @param str
 * @param pipeCount
 * @return the splited commands.
 */
char **splitPipe(char *str,int pipeCount){
    // parameter backup.
    char input[MAX_LEN];
    strcpy(input,str);
    char **pipeStr=(char**)malloc(sizeof (char*)*(pipeCount+1));

    //check if malloc failed.
    if(pipeStr==NULL){
        perror("Error: can not allocate memory!\n");
        exit(EXIT_FAILURE);
    }

    char * token = strtok(input, "|");//breaks a string very space character.
    int i=0;
    while( token != NULL ) {
        pipeStr[i]=(char*)malloc(sizeof(char)* strlen(token)+1);
        //check if malloc failed.
        if(pipeStr[i]==NULL){
            perror("Error: can not allocate memory!\n");
            exit(EXIT_FAILURE);
        }
        strcpy(pipeStr[i],token);
        pipeStr[i][strlen(token)]='\0';
        i++;
        token = strtok(NULL, "|");
    }

    return pipeStr;

}

/**
 * Removes the quotation from the input if available.
 * @param str
 */
void clearQuotation(char *str){
    char *read = str, *write = str;
    while (*read) {
        *write = *read++;
        write += (*write != '\"');
    }
    *write = '\0';
}

/**
 * Checks if the input contains ampersand symbol.
 * @param str
 * @return 1 if it contains ampersand, 0 otherwise.
 */
int isAmpersand(char *str){
    int found=FALSE;
    char *ampersand= strstr(str,"&");
    if(ampersand!=NULL){//checks the first instance of "exit" int the user input.
        str[strlen(str)-1] = '\0';//remove the ampersand symbol from the input.
        found=TRUE;
    }
    return found;
}

/**
 * Checks if the input contains ampersand symbol in the beginning.
 * @param str
 * @return 1 if it contains ampersand, 0 otherwise.
 */
int isNoHup(char *str){
    int found=FALSE;
    char *nohpu= strstr(str,"nohup");
    if(nohpu!=NULL){//checks the first instance of "exit" int the user input.
        if(str[0]=='n'&&str[1]=='o'&&str[2]=='h'&&str[3]=='u'&&str[4]=='p'){
            // remove the nohup from the command itself.
            memmove(str,str+6, strlen(str)-5);//-6+1
            found=TRUE;
        }
    }
    return found;
}

/**
 * creates the nohup.txt file, and redirects the output to it.
 */
void createNoHupFile(){
    int fd=open("nohup.txt",O_RDWR|O_CREAT,S_IRUSR| S_IWUSR | S_IRGRP|S_IROTH );
    int value=dup2(fd, STDOUT_FILENO);
    if (value==-1){
        perror("open failed\n");
    }
}
