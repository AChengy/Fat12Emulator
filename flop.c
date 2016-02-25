//
// Created by Alexc on 10/24/2015.
//

#include "flop.h"

int mountcondition = FALSE;
//path for holding all the usable paths for executing functions
char **path = NULL;
int numpaths =0;

int main(int argc, char **argv){
    char **shell_args;
    int shell_argc;
    int pipes;
    int redirections;
    command *commands;

    while(TRUE) {

        printf("flop:");
        char *line = NULL;
        size_t bufsize = 0; // have getline allocate a buffer for us
        getline(&line, &bufsize, stdin);
        if(strcmp(line,"\n")==0)
          continue;
        //Remove '\n' at the end of the line
        line[strcspn(line, "\n")] = '\0';

        //setup shell_args
        shell_args = (char **)malloc(sizeof(char *));
        //break line into multiple arguments
        int i;
        char *p; //temp variable for tokenizing

        for (i = 0, p = strtok(line, " "); p != NULL; ++i, p = strtok(NULL, " ")) {
            if(i>0){
                shell_args=realloc(shell_args,sizeof(char *)*(i+1));
            }

            shell_args[i] = (char *)malloc(strlen(p) + 1);
            strcpy(shell_args[i], p);

        }
        shell_argc = i;



        if(strcmp(shell_args[0],"path")==0){
            if(shell_argc==1){
                pathcommand(shell_args,shell_argc,0);
            }else if(strcmp(shell_args[1],"+")==0){
                pathcommand(shell_args,shell_argc,1);
            }else if(strcmp(shell_args[1],"-")==0){
                pathcommand(shell_args,shell_argc,2);
            }else{
                write(1,"Invalid path command type help for more.\n",41);
            }
        }else if(strcmp(shell_args[0],"cd")==0){
            //change the directory
            cd(shell_args);
        }else if(strcmp(shell_args[0],"quit")==0 || strcmp(shell_args[0],"exit")==0){
            for(i =0;i<shell_argc;i++){
                free(shell_args[i]);
            }
            free(line);
            break;
        }else if(strcmp(shell_args[0],"cwd")==0){
          //get current working directory
          char cwd[1024];
          if (getcwd(cwd, sizeof(cwd)) != NULL)
            fprintf(stdout, "Current working directory: %s\n", cwd);
          else
            perror("flop");
        }else{
          //find the number of pipes or redirections
          pipes=numofpipes(shell_args,shell_argc);
          redirections=numofredirections(shell_args,shell_argc);

          //only needs to break up arguments if there are pipes
          if(pipes>0){
            commands = breakargs(shell_args,shell_argc,pipes);
          }
          //checked for all the built in shell commands now to execute programs
          if(redirections ==0 && pipes==0){
            launch(shell_args,shell_argc);
          }else if(redirections==0 && pipes >0){
            launchpipes(commands,pipes);
          }else if(redirections>0 && pipes==0){
            launchredirections(shell_args,shell_argc);
          }else if(redirections>0 && pipes>0){
            launchpipedredirection(commands,pipes);
          }else{
            write(1,"flop: invalid input\n",21);
          }

        }//end else
		
		fflush(stdout); //safety check
        
		free(line);		//free the line
		
		//free all the lines
        for(i =0;i<shell_argc;i++){
            free(shell_args[i]);
        }

        free(shell_args);
		
    }//end while true

    return 0;
}
//Built in command to change the directory returns 1 to keep the shell executing
int cd(char **args){
    if (args[1] == NULL) {
        write(1, "flop: expected argument to \"cd\"\n",33);
    } else {
        if (chdir(args[1]) != 0) {
            perror("flop");
        }
    }
    return 1;
}

