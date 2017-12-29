#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <stdlib.h>     // malloc()
#include <sys/types.h>  // pid_t 
#include <sys/wait.h>   // waitpid()
#include <signal.h>     // kill(), SIGTERM, SIGKILL, SIGSTOP, SIGCONT
#include <errno.h>      // errno
#include <stdbool.h>
#define MAXPROC 300
typedef struct {
    pid_t pid;
    char *cmd;
    bool *isstop;
} proc;

proc *proc_list[MAXPROC]; // global proclist
int number =0 ; // proc count



 void update_bg_procss(){  //updating process
      
     int checkpid;
     int status;
     checkpid = waitpid(-1, &status, WNOHANG);
     while(1){
         if(checkpid ==0){
             break;
         }
         
         if(checkpid>0){ // remove the process from the list
             int i;
             int j;
             //using j to remove proc
            for(i=0;i<number;i++){
                
                if(proc_list[i]->pid==checkpid){
                printf("%s is terminated  with pid : %d \n",proc_list[i]->cmd,proc_list[i]->pid);
                number--;
                j = i;
                break;
                }
            }
            for(i =j; i < number;i++){
                proc_list[i] = proc_list [i+1];
            }
            break;
         }
         else{
             break;
         }
     }
 }

int main(){ 
    
    int status;
    char *input = NULL ;
    char *prompt = "PMan: >";
    
    while(1){
        
        
    //update_bg_procss();
    
    input = readline(prompt);
    
    
    int num = 0;
    char *a = strtok (input, " ");
    char *array[300];
    char* bg = a;
    int count =0;
    int result;
    a = strtok (NULL, " ");
   while (a != NULL)
    {
        array[num] = a;
        a = strtok (NULL, " ");
        
        num++;
        count++;
    }
          array[num]=NULL;
       
         //if nothing's typed
        if(bg == NULL){
         //printf("You typed Nothing\n");
        }  
        
         else{
           if(strcmp(bg, "bg")==0){
           
          //PSTAT
           if(strcmp(array[0], "pstat")==0){
             
             int i;
             int pid  = atoi(array[1]);
             printf("Printing status, pid: %d\n ",pid); 
            
            char bitmap[2000];
            sprintf(bitmap,"/proc/%d/stat",pid);
             FILE *file = fopen(bitmap,"r");
             if(file == NULL){
                 printf("Error: Process %d does not exist.\n",pid);
             }
             else{
                  
                  // success
                  char *read;
                  char *p = NULL;
                  size_t q = 0;
                  getline(&p,&q,file);
                  //
                  int i =0;
                  read = strtok(p, " ");
                   //token
                   
                   while (read != NULL){
                       switch(i){
                        //refer to man proc
                       case 1:
                       printf("comm: %s\n", read);
                       break;
                       case 2:
                       printf("state: %s\n", read);
                       break;
                       case 13:
                       printf("utime: %s\n", read);
                       break;
                       case 14:
                       printf("stime: %s\n", read);
                       break;
                       case 23:
                       printf("rss: %s\n", read);
                       break;
                       }
                       
                      read = strtok(NULL, " ");
                      i++;
                   }
                   fclose(file);
                   //move on to /proc/%d/status; reopen the file.
                   
                   
                    sprintf(bitmap,"/proc/%d/status",pid);
                    FILE *file2 = fopen(bitmap,"r");
                  
                     char *read2;//token
                     char  *p2;
                     size_t q2 = NULL;
                   //  getline(&p2,&q2,file2);
                     
                     char* volun = "voluntary_ctxt_switches";
                     char* nonvolun = "nonvoluntary_ctxt_switches";
                    //refer to man proc
                    
                    while( getline(&p2,&q2,file2) != -1)
                           {
                               read2 = strtok(p2, " :");
                    if(read2!=NULL){
                        
                        

                        if(strcmp(read2, volun) == 0){
                           read2 = strtok(NULL, " :\n");
                           printf("voluntary_ctxt_switches , %s\n",read2);
                            
                        }
                        if(strcmp(read2, nonvolun) == 0){
                           read2 = strtok(NULL, " :\n");
                           printf("nonvoluntary_ctxt_switches , %s\n",read2);
                        }
                       }
                      // read2 = strtok(NULL, " ");
                           }
                        
                    }
                    
                    
                   
         }  // pstat done
        else{
            
         int i;
         bg = NULL;
         proc_list[number] = (proc*) malloc (sizeof(proc)); 
         proc_list[number]->pid = fork();
         
          if(proc_list[number]->pid ==0){
              
            if (execvp( array[0], array) < 0){
                
    	       perror("Error on execvp");
    	       exit(EXIT_FAILURE);
        	   
           }
           
         }// if it's a child process 
    else if(proc_list[number]->pid <0){
        printf("error \n");
    } 
    else
    {
         
      sleep(1);
      
      int i;
      proc_list[number]->cmd=array[0];
      for (i=1;i<count;i++){
       strcat(proc_list[number]->cmd,array[i]);
      }
  
      number++;   
     
     } // if it is a parent process
     update_bg_procss();
    }
      }
    
  else if(strcmp(bg, "bglist")==0){
         update_bg_procss();
         int i;
         for (i=0; i< number; i++){
             
             printf("Pid : %d, Process name: %s \n",proc_list[i]->pid,proc_list[i]->cmd);
              }
              
             printf(" Number of jobs : %d \n",number);
    }  //bg list is done
    
  else if(strcmp(bg, "bgkill")==0){
      
      int i;
      int pid  = atoi(array[0]);
      
      for(i =0;i < number; i++){
          
          if(proc_list[i]->pid==pid){
            
            kill(proc_list[i]->pid, SIGKILL);
            
            printf("%d  is killed\n",pid);
            
          }
      }
  }  // bg kill is done
  else if(strcmp(bg, "bgstop")==0){
       int i;
      int pid  = atoi(array[0]);
      
      for(i =0;i < number; i++){
          
          if(proc_list[i]->pid==pid){
            
            kill(proc_list[i]->pid, SIGSTOP);
            
            printf("%d  is stopped\n",pid);
            
          }
      }
      
  } // bg stop is done
  else if(strcmp(bg, "bgstart")==0){
       int i;
      int pid  = atoi(array[0]);
      
      for(i =0;i < number; i++){
          
          if(proc_list[i]->pid==pid){
            
            kill(proc_list[i]->pid, SIGCONT);
            
            printf("%d  is  restarted\n",pid);
            
          }
      }//bg start is done
  }
  
  else {
      
      printf("Invalid comment\n");
  }
   }
    }
return 1;
}

