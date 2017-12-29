#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include <stdlib.h>
#include <sys/time.h> // timecheck
#define max 1000




typedef struct _flow
{
    float arrivalTime ;
    float transTime ;
    int priority ;
    int id ;
} flow;

struct timeval start[max], end[max],start2,end2;
int run = 0; // checks whether something is running or not 
int check =0; // checks if it is the first thread
float totaltime =0;  // global time calculation
int flownum = 0;  // stores flow number
int counter = 0;
int arr = 0;
flow flowList[max];   // parse input in an array of flow
flow *queueList[max];  // store waiting flows while transmission pipe is occupied.
pthread_t thrList[max]; // each thread executes one flow
pthread_mutex_t trans_mtx = PTHREAD_MUTEX_INITIALIZER ; 
pthread_mutex_t lock = PTHREAD_MUTEX_INITIALIZER ; 
pthread_mutex_t lock2 = PTHREAD_MUTEX_INITIALIZER ; 
pthread_mutex_t timelock = PTHREAD_MUTEX_INITIALIZER ; 
pthread_cond_t trans_cvar = PTHREAD_COND_INITIALIZER ;
 
void requestPipe(flow *item) {
    
    
    pthread_mutex_lock(&trans_mtx); // locks the critical section
    
    if (run ==0 && queueList[0]==NULL) {
         check++;
         flownum =  item->id;
         pthread_mutex_unlock(&trans_mtx);
         gettimeofday(&start2, NULL);
         pthread_mutex_unlock(&timelock);
         return;
    }
    
    
     pthread_mutex_unlock(&timelock);
      pthread_mutex_lock(&lock); //Second critical section for global counter.
       int num = 0;
       int i;
       if(queueList[0] == NULL){
        
        queueList[0] = item; // store the item if queue is empty.
        counter ++;
        }
       else{
     
       while (queueList[num] != NULL){
          num++;
       } // store all of the items in the array without sorting them.
       queueList[num] = item;
       
       while ( num != 0) {

       // check the priority   
       if(queueList[num]->priority<queueList[num-1]->priority){
          flow *hold = queueList[num-1];
		   queueList[num-1] = queueList[num];
	    	queueList[num] = hold;
	         num--;
	       if (num ==0){
	           break;
	       }
         }   // if item has higher priority, swap the value with  queueList[num-1]
      else if(queueList[num]->priority==queueList[num-1]->priority && queueList[num]->arrivalTime==queueList[num-1]->arrivalTime && queueList[num]->transTime==queueList[num-1]->transTime&&queueList[num]->id<queueList[num-1]->id){
            flow *hold = queueList[num-1];
		    queueList[num-1] = queueList[num];
	    	queueList[num] = hold;
	    	num--;
	    	if (num ==0){
	           break;
	       }
         
       }     // if everything is a tie, the one that appear first in the input file will go first
         
       else if(queueList[num]->priority==queueList[num-1]->priority && queueList[num]->arrivalTime==queueList[num-1]->arrivalTime&&queueList[num]->transTime<queueList[num-1]->transTime){
            flow *hold = queueList[num-1];
		    queueList[num-1] = queueList[num];
	    	queueList[num] = hold;
	    	num--;
	    	if (num ==0){
	           break;
	       }
       }   // if item has the same priority and arrival time, then  less transmission time will start first.
       else if (queueList[num]->priority==queueList[num-1]->priority && queueList[num]->arrivalTime<queueList[num-1]->arrivalTime){
           flow *hold = queueList[num-1];
		   queueList[num-1] = queueList[num];
	    	queueList[num] = hold;
	    	num--;
	    	if (num ==0){
	           break;
	       }
       }   // if item has the same priority, less arrival time will go first.
        else{
       num--; 
        } // otherwise just decrement num.
         
           
       }
      
        }
        pthread_mutex_unlock(&lock);
        printf("Flow %2d waits for the finish of flow %2d. \n",item->id, flownum);
        while (item->id != queueList[0]->id  || run ==1 ){
        pthread_cond_wait(&trans_cvar, &trans_mtx);
        }
        check ++;
         pthread_mutex_lock(&lock2);  // I locked this part because it is poissble to have two threads starting their transmission time at the same time
        gettimeofday(&start2, NULL);
        int a = 0;
        while(queueList[a+1]!= NULL){
            
            queueList[a] = queueList[a+1];//  push the items to the left.
            a++;
        }
        
        counter --;// number of items in the array.
        pthread_mutex_unlock(&trans_mtx);
}  // sorts the multiple threads.
void releasePipe() {
    pthread_cond_broadcast(&trans_cvar);
      pthread_mutex_unlock(&lock2);
    run--;
    
}   //broadcast condition variable and unlock the lock2, change the run value
float timecalculate(float x, int save){
  gettimeofday(&end[save], NULL);
  double arrivalspent = (double) (end[save].tv_usec - start[save].tv_usec) / 1000000 +(double) (end[save].tv_sec - start[save].tv_sec);
  
 return (arrivalspent);
 }  // calculates real machine arrival time(very accurate)
