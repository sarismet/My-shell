#include <stdio.h>

#include <stdlib.h>

#include <string.h>

#include <fcntl.h>

#include <sys/shm.h>

#include <sys/stat.h>

#include <sys/mman.h>

#include <pwd.h>

#include <unistd.h>

#include <sys/types.h>

#include<sys/wait.h>

#include <limits.h>

#include <signal.h>


//this funtion is designed to remove some certain chars such as " for instance if a user give grep "include" we need to remove  quotation marks.
void removeChar(char * chararraygiven, char charwedontwant) {

        //we create two separate char array then assign str to src and dst and increase the dst by one
        //if it is not the char we want if it is the char we dont want then we dont increase the adress of dst
        // after that step the char we want to delete dissapear.
        char * temp1;
        char * temp2;
        temp2 = temp1 = chararraygiven;
        for (;* temp2 != '\0'; temp2++) {
                * temp1 = * temp2;

                if ( * temp1 != charwedontwant) temp1++;
        }
        * temp1 = '\0';
}


//we execute basic command without pipe or '>' sign. These commands look like ls -al, rm something
void runbasiccommand(char ** basiccommand) {
        //we first fork and return the process id to pid variable
        pid_t pid = fork();
        // if pid is -1 then the process did not success
        if (pid == -1) {
                printf("\n there is a problem here");
                return;
        } else if (pid == 0) { //this is child. We just run the basic command here. 
                //The given command is array type so we just take first element and the adress of the array
                //and give them to execvp funtion.
                if (execvp(basiccommand[0], basiccommand) < 0) {
                        printf("\n there is a problem here..%s\n", basiccommand[0]); // if there is a problem that occured this would be printed. There would be probably no problem      
                        exit(0);
                }

        } else {
                wait(NULL); // we wait for child to terminate and return
                /*wait(&stat);
                if(stat!=0){
                printf("there is a problem that occurred in child %d exit code is : %d",pid,stat);
                }*/
                return;
        }
}

// this funtion was designed to take the name of systemusername but later on I decided to use and other way(fork and child you can it in main funtion)
char * getUserName() {
        uid_t uid = geteuid(); //first get id

        struct passwd * pw = getpwuid(uid); // then create the whole structure from this id
        if (pw) {
                return pw->pw_name; //and return the name from it
        }

        return "";
}
//this funtion was designed to list first 15 command what has been written by user. We need the number of current to see what code was executed last and current would be 15 less
int history(char * hist[], int current, int head) {
        if (head == -1) { //we set the head at first -1 if the user have not given 15 command yet. In this situation we set the head 0. Then from 0 to current we print out history from hist[]list
                head = 0;
        }
        int i = head;
        int hist_num = 1;

        for (i = head; i < current && hist[i]; i++) {
                printf("%4d  %s\n", hist_num, hist[i]);
                hist_num++;

        }
        return 0;
}
// this is to get the signal and act according to number of it. This print the number of the signal to inform user as well
void signal_callback_handler(int signum) {
        printf("Caught signal %d\n", signum);
        exit(signum);
}
// this funtion designed to check if there is the direction symbol '>' in input 
//if there is then the funtion divide the string into 2 part and store them in dividedarray array
int parseCAT(char * thewholestring, char ** dividedarray) {
        int i;
        for (i = 0; i < 2; i++) {
                dividedarray[i] = strsep( & thewholestring, ">");
                if (dividedarray[i] == NULL)//if we reach the end then stop
                        break;
        }

        if (dividedarray[1] == NULL)
                return 0; // returns zero if no '>' is found.
        else {// we found '>' then we need to return 2 or bigger then 0
                //    printf("CAT ICERDEYIM RETURNLUYOM   %s  %s\n",strpiped[0],strpiped[1] );
                return 2;
        }
}
//This funtion designed to check if there is the direction symbol '|' in input just like function above
//if there is then the funtion divide the string into 2 part and store them in dividedarray array
int parsePipe(char * thewholestring, char ** dividedarray) {
   int i;
        for (i = 0; i < 2; i++) {
                dividedarray[i] = strsep( & thewholestring, "|");
                if (dividedarray[i] == NULL)//if we reach the end then stop
                        break;
        }

        if (dividedarray[1] == NULL)
                return 0; // returns zero if no '|' is found.
        else {// we found '>' then we need to return 1 or bigger then 0
                //    printf("CAT ICERDEYIM RETURNLUYOM   %s  %s\n",strpiped[0],strpiped[1] );
                return 1;
        }
}
// This funtion was designed to parse the given input and take the basic commands and store them in parsedarray 
//like if the input is "ls -al" then array would be parsedarray[0]=ls and parsedarray[1]=-al
void parseSpace(char * stringinput, char ** parsedarray) {
        int i;

        for (i = 0; i < 100; i++) {
                parsedarray[i] = strsep( & stringinput, " ");// we first reach out to the spaces and take the input and store one by one them 
                if (parsedarray[i] != NULL) {//if it is not the end then create a temporrary string
                        char * string = malloc(strlen(parsedarray[i]) + 1);
                        strcpy(string, parsedarray[i]);//copy it to another string
                        removeChar(string, '\"');//check the string if it contains and \" and if it is then remove it.This function was used essipicially for grep command
                        strcpy(parsedarray[i], string);//then again store it to original array space
                }

                if (parsedarray[i] == NULL)//if we reach out to end then stop
                        break;
                if (strlen(parsedarray[i]) == 0)//the string we found has zero size then it is notthing but null so no need to increase i
                        i--;
        }
}

