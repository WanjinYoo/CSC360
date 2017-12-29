V00823396
WanjinYoo
CSC360 (Assignment2)


The program only runs on certain input file format. Taking flow number,arrivaltime,transmission time
and priority as input and simulates a series of flows.
I sorted multiple threads using mutex,condition variable and p_thread.
In order to run this program, type "make -f Makefile" on the command line to compile this program, and
then you will be able to run this program by typing "./MFS input.txt"(given inputfile).

My code consists of 6 major methods and global timer.

void requestPipe(flow *item)
sorts the multiple threads and requests a next pipe.

void releasePipe()
broadcast condition variable and unlock the lock2, change the run value.

float timecalculate(float x, int save)
calculates real machine arrival time(very accurate).

float timecalculate2(float x)
calculates real machine transmission time(very accurate).

void *thrFunction(void *flowItem)
entry point for each thread create.

int main(int agrc,char *agrv[])
 main will be idle until other threads join.