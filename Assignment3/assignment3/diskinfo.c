#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>


 int getfreesize();
 void *getlabel();
 int shifting();
 void *sector();
 void storevalue();
 int rootfile();
 int bytespersec;
 int sectorpercluster;
 int numberoffats; 
 int numberofreservedsectors;
 int totals;
 int sectorperfat;
 char *name;
 char *original;
 struct stat sf;
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
       printf("Your input file doesn't make the corret extesion(ima or  IMA)\n");
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
        //Success 
   storevalue(original);
   printf("OS Name: %s\n",name);
   label =  getlabel(p);
  printf("Label of the disk: %s\n",label);  
  int totalsize = bytespersec*totals;  
  printf("Total size of the disk: %d bytes\n",totalsize);
  usedsize = getfreesize(p,totalsize);
  p = original;
  int rf = rootfile (p);
  printf("Free size of the disk: %d bytes\n",totalsize -(usedsize*bytespersec));  // total size -(512 * fat entries)
  printf("==============\n");
  printf("The number of files in the root directory (not including subdirectories): %d\n",rf);
  printf("==============\n");
  printf("Number of FAT copies: %d\n",numberoffats);
  printf("Sectors per FAT: %d\n", sectorperfat);
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
int rootfile(char *p){
    
 int temp = 0;
 int NumberofFiles = 0;
 char *filename;
 p = original + (bytespersec*(sectorperfat*numberoffats+1)); // move to root directory
 for (temp;temp < 16 *13 ;temp++){
     filename = sector(p,3,8); // Only file can have an extnsion, which is not null.
     if(shifting(p,11,1) != 15 && shifting(p,11,1) != 8&&shifting(p,11,1) != 16 &&filename[0] != '\0') // long file and subdirectory can be ingnored (0x0F), and each file has its extension.
     NumberofFiles++;
     p +=32;
     
 }
    
    return NumberofFiles;
}  // find a number of files in the root directory exclude sub directory
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
void *getlabel(char *p){
    
    int a;
    int b = 0;
    int i;
    char *getname = malloc(sizeof(char));
    if(p = "           "){
         
      p = original + (bytespersec*(sectorperfat*numberoffats+1));   //  number of fats * number of sectors for the fat table + boot sector(1) change the current sector to root directory 
     for(i=0;i<=16 * 13;i++){
         
     
      a = shifting(p,11,1); // a is an atrribute.
    
      if(a == 8){ // if it is a volume label , break
      b++;   
      break;
     }
     p+=32; // add 32 to p, moves to anoter directory entry.
     }
     if(b == 0){
          printf("Error: Volume does not exist \n");
          exit(EXIT_FAILURE);
      }  
     
      }
    return p;

}  // returns volume label.
int shifting(char *p,int q,int len){
  char* s = sector(p,q,len);     
  int *temp = (int *)malloc(100);
   int i;
   int retval = 0;

   for(i=len-1;i>=0;i--){
     temp[i] = s[i];
     retval = (retval << 8) + (0xFF & temp[i]); // because it is an little endian
   }
  
   return retval;
}// find an integer value from the file system
void *sector(char *p,int q,int size){
       
       char *fat= (char *)malloc(100);
       memcpy(fat,p+q,size);
       fat[size] = '\0';
       return fat;
   } // find a string from the boot sector or root directory
  