int inputhandler(char * stringinput, char ** firstpartarray, char ** secondpartarray) {

        char * parts[2] = {// we first create this empty array to store temporarily the first and second part this given string
                "",
                ""
        };
        int is_there_pipe = 0; //it is to check if there exists a '|' in given strings
        int is_there_cat = 0; //it is to check if there exists a '>' in given strings

        is_there_pipe = parsePipe(stringinput, parts); //this function first check if there exists a '|' in given strings and 
        //it it is then return 1 and stores the commands in parts arrays

        if (is_there_pipe) { // so this if will be executed
                      //printf("PIPE VAR   %s  %s\n",strpiped[0],strpiped[1] );
                     
                      int index = 0;

                      char find = '\"';//this is to fint where the beginning of \" 

                      const char * ptr = strchr(parts[1], find);//this is find the \" then come to the index of it
                      if (ptr) {
                              index = (ptr - parts[1]); // we find the index 

                      }
                      char new1[200];//this is a temporary array to store 
                      if (index != 0) {
                          secondpartarray[0] = "grep";//we define first part as grep since it has to be in this if statement

                          parseSpace(parts[0], firstpartarray); //this function parses the parts[0] and store one by one into  firstpartarray
                          
                              char* token;//we define the name to store every part
                                char* rest = parts[1];
                          int ls=0;
                          while ((token = strtok_r(rest, "\"", &rest))){//this is parse the string according to \"
                                    
                              if(ls==1){//we neet to keep going until second \" sign
                                  secondpartarray[1] = (char * ) malloc(200 * sizeof(char));//we define the array as dynamic since it is NULL before definition

                                  for (int i = 0; i < strlen(token); i++) {
                                          secondpartarray[1][i] = token[i];//when we have the token between "" we copy it to secondpartarray[1]
                                  }
                                  return 2;//we are done
                              }
                              ls++;
                          }
                              
                         
                                }
                                //if there is no \" then we need to do as usual
                      parseSpace(parts[0], firstpartarray); //this function parses the parts[0] and store one by one into  firstpartarray
                   parseSpace(parts[1], secondpartarray); //this function parses the parts[1] and store one by one into  secondpartarray
                      return 2;

              }
        parts[0] = "";// if there is no '|' pipe symbol then refresh the parts array to continue with '>' symbol if there is then we make operation below and return 3
        parts[1] = "";// we return 3 just because of detection 
        is_there_cat = parseCAT(stringinput, parts);
        if (is_there_cat) {
                //printf("CAT VAR   %s  %s\n",parts[0],parts[1] );
                parseSpace(parts[0], firstpartarray);//this function parses the parts[0] and store one by one into  firstpartarray
                parseSpace(parts[1], secondpartarray);//this function parses the parts[1] and store one by one into  secondpartarray
                return 3;
        }

        //if there is neiher '|' or '>' then it means that it is a basic command so
        //we refresh the parts array and completely parse all stringinput and store them one by one in firstpartarray then return 1 to detect it.
        parts[0] = "";
        parts[1] = "";

        parseSpace(stringinput, firstpartarray);

        return 1;
}
//this funtion was designed to merge two string if I want to execate basic command then I need to have space between shell command lile ls + space +-al
// I can use it 3 time. two of them are for space one of them is for -al. I found this command on a site that I don't remember
char * concat(const char * s1,
        const char * s2) {
        const size_t len1 = strlen(s1);
        const size_t len2 = strlen(s2);
        char * result = malloc(len1 + len2 + 1); // +1 for the null-terminator
        // in real code you would check for errors in malloc here
        memcpy(result, s1, len1);
        memcpy(result + len1, s2, len2 + 1); // +1 to copy the null-terminator
        return result;
}