int pathcommand(char **args, int argsc,int flag){
    if(argsc==1){
        if(path == NULL){
            write(1,"flop: no path selected use path + to add to the path\n",53);
        }else{
            for(int i=0;i<numpaths;i++){
              //the semicolon is to deliminate the different paths while printing.
              printf("%s",path[i]);
              if(i!=(numpaths-1))
              printf(":");
            }
            printf("\n");
        }
    }else if(strcmp(args[1],"+")==0){
        if(path == NULL){
            path = (char **)malloc(sizeof(char *));
            path[numpaths]=malloc(sizeof(args[2])+1);
            strcpy(path[numpaths++],args[2]);
        }else{
            path = realloc(path,sizeof(char *)*(numpaths+1));
            path[numpaths]=malloc(sizeof(args[2])+1);
            strcpy(path[numpaths++],args[2]);
        }

    }else if(strcmp(args[1],"-")==0){
        if(path==NULL){
            write(1,"flop: error: no path file currently selected\n",47);
        }else{
            //find the matching path to Remove
            int matchingindex;
            for(int i =0;i<numpaths;i++){
              if(strcmp(path[i],args[2])==0)
                matchingindex =i;
            }
            //shift everything down by one erasing the path to delete
            for(int i =matchingindex;i<numpaths-1;i++){
              path[i]=path[i+1];
            }
            //change the number of paths to one less
            numpaths--;
            //realloc the path to the new smaller number of paths
          path = realloc(path,sizeof(char *)*numpaths);

        }
    }
    return 1;
}
//get number of pipes needed to figure out how to launch
int numofpipes(char **args, int argc){
  int ret =0;

  for(int i =0;i<argc;i++){
    if(strcmp(args[i],"|")==0){
      ret++;
    }
  }

  return ret;
}
//number of redirections need to figure out how to launch
int numofredirections(char **args, int argc){
  int ret =0;

  for(int i =0;i<argc;i++){
    if(strcmp(args[i],"<")==0 || strcmp(args[i],">")==0){
      ret++;
    }
  }

  return ret;
}

//breaks up the long argument into multiple small commands for piping
command* breakargs(char **args,int argc,int pipes){
  command *ret;
  int retnum =0;
  int index=0;
  char **commandargs;


  for(int i=0;i<argc;i++){
    //store the smaller arrays in a pointer of Commands which consist of
    //of char** and the number of args in each.
    if(strcmp(args[i],"|")==0){
      if(retnum==0){
        ret=(command *)malloc(sizeof(command));
        commandargs=realloc(commandargs,sizeof(char *)*(index+1));
        commandargs[index]=NULL;
        ret[retnum].argc=index;
        ret[retnum].args=commandargs;
        retnum++;
        index=0;
        continue;
      }else{
        ret=realloc(ret,sizeof(command)*(retnum+1));
        commandargs=realloc(commandargs,sizeof(char *)*(index+1));
        commandargs[index]=NULL;
        ret[retnum].argc=index;
        ret[retnum].args=commandargs;
        retnum++;
        index=0;
        continue;
      }
    }
    //break each line into smaller arrays
    if(index ==0){
      commandargs = (char **)malloc(sizeof(char *));
      commandargs[index]=(char *)malloc(strlen(args[i]) + 1);
      strcpy(commandargs[index++], args[i]);
    }else{
      commandargs=realloc(commandargs,sizeof(char *)*(index+1));
      commandargs[index]=(char *)malloc(strlen(args[i]) + 1);
      strcpy(commandargs[index++], args[i]);
    }

  }

  ret=realloc(ret,sizeof(command)*(retnum+1));
  commandargs=realloc(commandargs,sizeof(char *)*(index+1));
  commandargs[index]=NULL;
  ret[retnum].argc=index;
  ret[retnum].args=commandargs;


  return ret;

}//end breakargs

