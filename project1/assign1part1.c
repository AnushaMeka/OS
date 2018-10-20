#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <errno.h>
#include <string.h>
#include <alloca.h>
#include <math.h>
#include <sys/stat.h>

#define REAL_PAGE_SIZE 4096
#define PHY_SIZE 1024
#define PAGE_SIZE 128          
char *sequenceFileName;


unsigned long hexadecimalToDecimal(char hexVal[])
{   
    int len = strlen(hexVal);
    int base = 1;
    int dec_val = 0;
    for (int i=len-1; i>=0; i--) {
        if (hexVal[i]>='0' && hexVal[i]<='9') {
            dec_val += (hexVal[i] - 48)*base;
            base = base * 16;
        }
        else if (hexVal[i]>='A' && hexVal[i]<='F') {
            dec_val += (hexVal[i] - 55)*base;
            base = base*16;
	}
        else if (hexVal[i]>='a' && hexVal[i]<='f') {
            dec_val += (hexVal[i] - 87)*base;
            base = base*16;
	}
    }
     
    return dec_val;
}

char * decToHexa(unsigned long n) 
{    
    char hexaDeciNum[100]; 
    char *hexa = (char *) malloc(sizeof(char) * 16);  
    int i = 0; 
    int k = 0;
    while(n!=0) {    
        int temp  = 0; 
        temp = n % 16;  
        if(temp < 10) {
            hexaDeciNum[i] = temp + 48; 
            i++; 
        } 
        else {
            hexaDeciNum[i] = temp + 87; 
            i++; 
        }   
        n = n/16; 
    }  
    for(int j=i-1; j>=0; j--) {
        hexa[k] = hexaDeciNum[j];
        k++;
    }
    return hexa;
} 


void getPage(unsigned long logical_address, unsigned long offset_result, FILE **fileIO){
    int a;
    int frameNumFind;
    unsigned long physical_address;
    int frameNum[7] = {2,4,1,7,3,5,6};
    frameNumFind = logical_address/PAGE_SIZE;
    physical_address = frameNum[frameNumFind]*PAGE_SIZE+offset_result;
    char *returned_hexa = decToHexa(physical_address);
    //FILE *output = fopen("output-part1","a");
    fprintf(*fileIO," %s \n",returned_hexa);
    //fflush(output);
    //fprintf(output," %s \n",returned_hexa);
    //fclose(output);
}

size_t alignup(size_t size, size_t alignto) {
  return ((size + (alignto - 1)) & ~(alignto - 1));
}

void analyzeAccessSequenceFromFile(char * fileName) {
	// Open this file 
	struct stat st;
	unsigned long filesize, mapsize;
	unsigned long logical_address, offset_result;
	int i,fd;
        char buffer[100];
	unsigned long * memAccesses;
	stat(fileName, &st);

	filesize = st.st_size;

	fd = open(fileName, O_RDONLY); 
	if(fd == -1) {
		fprintf(stderr, "fd is invalid, with error %s\n", strerror(errno));
		exit(-1);
	}

	// Compute the aligned size;
	mapsize = alignup (filesize, REAL_PAGE_SIZE);
	memAccesses = (unsigned long *)mmap(0, mapsize, PROT_READ, MAP_PRIVATE, fd, 0);
	if(memAccesses == MAP_FAILED) {
		fprintf(stderr, "mmap the input file failed. \n");
		exit(-1);
	}		

        FILE *output = fopen("output-part1","w+");
	fprintf(stderr, "map starting %p filesize %ld\n", memAccesses, filesize);
	i = filesize/sizeof(unsigned long);        
	for(i = 0; i < filesize/sizeof(unsigned long); i++) {
		//printf("%d: %lx\n", i, memAccesses[i]);      
                sprintf(buffer, "%lx", memAccesses[i]);
                logical_address = hexadecimalToDecimal(buffer); 
        	offset_result = logical_address%PAGE_SIZE;
        	// get the physical address
        	getPage(logical_address, offset_result, &output);
    	}

	//close(sequenceFileName);
}

int main(int argc, char *argv[])
{
    int offset_result;
    sequenceFileName = argv[1];
   
    // Check whether the sequence file is existing and I can access this file?
    if(access(sequenceFileName, F_OK ) == -1 ) {
        fprintf(stderr, "The sequence file %s is not existing\n", sequenceFileName);
	exit(-1);
    } 

    // perform basic error checking
    if (argc != 2) {
        fprintf(stderr,"Usage: ./a.out [input file]\n");
        return -1;
    }
    analyzeAccessSequenceFromFile(sequenceFileName);
    return 0;
}