// this is most important funtion that I had created. This of that we have a pipe in a given inputstring like ls -al|grep .c then we need to have to child to run first and second part
// first we use fork() funtion then make the first child to execute the first command then to redirect the putput of it to pipeofismet[1] so that ismetofpipe[0] can read
// then we use fork() again but in this time in pipeofismet[0] has the output of the command the firstchild executed so the second child redirect the output from pipe to stdin so that
// the second command can read it like grep because grep looks at stdin for input
void pipefuntion(char ** firstcommand, char ** secondcommand) {

        int pipeofismet[2];// we create the pipe

        //int status; no need
        //int stat; no need

        int pid;

        for (int i = 0; i < 2; i++) {// we need two child so we can have it by loop

                if (i != 1) {
                        if (pipe(pipeofismet) < 0) {
                                perror("pipe creating was not successfull\n");
                                return;
                        }
                }
                pid = fork();
                if (pid == 0) { //this is child 1 it would run the first command and direct the output of it(from stdout) to pipeofismet(1)
                        if (i != 1) { //i!=1 i=0
                                dup2(pipeofismet[1], 1); // to direct output from stdout to pipe1. Pipe 1 would write pipe0 would read at parent so when we fork again the second child can read by pipe0
                                close(pipeofismet[0]); //no need to read
                                close(pipeofismet[1]); //after redirection no need to read
                        }

                        if (i != 0) { //i=1
                                dup2(pipeofismet[0], 0); //after second for we read from pipe0 because the output of firstcommand is there
                                close(pipeofismet[1]); //no need to write
                                close(pipeofismet[0]); //no need to read
                        }
                        if (i == 0) {// first time means i=0 then first command wpuld be executed here
                                if (execvp(firstcommand[0], firstcommand) < 0) {
                                        perror("input cannot be executable ");// if there is a problem that occured this would be printed out
                                        exit(1);
                                }
                        } else if (i == 1) {// second time means i=1 then second command would be executed here
                                execvp(secondcommand[0], secondcommand);//I dont write any error mesage probably

                }
                }
                //this is for parent because for every child there is a exec method so they cannot reach out here but parent
                if (i != 0) { //second process
                        close(pipeofismet[0]); // we need to close pipe 0 because it is the end no need to read
                        close(pipeofismet[1]); // we need to close pipe 0 because it is the end no need to read
                }

                wait(NULL);// we wait first child to terminate then we would go to second loop iteration.
              
                

        }

}

