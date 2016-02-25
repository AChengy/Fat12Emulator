#include "structure.h"

int main(int argc, char *argv[]){

    //open up the floppy for reading
    int img = open("floppy.img", O_RDONLY);

    //use only what is needed
    unsigned int BYTES_PER_SECTOR = 0;
    unsigned int SECTORS_PER_CLUSTER = 0;
    //RESERVED_SECTORS, 2
    unsigned int NUM_OF_FATS = 0;
    unsigned int MAX_ROOT_DIRS = 0;
    //TOTAL_SECTORS, 2; + 1
    unsigned int SECTORS_PER_FAT = 0;

    //Ignore the first 11 bytes
    lseek(img, 11, SEEK_SET);

    //read in bytes/sector and sectors/cluster
    read(img, &BYTES_PER_SECTOR, 2);
    read(img, &SECTORS_PER_CLUSTER, 1);

    //Skip number of reserved sectors, we don't need it
    lseek(img, 2, SEEK_CUR);

    //read in number of file alloc tables and maximum number of root dir entries
    read(img, &NUM_OF_FATS, 1);
    read(img, &MAX_ROOT_DIRS, 2);

    //Skip total sector count and one byte
    lseek(img, 3, SEEK_CUR);

    //read in sectors/fat
    read(img, &SECTORS_PER_FAT, 2);

    //print the data
    printf("\n\tMounted\n");
    printf("\t\tnumber of FAT:\t\t\t\t\t\t%d\n", NUM_OF_FATS);
    printf("\t\tnumber of sectors used by FAT:\t\t%d\n", SECTORS_PER_FAT);
    printf("\t\tnumber of sectors per cluster:\t\t%d\n", SECTORS_PER_CLUSTER);
    printf("\t\tnumber of ROOT Entries:\t\t\t\t%d\n", MAX_ROOT_DIRS);
    printf("\t\tnumber of bytes per sector\t\t\t%d\n", BYTES_PER_SECTOR);

    //print header
    printf("\t\t---Sector #---\t\t---Sector Types---\n");

    //Boot is always the first sector.
    printf("\t\t0\t\t\t\t\tBOOT\n");

    //print the location of the fat sectors
    int i;
    for(i = 0; i < NUM_OF_FATS; i++) {
        printf("\t\t%02d -- %02d\t\t\tFAT%d\n", (SECTORS_PER_FAT * i) + 1, SECTORS_PER_FAT * (i + 1), i);
    }

    //print the location of the root directory sectors
    printf("\t\t%02d -- %02d\t\tROOT DIRECTORY\n", SECTORS_PER_FAT * NUM_OF_FATS + 1, (MAX_ROOT_DIRS / 32) + (SECTORS_PER_FAT * NUM_OF_FATS));

    return 1;
}