float timecalculate2(float x){ 
    gettimeofday(&end2, NULL);
    double arrivalspent = (double) (end2.tv_usec - start2.tv_usec) / 1000000 +(double) (end2.tv_sec - start2.tv_sec);
    return (arrivalspent);
}   // calculates real machine transmission time(very accurate)
void *thrFunction(void *flowItem) {
    
  flow *item = (flow *)flowItem ;
  /// get the real mahcine time for the flows (arrivaltime )
  float time;
  int saveval; // save the value for the time calculation.
  
  pthread_mutex_lock(&timelock);
  
  gettimeofday(&start[arr], NULL);
  saveval = arr;
  arr++;  
  // other thread can't increment arr while one thread starts counting time.
  pthread_mutex_unlock(&timelock);
  
  
  
  usleep((int)(100000*item->arrivalTime));   // we need to multiply 100,000 to the parameter since it is in micro seconds.
  time = timecalculate(item->arrivalTime,saveval);
  
  
  ////  finishes the time calculation for the arrivaltime 
  pthread_mutex_lock(&timelock);  // Lock the other flows until the first thread starts
  printf("Flow %2d arrives: arrival time (%.2f), transmission time (%.1f), priority (%2d). \n",item->id, time ,item->transTime*0.1,item->priority);
  
 
  requestPipe(item);
  float loadingtime = timecalculate2(item->arrivalTime); 
  //Loading time calculation  Total time spent during (wait -> start)
  run++;
     if(check == 1){
      totaltime = totaltime + time + loadingtime; // if the thread is the first thread
     }
     else{
        totaltime  =  totaltime +loadingtime;  // other wise
     }
  printf("Flow %2d starts its transmission at time %.2f. \n",item->id,totaltime);
  gettimeofday(&start2, NULL);  // start the timer to get the processing time.
  usleep((int)(100000*item->transTime));
  
  time = timecalculate2(item->transTime);  // calculates transmission time.
  
  totaltime = totaltime + time;
  releasePipe(item);
  printf("Flow %2d finishes its transmission at time %.1f. \n",item->id,totaltime);
  
}  // entry point for each thread created
int main(int agrc,char *agrv[]){
     FILE *file;
    char *a = agrv[1];
    file = fopen(a,"r");
    if(file == NULL){
         printf("Error: File does not exist.\n");
      }
    //Success 
    else {
     int i =0;
     int flowlen = fgetc(file);
     
     flowlen = flowlen - 48;// convert html val to char (ascii table)
     if(flowlen <= 0){
         printf("Your input values are not correct.\n");
     }
    
     int val1,val4;
     float val2,val3;
      
     
    while(i != flowlen)
   {
     fscanf(file, "%d:%f,%f,%d", &val1, &val2, &val3, &val4); // Scan the input file.
     flowList[i].id = val1;
     flowList[i].arrivalTime = val2;
     flowList[i].transTime = val3;
     flowList[i].priority = val4;  
     // Store the parse input values to flowlist[] array
      if( feof(file) )
      { 
         break ; // breaks if there's nothing in the input file
      }
     
     i++;
   }// while loop ends
   fclose(file);
   
   for (i=0;i<flowlen;i++){
   pthread_create(&thrList[i], NULL, thrFunction, (void *)&flowList[i]);
   }    //  create threads by calling pthread_cread (number of threads = number of flows)
   
   for (i=0;i<flowlen;i++){
   pthread_join(thrList[i],NULL);   // Main thread would be idle and wait for other threads to terminate.
   }
   pthread_mutex_destroy(&trans_mtx); 
   pthread_mutex_destroy(&lock);   // destory the mutex 
   pthread_cond_destroy(&trans_cvar); 
   pthread_mutex_destroy(&lock2);   // destory the condition variable 
    pthread_mutex_destroy(&timelock);
   
   
    }
    return 0;
}  // main will be idle until other threads join