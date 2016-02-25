/** showfat **/

#include "showfat.h"

int main (int argc, char * argv[]) {
    
    //we need to only output the first 256 entries
    //Three bytes store two FAT entries, therefore, 1.5 bytes store 1 entry
    // 1.5 * 256 = 384
    size_t req = 384;


    //open up the floppy for reading
    int img = open("floppy.img", O_RDONLY);

	//lseek to location of the BYTES_PER_SECTOR in the boot partition and read it
    unsigned int BYTES_PER_SECTOR;
    lseek(img, 11, SEEK_SET);
    read(img, &BYTES_PER_SECTOR, 2);   

    //temporary buffers
    uint8_t fatable[req];
    uint32_t before;  
    uint32_t after; 
    unsigned int j = 2;

    //lseek to the location of the fat table
    //skipping F0 FF FF (2 bytes)
    //read the fat into fatable
    lseek(img, BYTES_PER_SECTOR + 1, SEEK_SET);
    read(img, &fatable, req);  
    //print header
    printf("\t0\t1\t2\t3\t4\t5\t6\t7\t8\t9\tA\tB\tC\tD\tE\tF\n0\t\t\t");

    //shift bits into readable format and dump it
   	for (size_t i = 0; i < req; ++i){
   		if (i%3 == 0 && i != 0) {
		//Two FAT12 entries are stored into three bytes; 
		//if these bytes are uv,wx,yz then the entries are xuv and yzw.
	   	 //Let's first reverse the order to yz wx uv and push them into one big lump, yzwxuv
	 	   before = fatable[i - 1] + (fatable[i] << 8) + (fatable[i + 1] << 16);

         	   //make a copy
         	   after = before;

		    //push the right bits out to get yzw, since order is reversed, this is after
		    after >>= 12;
		    //run an AND function on the second half to mask up the yzw
		    before &= (1 << 12) - 1;
            
		    //sa
		    //if (j == 0) printf("\t\t");
		    //check if any of them is actually 0 and dump
		    if (before == 0 && after == 0) printf("FREE\tFREE\t");
		    else if (before == 0) printf("FREE\t%X\t", after);
				else if (after == 0) printf("%X\tFREE\t", before);
		    else printf("%X\t%X\t", before, after);
		    

		    j += 2;
		    if (j%16 == 0) printf("\n%X\t", j);

		}
   	}
   	printf("\n");

    return 0;
}