/*
* Begin launch section of shell
*/
int launch(char **args,int argc){
    pid_t pid;
    int status;

    //add in the NULL for the execv call
    args=realloc(args,sizeof(char *)*(argc+1));
    args[argc]=NULL;

    pid = fork();
    if (pid == 0) {

        // Child process
        //check the current directory first
        //have to create a temp string with the name to send through execv
        char temp[1024];
        sprintf(temp, "./%s", args[0]);
        if (execv(temp, args) == -1) {
        //check all the different paths for the program to execute
          for(int i =0;i<numpaths;i++){
          char temp[1024];
          sprintf(temp, "%s/%s",path[i], args[0]);
          //printf("%s\n",temp);
          execv(temp,args);
        }
          perror("flop");
		  exit(1);
      }


    } else if (pid < 0) {
        // Error forking
        perror("flop");
    } else {
        // Parent process
        do {
          pid_t  wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    return 1;
}//end launch

int launchredirections(char **args, int argc){
  int in, out, inbool, outbool,status;
  int index =0;
  char **command;
  pid_t pid;


//now set up the array for execv command
for(int i=0;i<argc;i++){
  if(strcmp(args[i],"<")==0 || strcmp(args[i],">")==0)
    break;

  if(i==0){
    command=(char **)malloc(sizeof(char *));
    command[index]=(char *)malloc(strlen(args[i]) + 1);
    strcpy(command[index++], args[i]);
  }else{
  command=realloc(command,sizeof(char *)*(i+1));
  command[index]=(char *)malloc(strlen(args[i]) + 1);
  strcpy(command[index++], args[i]);
}
}

//add in the NULL at the end
command=realloc(command,sizeof(char *)*(index+1));
command[index]=NULL;


  pid = fork(); //fork the process

  if(pid == 0){
  for(int i =0;i<argc;i++){

    if(strcmp(args[i],"<")==0){
      in=open(args[i+1],O_RDONLY); //open file
      dup2(in,0); //have file replace stdin
      inbool=TRUE;
  }else if(strcmp(args[i],">")==0){
      out=open(args[i+1],O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR); //open file
      dup2(out,1); // have file replace stdout
      outbool = TRUE;
  }else{
   //nothing
}
}

//close unused file descriptors
if(inbool == TRUE)
  close(in);

if(outbool == TRUE)
  close(out);

//check current directory first
char temp[1024];
sprintf(temp, "./%s", command[0]);
if(execv(temp,command)==-1){
//check all other path directories
  for(int i =0;i<numpaths;i++){
  char temp[1024];
  sprintf(temp, "%s/%s",path[i], args[0]);
  execv(temp,command);
}
  perror("flop");
  exit(1);
}
}//end child
else{
  do {
    pid_t  wpid = waitpid(pid, &status, WUNTRACED);
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));
}
return 1;
}//end launchredirections



//start launchpipes
int launchpipes(command *commands,int pipes){
  int status;
  int numberofpipes = pipes;
  int fd[numberofpipes*2]; //need two array points for each pipe
  pid_t pid;

  for(int i =0,j =0;j<numberofpipes;i+=2,j++){
    pipe(fd+i);
  }

  for(int i=0, k=0;k<=(numberofpipes);i+=2,k++){
    pid = fork();
    if(pid==0){

      if(i==0){
        //replace the stdout of this process with the write of the first pipe
        dup2(fd[1],1);
        //replace the stdin with the input read of the final pipe
      }else if(k==(numberofpipes)){
        dup2(fd[0+(2*numberofpipes-2)],0);
      }else{
        //replace standin with read end of previous pipe
        dup2(fd[i-2],0);
        //replace the stdout with the write end of the next pipe
        dup2(fd[i+1],1);
      }
		//close all unused pipes
      for(int j=0;j<numberofpipes*2;j++){
        close(fd[j]);
      }
      /*Checks the current directory for the program to run if it's not there
      it checks the path. If it's not there it reports the error and exits.*/

	//check current directory for executable first
      char temp[1024];
      sprintf(temp, "./%s", commands[k].args[0]);
      if(execv(temp, commands[k].args)<0){
		//if it fails check all the path directories
        for(int i =0;i<numpaths;i++){
        char temp2[1024];
        sprintf(temp2, "%s/%s",path[i], commands[k].args[0]);
        execv(temp2,commands[k].args);
      }
        perror("flop");
        exit(1);

      }
    }else if(pid<0){
      perror("flop");
    }else{
      continue;
    }
  }
	//close all unused pipes so parent can continue
  for(int j=0;j<numberofpipes*2;j++){
    close(fd[j]);
  }

  do {
    pid_t  wpid = waitpid(pid, &status, WUNTRACED);
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));

      return 1;
}//end launchpipes