// this funtion is used for commands with direction '>' sign. First read and write all what we have read to new file
void redirectinput(char ** firstcommand, char ** filename) {
          
        pid_t pid;
        char read_msg[500000] = ""; //we first allocate a char array to store the output of first child
        int i = 0;
        int p[2]; // this is our pipe
        if (pipe(p) == -1) {//if there is a problem
                fprintf(stderr, "First Pipe failed");
                return;
        }

        while (i < 2) {// we need to child so we iterate twice

                pid = fork();

                if (pid > 0) // this is parent
                {
                        close(p[1]);// we dont write anywhere so close what we dont need
                        if (i == 0) {// if it is first parent
                                read(p[0], read_msg, 500000);// then read it from p[0] and store it read_msg variable with type char array
                                close(p[0]);//then we are done with p[0] so we can close it
                        }
                        wait(NULL);// we wait first child to terminate
                } else if (pid == 0) //this is child
                {
                        if (i == 0) {// if it is first child then we have a command to run
                                dup2(p[1], 1);// we redirect the output of the command execution to p[1] so that
                            //the first parent(actually there is one parent there are nearly the same) can read by p[0]
                                close(p[1]);//then we are done with p[1] so we can close it
                                execvp(firstcommand[0], firstcommand);// after all run the first command

                        } else if (i == 1) {// if it is second child then
                                FILE * fp = fopen(filename[0], "w");// we first open a file named given filename array actually the array only contain one element which is filename

                                for (i = 0; read_msg[i] != '\0'; i++)// then we write to file one char one
                                        fputc(read_msg[i], fp);
                               
                                fclose(fp);// after we finish the loop close it
                                exit(0);// and kill the child so that it cannot reach to insruction in main funtion

                        }
                }
                i++;

        }
}

