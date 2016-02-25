/** Showsector **/

#include "showsector.h"

int main(int argc, char * argv[]){
    
    //handle illegal inputs - number of arguments
    if (argc != 2) {
    	printf("Wrong number of arguments\n");
    	printf("Usage: [showsector n] where n is a valid sector number\n");
   		return 0;
   	}
   	
    //open up the floppy for reading
    int img = open("floppy.img", O_RDONLY);
    
    //check if the argv is actually larger than the number of sectors on the floppy
    unsigned int NUM_OF_SECTORS;
    int sector = atoi(argv[1]);
    
    //lseek to location of NUM_OF_SECTORS and read it
    lseek(img, 19, SEEK_SET);
    read (img, &NUM_OF_SECTORS, 2);
    
    if (sector < 0){
    	printf("The floppy does not exist in the fourth dimension.\n");
    	printf("Please use a number greater than 0\n");
   		return 0;
   	}
    
    if (sector > NUM_OF_SECTORS){
    	printf("The floppy is not big enough to contain that sector\n");
    	printf("Use a value smaller than %d\n", NUM_OF_SECTORS);
   		return 0;
   	}
    
    //lseek to location of the BYTES_PER_SECTOR in the boot partition and read it
    unsigned int BYTES_PER_SECTOR;
    lseek(img, 11, SEEK_SET);
    read(img, &BYTES_PER_SECTOR, 2);

    //lseek to the sector requested
    lseek(img, sector * BYTES_PER_SECTOR, SEEK_SET);
    
    //dump the sector and exit
    unsigned char dump;
    unsigned short j = 0;
    printf("\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\ta\tb\tc\td\te\tf\n");
    for (size_t i = 0; i < 512; i++){
        if (j == 0 || j%16 == 0) printf("\n%X\t", j);
        read(img,&dump,1);
        printf("%X\t", dump);
        j++;
    }
    printf("\n");
    return 1;
}