int launchpipedredirection(command *commands, int pipes){
  int status, in, out, inbool = FALSE,outbool = FALSE;
  int index =0, index2=0;
  int numberofpipes = pipes;
  int fd[numberofpipes*2];
  char **cmd, **cmd2;
  pid_t pid;


  for(int i =0,j =0;j<numberofpipes;i+=2,j++){
    pipe(fd+i);
  }

  //check to see if there is in and out redirection
for(int i =0;i<=numberofpipes; i++){
	for(int j =0;j<commands[i].argc;j++){
		if(strcmp(commands[i].args[j],"<")==0)
			inbool=TRUE;

		if(strcmp(commands[i].args[j],">")==0)
			outbool=TRUE;
	}
}


  for(int i=0, k=0;k<=(numberofpipes);i+=2,k++){
    pid = fork();
    if(pid==0){

      if(i==0){
        
	if(inbool == TRUE){ //if inbool is TRUE that means we have to have rediection in
        for(int i2=0;i2<commands[0].argc;i2++){
          if(strcmp(commands[0].args[i2],"<")==0){
            in = open(commands[0].args[i2+1],O_RDONLY); //open the file	
            dup2(in,0); //have it replace the standard in
            close(in); //close in
          }
        }
	
		//set up the command for passing through execv
        for(int i2=0;i2<commands[0].argc;i2++){
          if(strcmp(commands[0].args[i2],"<")==0){

            break;
          }

          if(i2==0){
            cmd=(char **)malloc(sizeof(char *));
            cmd[index]=(char *)malloc(strlen(commands[0].args[i2]) + 1);
            strcpy(cmd[index++], commands[0].args[i2]);
          }else{
          cmd=realloc(cmd,sizeof(char *)*(i2+1));
          cmd[index]=(char *)malloc(strlen(commands[0].args[i2]) + 1);
          strcpy(cmd[index++], commands[0].args[i2]);
        }
        }

        //add in the NULL at the end
        cmd=realloc(cmd,sizeof(char *)*(index+1));
        cmd[index]=NULL;


        commands[0].args = cmd;

        //replace the stdout of this process with the write of the first pipe
        dup2(fd[1],1);
        
	}else{//no input redirection so just do the standard pipe
	 dup2(fd[1],1);
	}
      }else if(k==(numberofpipes)){
		  //if outbool is true that means we have to redirect the output out
	if(outbool == TRUE){ 
		//find and open the out file
		for(int i2=0;i2<commands[k].argc;i2++){
          if(strcmp(commands[k].args[i2],">")==0){
            out = open(commands[k].args[i2+1],O_WRONLY | O_TRUNC | O_CREAT, S_IRUSR | S_IRGRP | S_IWGRP | S_IWUSR); //open the file out
            dup2(out,1); //have it replace the stdout 
            close(out); //close the out file

          }
        }
		
		//set up the commands for execv
        for(int i2=0;i2<commands[k].argc;i2++){
          if(strcmp(commands[k].args[i2],">")==0){

            break;
          }//end if

          if(i2==0){
            cmd2=(char **)malloc(sizeof(char *));
            cmd2[index2]=(char *)malloc(strlen(commands[k].args[i2]) + 1);
            strcpy(cmd2[index2++], commands[k].args[i2]);
          }else{
          cmd2=realloc(cmd2,sizeof(char *)*(i2+1));
          cmd2[index2]=(char *)malloc(strlen(commands[k].args[i2]) + 1);
          strcpy(cmd2[index2++], commands[k].args[i2]);
        }// end else
      }//end for

        //add in the NULL at the end
        cmd2=realloc(cmd2,sizeof(char *)*(index2+1));
        cmd2[index2]=NULL;

        commands[k].args = cmd2;
        dup2(fd[0+(2*numberofpipes-2)],0); //replace the stadin to the pipe
	}else{	//no output redirection so just replace the stdin with the pipe
		dup2(fd[0+(2*numberofpipes-2)],0);
	}
	//cannot have redirection of the stderror so all pipes between the ends just keep replacing stdins and outs with the pipe
      }else{
        //replace standin with read end of previous pipe
        dup2(fd[i-2],0);
        //replace the stdout with the write end of the next pipe
        dup2(fd[i+1],1);
      }
	//close all unused pipes
      for(int j=0;j<numberofpipes*2;j++){
        close(fd[j]);
      }

      /*Checks the current directory for the program to run if it's not there
      it checks the path. If it's not there it reports the error and exits.*/


      char temp[1024];
      sprintf(temp, "./%s", commands[k].args[0]);

      if(execv(temp, commands[k].args)<0){

        for(int i =0;i<numpaths;i++){
        char temp2[1024];
        sprintf(temp2, "%s/%s",path[i], commands[k].args[0]);

        execv(temp2,commands[k].args);
      }
        perror("flop");
        exit(1);

      }
    }else if(pid<0){
      perror("flop");
    }else{
      continue;
    }
  }
	//close all unused pipes so parent can wait
  for(int j=0;j<numberofpipes*2;j++){
    close(fd[j]);
  }

//only the parent process can make it here
  do {
    pid_t  wpid = waitpid(pid, &status, WUNTRACED);
  } while (!WIFEXITED(status) && !WIFSIGNALED(status));

      return 1;

}//end piperedirections
