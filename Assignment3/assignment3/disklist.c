#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
int shifting();
void *sector();
void storevalue();
void printfiles();
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
   
    if( a == NULL){
         printf(" Please type your input file\n");
         exit(EXIT_FAILURE);
      } //check if user typed right input file.
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
    printfiles(p);
    close(fd);
    munmap(p,sf.st_size);
    
}
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
void printfiles(char *p){
      int i =0;
      int j =0;
      p = original + (bytespersec*(sectorperfat*numberoffats+1));
      char *name;
      char *creationtime;
      char *creationdate;
      char *filename;
      int filesize;
      char *extend;
      int length;
      for (i;i < (16*13);i++){  // number of directories * number of sectors for root directory
      length = 0;
      filename = sector(p,3,8); // Only file can have an extnsion, which is not null.
      if(shifting(p,11,1) != 15 && shifting(p,11,1) != 8&&filename[0] != '\0'){ // long file and subdirectory can be ingnored (0x0F), and each file has its extension.
          if(shifting(p,11,1) == 16){ // it is a file directory
          for (j;j<8;j++){
              if(strcmp((sector(p,j,1))," ")==0){
              length++;
              }
          }
         
        name = sector(p,0,8-length);
          int CT = shifting(p,22,2);
          int CD = shifting(p,24,2);
          int yy = (CD >> 9)+1980;
          int mm = (CD & 0x1FF) >> 5;
          int dd = CD & 0x1F;
          int t = CT >> 11;
          int m = (CT & 0x7FF) >> 5;
          filesize = shifting(p,28,4);
          extend = sector(p,8,3);
          
           
          printf("F\t%d bytes\t%s.%s\t%d/%d/%d\t%d:%d\n",filesize,name,extend,yy,mm,dd,t,m);
              
          } else{  // it is a file 
   
        
          for (j;j<8;j++){
              if(strcmp((sector(p,j,1))," ")==0){
              length++;
              }
          }
         
          name = sector(p,0,8-length);
          int CT = shifting(p,14,2); // time in decimal
          int CD = shifting(p,16,2); // date in decimal
          int yy = (CD >> 9)+1980;  // 15~9 bits (little endian)
          int mm = (CD & 0x1FF) >> 5;// 8~5 bits (little endian)
          int dd = CD & 0x1F;  // 5 ~0 bits (little endian)
          int t = CT >> 11;  // 15~11 bits
          int m = (CT & 0x7FF) >> 5; // 10~5 bits
          int s = (CT &0x1F)*2;
          filesize = shifting(p,28,4);
          extend = sector(p,8,3);
          
           
          printf("F\t%d bytes\t%s.%s\t%4d/%2d/%2d\t%2d:%2d:%2d\n",filesize,name,extend,yy,mm,dd,t,m,s);
          }
      }
      p+=32;
      }
    
} // print file information