int main() {

        signal(SIGINT, signal_callback_handler);//if there is any signal such as SIGKILL then this funtion would catch it oparete the process according to int of signal returnned
        printf("WELCOME TO MY SHELL \n");
        char * hist[1000];// this is to keep track of command the user give

        int head = -1;//this is head of list we need to print on out terminal when user write footprint. it is initialized with -1 so we can see if the user write more than or equal to 15 command
        int current = 0;// this is to keep the size of command the user give

        //char * username = getUserName();
        char username[100];// this is to keep the user name

        pid_t pid;

        int p[2];// we need a pipe to read the child output and write it to username char array
        if (pipe(p) == -1) {
                fprintf(stderr, "First Pipe failed");

        }
        pid = fork();

        if (pid > 0) //parent
        {
                close(p[1]);//no need

                read(p[0], username, 100);// read from p[1] and write it to username

                //printf("this time %s",username);
                close(p[0]);// no need anymore
                wait(NULL);
                /* wait(&stat);
                   if(stat!=0){
                   printf("there is a problem that occurred in child %d exit code is : %d",pid);
                   }*/
        } else if (pid == 0) //Child
        {

                dup2(p[1], 1);// redirection from stdout to p[1] so that parent can read and write
                close(p[1]);//no need anymore
                execlp("/usr/bin/whoami", "whoami", NULL);//run the whoami command to see who the username is

        }

        if (username[strlen(username) - 1] == '\n')// the username has \n so we need to remove this no to go to newline
                username[strlen(username) - 1] = '\0';

        for (int i = 0; i < 1000; i++)
                hist[i] = NULL;// since we dont create this array dynamically we need to fill this with NULL. It is also important to run command because execvp function take NULL parameters at last.

        char input_from_user[1000];
        char * first_part_of_commands[100];
        char * second_part_of_commands[100];

        for (int i = 0; i < 100; i++) {//again because of the same reason we need to fill first_part_of_commands array NULL as a initialization
                first_part_of_commands[i] = NULL;
        }

        for (int i = 0; i < 100; i++) {
                second_part_of_commands[i] = NULL;
        }

        int what_is_process = 0;// this is to define the structure of input if it is 1 then input is basic command if it is 2 then the input include pipe, 3 means it has '>' sign

        while (1) {

                printf("%s >>> ", username);//write the username without newline

                fgets(input_from_user, 1000, stdin);//take input with size 1000
                fflush(stdin);//empty the stdin
            
                if (strcmp(input_from_user, "\n") == 0) {
                        
                        continue;// and we must not to go further
                }
            

                if (input_from_user[strlen(input_from_user) - 1] == '\n')// if there is a newline character we could have problem to execute and printout so we need to get rid of it
                        input_from_user[strlen(input_from_user) - 1] = '\0';// and replace it with the end of string character
                //printf("%s%s\n","input is : ",inputString );

                free(hist[current]);// first since the array is filled with NULL  we first make the space available

                hist[current] = strdup(input_from_user);// then duplucate the given input and store in in hist array

                what_is_process = inputhandler(input_from_user,first_part_of_commands, second_part_of_commands);// we first give the adress of arrays and given input to
                // inputhandler funtion and it return a interger. If it is 1 then the input is basic command if it is 2 then the input includes pipe, if it is 3 then it means it has '>' sign
                
                // we initialize head with -1 to see the current whether or not the user gives 15^th command. The -1 is meaningless just consider it as a boolean varible like
                if (current == 15) {// we initialize head with -1 to see the current whether or not the user gives 15^th command. The -1 is meaningless just consider it as a boolean varible like
                        head = 0;//so if the user gives then we need to start head from 0
                }
                current++;// we need to increase current here because we run the command below and if there is a problem that occurred then it might not be increased and as you can see if the user
                // give "footprint" command then the if statement uses continue insruction
            
               // if the user has given more than or equal to 15 command than the head must be 0 or bigger then in this condition we need to increase it by one if it is negative
                if (head != -1) { //then we over pass the if statement
                        head++;
                }
            
                //if the user give footprint input then we need to run history funtion and it print out first 15 command but if the user has not give 15 command then the head would be -1 and
                // the program see that the user has not give 15 command so it only print out how many command the user give which is less than 15 like up to 5^th command on out terminal
                if (strcmp(input_from_user, "footprint") == 0) {
                        history(hist, current, head);
                        continue;// and we must not to go further
                }
                //if the user give exit then we need to kill the process I could also use kill(getpid,9) but it work as well
                if (strcmp(input_from_user, "exit") == 0)
                        exit(0);
             
                // if the user write "printfile" at the beginning then we need to replace it with "cat" because the linux based system cat can invoke system call not printfile
                if (strcmp(first_part_of_commands[0], "printfile") == 0)
                        first_part_of_commands[0] = "cat";
                // if the user write "listdir" at the beginning then we need to replace it with "ls" because the linux based system ls can invoke system call not printfile
                else if (strcmp(first_part_of_commands[0], "listdir") == 0)
                        first_part_of_commands[0] = "ls";
                // if the user write "currentpath" at the beginning then we need to replace it with "pwd" because the linux based system pwd can invoke system call not printfile
                else if (strcmp(first_part_of_commands[0], "currentpath") == 0)
                        first_part_of_commands[0] = "pwd";
                //if the returnned value from inputhandler is 1 then it means the user gave a basic command
                if (what_is_process == 1) {

                        runbasiccommand(first_part_of_commands);// so we need to run basic command by using executebasıccommand() funtion.
                }
                //if the returnned value from inputhandler is 2 then it means the user gave command with pipe
                if (what_is_process == 2) {

                        pipefuntion(first_part_of_commands, second_part_of_commands);// so we need to run command with pipe by using pipefuntion() funtion.

                }
                //if the returnned value from inputhandler is 3 then it means the user gave command with this '>' redirect sign
                if (what_is_process == 3) {
                              

                        redirectinput(first_part_of_commands, second_part_of_commands);// so we need to run command with this '>' redirect sign by using redirectinput() funtion.

                }

        }

        return 0;// after all we retutn 0 but since we are using infinite while loop the program probably would not reach there
}
