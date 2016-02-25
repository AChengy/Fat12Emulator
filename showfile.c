#include "showfile.h"

int img;
uint16_t *fat;
char files[4100][11];

struct BootEntry boot;
struct FileEntry *entry;

int main(int argc, char *argv[]){
	
	img = open("floppy.img", O_RDONLY);
	
    //we are in the boot sector
    lseek(img, 11, SEEK_SET); //skip 11 bytes for reserved
    read(img,&boot.BYTES_PER_SECTOR,2);

    read(img,&boot.SECTORS_PER_CLUSTER,1);

    read(img,&boot.RESERVED_SECTORS,2);

    read(img,&boot.NUM_OF_FATS,1);

    read(img,&boot.MAX_ROOT_DIRS,2);;


    read(img,&boot.TOTAL_SECTORS,2);
    lseek(img, 1, SEEK_CUR); //skip 1 byte

    read(img,&boot.SECTORS_PER_FAT,2);

    read(img,&boot.SECTORS_PER_TRACK,2);

    read(img,&boot.NUM_OF_HEADS,2);
    lseek(img, 11, SEEK_CUR); //skip 11 bytes

    read(img,&boot.VOLUME_ID,4);

    read(img,&boot.VOLUME_LABEL,11);

    //Move to beginning of ROOT DIRECTORY
    lseek(img, ((boot.NUM_OF_FATS * boot.SECTORS_PER_FAT) + 1) * boot.BYTES_PER_SECTOR, SEEK_SET);

    entry = (struct FileEntry *)malloc(sizeof(struct FileEntry) * boot.MAX_ROOT_DIRS);

    int i = 0;
    for (i = 0; i < boot.MAX_ROOT_DIRS; i++) {
        //FILENAME max 8 bytes
        read(img,&entry[i].FILENAME,8);

        //EXT max 3 bytes
        read(img,&entry[i].EXT,3);
        
        //ATTRIBUTES max 1 byte
        read(img,&entry[i].ATTRIBUTES,1);
        
        //RESERVED  max 10 bytes
        read(img,&entry[i].RESERVED,2);

        //CREATION_TIME max 2 bytes
        read(img,&entry[i].CREATION_TIME,2);

        //CREATION_DATE max 2 bytes
        read(img,&entry[i].CREATION_DATE,2);

        //LAST_ACCESS_DATE max 2 bytes
        read(img,&entry[i].LAST_ACCESS_DATE,2);
        lseek(img, 2, SEEK_CUR); //skip 2 bytes

        //MODIFY_TIME max 2 bytes
        read(img,&entry[i].MODIFY_TIME,2);

        //MODIFY_DATE max 2 bytes
        read(img,&entry[i].MODIFY_DATE,2);

        //START_CLUSTER max 2 bytes
        read(img,&entry[i].START_CLUSTER,2);
		
        //FILE_SIZE max 4 bytes
        read(img,&entry[i].FILE_SIZE,4);

        //If the this file was actually nonexistant, break loop and set NUM_OF_FILES to i - 1
        if(entry[i].FILENAME[0] == '\0'){
            entry->NUM_OF_FILES = (unsigned short)(i - 1);
            break;
        }

    }
    
    //printf("%d\n", entry->NUM_OF_FILES);
    //Parse the File Allocation Table, and shift bits to a readily readable format

    //raw storage for reading the fat
    uint8_t fatable[boot.BYTES_PER_SECTOR];

    //move to beginning of the FAT sector and read into the above storage
    //skipping F0 FF FF (2 bytes)
    lseek(img, boot.BYTES_PER_SECTOR + 1, SEEK_SET);
    read(img, &fatable, boot.BYTES_PER_SECTOR);

    //initialise the data structure for the uncovered fat
    fat = malloc (boot.BYTES_PER_SECTOR * sizeof (uint16_t));

    unsigned int j;
    i = 0;

    //start parsing and converting the fat
    //j = 0, remember F0 FF FF is SKIPPED
    for (j = 0; j < boot.BYTES_PER_SECTOR; j++) {
        if (j % 3 == 0 && j != 0) {
            //Two FAT12 entries are stored into three bytes; if these bytes are uv,wx,yz then the entries are xuv and yzw.
            //Let's first reverse the order to yz wx uv and push them into one big lump, yzwxuv
            uint32_t before = 0;
            before = fatable[j - 1] + (fatable[j] << 8) + (fatable[j + 1] << 16);

            //make a copy
            uint32_t after = before;

            //push the right bits out to get yzw, since order is reversed, this is after
            after >>= 12;
            //run an AND function on the second half to mask up the yzw
            before &= (1 << 12) - 1;

            //copy readable values into the table
            fat[i] = (uint16_t) before;
            fat[i + 1] = (uint16_t) after;
            i += 2;
        }

    }

    for(i = 0; i < boot.MAX_ROOT_DIRS;i++){

        if(entry[i].FILENAME[0] != 0x00 && entry[i].START_CLUSTER != 0){
            if(entry[i].ATTRIBUTES[0] == 0x10){
                //SET appropraiate file name
                char file[9];
                int z;
                //Remove spaces that are used for padding
                for(z = 0; z < 8; z++){
                    if(entry[i].FILENAME[z] == ' ' || z == 7){
                        if(z == 7){
                            file[z+1] = '\0';
                            break;
                        }
                        file[z] = '\0';
                        break;
                    }else{
                        file[z] = entry[i].FILENAME[z];
                    }
                }
                strcpy(files[i], file);
            }else{

                //SET appropriate file name
                char file[9];
                int z;
                for(z = 0; z < 8; z++){
                    if(entry[i].FILENAME[z] == ' ' || z == 7){
                        if(z == 7){
                            file[z+1] = '\0';
                            break;
                        }
                        file[z] = '\0';
                        break;
                    }else{
                        file[z] = entry[i].FILENAME[z];
                    }

                }
                char tmp[11];
                strcpy(tmp, strcat(file, "."));
                strcat(tmp,entry[i].EXT);
                strcpy(files[i], tmp);
                //printf("/%s%s\n",file,entry[i].EXT);
                //printf("%s\n", tmp);
                //printf("%s\n", files[i]);
            }
        }

    }


    //find the physical offset of the beginning of the data partition and move to it
    size_t firstdatapartition = ((boot.MAX_ROOT_DIRS/16) + (boot.SECTORS_PER_FAT*boot.NUM_OF_FATS)) + 1;
    lseek(img, firstdatapartition, SEEK_SET);

    //buffer table to hold the read data
    unsigned char *buf = malloc (boot.BYTES_PER_SECTOR);

    //get the file cluster offset in the fat

    i = 0;

    //+ 1 removes leading /

    char * thing = strcat(argv[1], " ") + 1;

    //char * thing = argv[1];
    printf("'%s'\n", thing);
    while(i < 100){
        //printf("'%s'\n", files[i]);

        //char * tmp = entry[i].files;
        //printf("%d\n", strcmp(thing, files[i]));
        if (strcmp(thing, files[i]) == 0){
            break;
        }
        i+=1;
    }
    if (i == 100) return 50;
    //j = 0;

    i = entry[i].START_CLUSTER - 2;

    printf("%d\n", i);
    printf("\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\ta\tb\tc\td\te\tf");
    //keep reading the fat directories until you hit a 0x0FF8 or greater)
    while(1){
        //if (j%16 == 0) printf("\n%X\t", j);
        if (fat[i] != 0xFFF){

            //lseek to the cluster the fat entry points to
            lseek (img, (i + firstdatapartition) * boot.BYTES_PER_SECTOR, SEEK_SET);
            //read the sector into buf
            read(img, buf, boot.BYTES_PER_SECTOR);

            for (j = 0; j < boot.BYTES_PER_SECTOR; j++){
                if (j%16 == 0) printf("\n%X\t", j);
                printf("%X\t", buf[j]);
            }
        } else {
            //print one more time

            //lseek to the cluster the fat entry points to
            lseek (img, (i + firstdatapartition) * boot.BYTES_PER_SECTOR, SEEK_SET);
            //read the sector into buf
            read(img, buf, boot.BYTES_PER_SECTOR);

            for (j = 0; j < boot.BYTES_PER_SECTOR; j++){
                if (j%16 == 0) printf("\n%X\t", j);
                printf("%X\t", buf[j]);
            }
            break;
        }

        i = (unsigned int)fat[i] - 2;
    }
    printf("\n");
    return 0;
}