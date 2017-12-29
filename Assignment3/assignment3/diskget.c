#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
int Fatentry();
int shifting();
void *sector();
void storevalue();
int loaction();
void getfile();
struct stat sf;
char *original;
int bytespersec;
int sectorpercluster;
int numberoffats; 
int numberofreservedsectors;
int totals;
int sectorperfat;
char *name;
int main(int agrc,char *agrv[]) {
   int fd;
   int usedsize;
   char *label;
   char *a = agrv[1];
   char *P =agrv[2];
    if( a == NULL){
         printf(" Please type your input file\n");
         exit(EXIT_FAILURE);
      } //check if user typed right input file.
      if(P==NULL){
        printf("Object file not found.\n");
        exit(EXIT_FAILURE);
    }
   fd = open(a,O_RDONLY);
   
   if (fd < 0){
        printf("Error: File does not exist. \n");
	    exit(EXIT_FAILURE);
	} //check if file exists
	
     int b = strlen(agrv[1]);
     const char *check = &a[b-4]; // check if the argv has a extesion ".IMA" or .ima  
      
    if(strcmp(".IMA", check)!=0&&strcmp(".ima", check)!=0){
       printf("Your input file doesn't make the corret extesion (ima or  IMA) \n");
       exit(EXIT_FAILURE);
   } //check if  the input has the correct extension.
   
    if (fstat (fd,&sf) < 0) {
        printf ("fstat error");
        exit(EXIT_FAILURE);
    }//check if there is a fstat error
    char *p = mmap(NULL,sf.st_size,PROT_READ,MAP_SHARED,fd,0);
    if(p==(caddr_t) -1){
        printf ("mmap error");
        exit(EXIT_FAILURE);
    }  //check if there is a mmap error
    original = p;
    storevalue(p);
    int  num  = loaction(p,agrv[2]); // find a file location
    getfile(p,num,agrv[2]);
    close(fd);
    munmap(p,sf.st_size);
    
}
int loaction(char *p,char *file){
    int i = 0;
    int filelocation;
    char *filecheck = (char *)malloc(1000);; // extension
    char *name;
    int length;  // length counts without spaces 
    char *filename; // extension
    char *extension;
    int extensionlength;
    int exist = 0;
   p = original + (bytespersec*(sectorperfat*numberoffats+1));
    for (i;i < (16*13);i++){
        int j  = 0;
         filename = sector(p,8,3);
         if(shifting(p,11,1) != 15 && shifting(p,11,1) != 8&&filename[0] != '\0'){
        length = 0;
        extensionlength = 0;
        for (j=0;j<8;j++){
              if(strcmp((sector(p,j,1))," ")==0){
              length++;
              }
          }
           for (j=8;j<11;j++){
              if(strcmp((sector(p,j,1))," ")==0){
              extensionlength++;
              }
          }
           name = sector(p,0,8-length); // filename without spaces
           extension = sector(p,8,11-extensionlength);
           sprintf(filecheck,"%s.%s",name,extension);
          if(strcmp(filecheck,file)==0) // check if  they have the same file name
          exist++;
          break;
         }
        p +=32; // 32 bytes per one directory
    }
    filelocation = i*32;
   if(exist == 0){
       printf("File not found\n");
       exit(EXIT_FAILURE);
   }
   else{
       //File exists
       p = original;
       return filelocation;
   }
}// find a loaction of the file and returns its location
void getfile(char *p,int a,char *file){
 p = original + (bytespersec*(sectorperfat*numberoffats+1));
  int getbyte = shifting(p,a+28,4);
  int firstcluster = shifting(p,a+26,2);
  int* fatvalues;
  p = original;
  p = p +bytespersec;
  int value;
  int i;
  int count = 0;
  fatvalues[count] = firstcluster +31;
  value = Fatentry(p,firstcluster);
  count++;
  fatvalues[count] = value +31; // fatentries
  for (i=0;i<4080;i++){
   
   value = Fatentry(p,value);
    if(value >=0xFF0||value == 0){
   count++;
   break;
    }
    count++;
    fatvalues[count] = value +31;
  }
  p = original;
FILE *myF = fopen(file, "w");
if(myF == NULL){
 printf("file error");
 exit(EXIT_FAILURE);
}
char *map =  malloc(sizeof(char)* bytespersec) ;
for(i=0;i<count;i++){
 
 map =  memcpy(map, p+ (bytespersec*fatvalues[i]), bytespersec); // print sector by sector.
 if(i == count-1){

  fwrite(map, 1, getbyte, myF); // last thing 
  fclose(myF);
  break;
 }
fwrite(map, 1, bytespersec, myF);
}
} // creates file
void storevalue(char*p){ 
 name = sector(p,3,8);    
 bytespersec = shifting(p,11,2);
 sectorpercluster= shifting(p,13,1);
 numberoffats= shifting(p,16,1);
 numberofreservedsectors= shifting(p,14,2);
 totals= shifting(p,19,2);
 sectorperfat= shifting(p,22,2);
} // store all of the necessary values.
int shifting(char *p,int q,int len){
  char* s = sector(p,q,len);     
   unsigned *temp = (unsigned *)malloc(100);
   int i;
   int retval = 0;
   int count = 0;
   for(i=len-1;i>=0;i--){
     temp[i] = s[i];
     retval = (retval << 8 )+ (0xFF & temp[i]); // because it is an little endian
     count++;
   }
  
   return retval;
}// find an integer value from the file system
void *sector(char *p,int q,int size){
       
       char *fat= (char *)malloc(100);
       memcpy(fat,p+q,size);
       fat[size] = '\0';
       return fat;
   } // find a string from the boot sector or root directory
int Fatentry(char *p,int a){
    int entry;
    
    if(a%2==1){
  int highbits = shifting(p, (3*a)/2,1);
		int lowbits = shifting(p, 1 + (3*a)/2,1);
		highbits &= 0xF0; 
	 	entry = (lowbits<<4) | (highbits>>4); 

	 	return entry;
	
    }  // when a is an odd number
    else{
       
		int Lowbits = shifting(p, 1+(3*a)/2,1);
		int highbits = shifting(p, (3*a)/2, 1);
	 	entry = (Lowbits<<8) | highbits; 
		 	entry &= 0x0FFF;
	 	return entry;
    } // when a is an even number
    
    
    
    
} // According to the tutorial slide, convert 8bits to 12 bits