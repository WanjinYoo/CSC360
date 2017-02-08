#include <unistd.h>
#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
int Fatentry();
void fatput();
int shifting();
void *sector();
void storevalue();
int loaction();
void getfile();
int getfreesize();
void diskput();
struct stat sf;
char *original;
int bytespersec;
int sectorpercluster;
int numberoffats; 
int numberofreservedsectors;
int totals;
int sectorperfat;
char *name;
char *namecopy;
int main(int agrc,char *agrv[]) {
   int fd;
   int usedsize;
   char *label;
   char *a = agrv[1];
   char *P =agrv[2];
   FILE *myF;
    if( a == NULL){
         printf(" Please type your input file\n");
         exit(EXIT_FAILURE);
      } //check if user typed right input file.
      if(P==NULL){
        printf("Object file not found.\n");
        exit(EXIT_FAILURE);
    }
    namecopy = P;
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
    myF = fopen(P, "r");
     if(myF == NULL){
          printf ("File not Found.\n");
          exit(EXIT_FAILURE);
     }
     FILE *store = fopen(a, "r+b");
      if(store == NULL){
          printf ("File not Found.\n");
          exit(EXIT_FAILURE);
     }
    original = p;
    storevalue(p);
    int freesize = getfreesize(p,(bytespersec*totals));
    int temp = freesize;
    freesize = (totals*bytespersec)-(freesize*bytespersec);
   fseek(myF, 0L, SEEK_END);
   int filesize = ftell(myF);
  if(filesize > freesize ){
       printf ("No enough free space in the disk image.\n");
       exit(EXIT_FAILURE);
  }   
  else
      p = original;
      diskput(p,temp,filesize,myF,store);
      munmap(p,sf.st_size);
      close(fd);
      fclose(myF);
    
}
void fatput(char*p,int Fentry,int filesize,FILE *store){
    int i = 0;
    int *temp;
    p =p + bytespersec + (Fentry * 3); // move to fat sector.
    while(i < Fentry){
    *temp = Fatentry(p,i);    
    //fseek(store,i, SEEK_SET); 
    //fwrite(temp, 1, 2, store);
    i++;
    }
} // fat part
void diskput(char*p,int Fentry,int filesize,FILE *myF,FILE *store){
    char rootdirectory[32];
    char memory[32];
    char Fat[32];
    char *check;
    int i = 0;
    int a;
    p = original + (bytespersec*(sectorperfat*numberoffats+1));
    for (i;i < (16*13);i++){
        a = shifting(p,0,8);
        if(a == 0xE5 ||a ==  0x00 ){ // entry is available
        break;
        }
         
        p +=32; // 32 bytes per one directory
    }
    int Rentry = i;
    time_t h = time(NULL);
    struct tm rn = *localtime(&h);
    int hour = rn.tm_hour << 11;
    int min = rn.tm_min << 5;
    int sec = rn.tm_sec / 2;
    int year = (rn.tm_year - 80) << 9;
    int month = (rn.tm_mon + 1) << 5;
    int day = (rn.tm_mday);
    char *Name = malloc(sizeof(char));
    char *extension;
     int x =8;
     int j = 0;
   int lenth = strlen(namecopy);
    for (i = lenth - 1; i >= 0; i--) {
        if (namecopy[i] == '.') {
           strncpy(Name,namecopy, i);
           extension =namecopy+i+1;
           namecopy[i] = '\0';
           for(j;j<i; j++){
           rootdirectory[j] = namecopy[j] ;
           }
         break;
        }
        rootdirectory[x] = namecopy[i] ;
        x++;
    }
    for(x;x<22;x++){
         rootdirectory[x] = 0x20;
    }
    for(j;j<8;j++){
         rootdirectory[j] = 0x20;
    }
    
	rootdirectory[22],rootdirectory[14] = (sec | (min<<5))&  0xFF;
	rootdirectory[23],rootdirectory[15] = ((min >> 3) | (hour << 3)) & 0xFF;
	rootdirectory[24],rootdirectory[16] = (day | (month << 5)) & 0xFF ;
	rootdirectory[25],rootdirectory[17] = ((month >> 3)|(year << 1))& 0xFF;
	rootdirectory[26] = Fentry & 0xFF;
    rootdirectory[27] = (Fentry>>8) & 0xFF;
    rootdirectory[28] =  Fentry & 0xFF;
    rootdirectory[29] =  (filesize>>8) & 0xFF;
    rootdirectory[30] =  (filesize>>16) & 0xFF;
    rootdirectory[31] =  (filesize>>24) & 0xFF;
    fwrite(rootdirectory, 1, 32, store);
    p = original;
    fatput(p,Fentry,filesize,store);
    
    
 // name and extension are done.

}// finished root directory part
int getfreesize(char *p){
   
    int i;
    int count =0;
    int fatentry;
    fatentry = (bytespersec*sectorperfat)/3; // each fat entry has 3 bytes. (512(bytespersec) * 9 (sectorperfat))= number of fat entries
    p +=bytespersec; // enter fat table.
    for(i=0;i<fatentry;i++){
        if (Fatentry(p,i) != 0){  // check if fat entry is empty.
		count++;
		}
    }
    
    return count;
}  // returns number of used place in fat table.
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