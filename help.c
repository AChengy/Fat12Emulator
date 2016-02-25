//
// Created by Alexc on 10/24/2015.
//

#include "help.h"


int main(){
    printf("List of Commands:\n");
    printf("path       - can use + or - flags to alter the usable paths for executing\n");
    printf("cd         - changes the current directory. Full path names or .. must be used\n");
    printf("cwd        - shows the current working directory\n");
    printf("traverse   - shows the files stored in the floppy use -l flag for file info\n");
    printf("showsector - takes an int argument and prints the hex values of the corresponding sector\n");
    printf("showfat    - shows first 256 entries in the fat table\n");
    printf("structure  - shows the structure of the floppy\n");
    printf("showfile   - Takes a file name as an argument and shows the file in hex\n");
    printf("help       - shows list of commands\n");
    printf("quit       - quits flop\n");
    printf("exit       - exits flop\n");

return 0;
}

//
// void printmounterror(){
//     printf("error: no image mounted\n");
// }
//
// void printargumentserror(){
//     printf("error: Invalid number of arguments. Type help for more info.\n");
// }
