V00832396
WanjinYoo
CSC360 (Assignment1)

rsi.c

int main() 
 
 get input from the User by calling readline().
 
 takes input characters and tokenize it and creates an array that contains tokenized characters.
 
 
 
 if(strcmp(array[0], "pstat")==0) : takes pid value from the array and convert pid value into int value.
 
 prints following information related to the pid using switch,case,file...etc
 
 if(strcmp(bg, "bg")==0) : creates a new item in proc_list using malloc().
 
 execute following input using execvp()
 
 store the concatenated characters from the array into proc_list.
 
 update background process.
 
 if(strcmp(bg, "bglist")==0) : creates a background list using for loop.
 
 if(strcmp(bg, "bgkill")==0): kills the process with following pid using for loop and the command from the proc list.
 
 if(strcmp(bg, "bgstart")==0): resume the process with following pid using for loop and the command from the proc list.
 
 if(strcmp(bg, "bgstop")==0): stops the process with following pid using for loop and the command from the proc list.
 
 void update_bg_procss(): checks if there are any process running using waitpid(-1, &status, WNOHANG)
 
 if it returns 0, it is already terminated. just need to break the loop.
 
 if it returns >0, I need to remove it from the bglist, and inform the user that the process is terminated.
  
 I also need to remove items from the proclist
 
 if it returns <0, just need to break the loop.
 
 
 Makefile:
 
 it compiles multiple source files that are listed in the file.
 
 Use this file by calling make -f Makefile in the command line.
 
 inf.c
 
 included this file because it is easy to check whether it is running or not.
 
 main(int argc, char* argv[]) : prints tag with certain interval.
 
 Tag and interval would be created by a user.
 
 
 
 
 
 
