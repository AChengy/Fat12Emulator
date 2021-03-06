//
// Created by Alexc on 10/24/2015.
//

#ifndef PROJECT3_FLOP_H
#define PROJECT3_FLOP_H

//libraries
#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/wait.h>
#include <math.h>


//boolean def
#define TRUE    1
#define FALSE   0
#define _GNU_SOURCE

extern int mountcondition;

#include "help.h"

typedef struct command{
  int argc;
  char **args;
}command;

int cd(char **args);
int pathcommand(char **args, int argsc,int flag);
int launchpipedredirection(command *commands, int pipes);
int launchpipes(command *commands,int pipes);
command* breakargs(char **args,int argc, int pipes);
int launch(char **args,int argc);
int launchredirections(char **args, int argc);
int numofpipes(char **args, int argc);
int numofredirections(char **args, int argc);

#endif //PROJECT3_FLOP_H
