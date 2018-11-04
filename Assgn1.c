 /* Hannah Simurdak Assignment 1

 Analysis

The scheduler decides which thread to execute next based on its thread policy and priority. When I call pthread_cond_broadcast in the main, the threads unblock based on their policies. Only one thread can use the CPU at a time, however in multicore systems multiple threads can use different CPUs at the same time.
This may explain why the timestamps do not always reflect the expected latencies of the threads.
First of all, in the FIFO threads there is no time slicing and the first thread to start runs to completion. This is why the delta (change) in the timestamps are fairly uniform in FIFO threads because each one starts and finishes without interuption in this program.
There is less latency because the threads do not switch as often.
For the RR threads, each thread runs until it reaches its time quantum, then the next thread begins running. This explains why there is a larger difference in the delta of the timestamps and more latency.
For OTHER threads, they are supposed to progress fairly which is achieved through a nice value and dynamic prority. Also, it can be prempted by FIFO and RR threads. Therefore the latency between these threads can vary.
*/

//========================================================================================================================================================

#include <time.h>
#include <stdio.h>
#include <string.h>
#include <stdbool.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/time.h>
#include <inttypes.h>
#include <errno.h>

//========================================================================================================================================================
//#defines

#define MAX_THREAD_COUNT 9
#define MAX_TASK_COUNT 3
#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); } while (0)


//========================================================================================================================================================
//Typedefs
typedef struct{
	int threadCount;
	pthread_t threadId;
	int threadPolicy;
	int threadPri;
	long processTime;
	int64_t timeStamp[MAX_TASK_COUNT+1];
	time_t startTime;
	time_t endTime;
} ThreadArgs;


//Globals


pthread_mutex_t g_ThreadMutex = PTHREAD_MUTEX_INITIALIZER; //initializes a mutex with default attributes using a macro
pthread_cond_t g_conditionVar = PTHREAD_COND_INITIALIZER; //initializes a condition variable with default attributes using a macro
ThreadArgs g_ThreadArgs[MAX_THREAD_COUNT];

int condition = 0; //used to have threads wait on cond
int condition2= 0; //used to have main wait until all threads have locked

pthread_mutex_t g_ThreadMutex2 = PTHREAD_MUTEX_INITIALIZER;
pthread_cond_t g_conditionVar2 = PTHREAD_COND_INITIALIZER;



//========================================================================================================================================================
void InitGlobals(void)
{
		for(int i=0;i<MAX_THREAD_COUNT; i++)
		{
			g_ThreadArgs[i].threadCount = -1;
			g_ThreadArgs[i].threadId = "0";
			if (i <= 2) { //First 3 OTHER
				g_ThreadArgs[i].threadPolicy = SCHED_OTHER;
				g_ThreadArgs[i].threadPri = 0;
			} else if (i <= 5) { //Next 3 FIFO
				g_ThreadArgs[i].threadPolicy = SCHED_FIFO;
				g_ThreadArgs[i].threadPri = i;
			} else { //Last 3 RR
				g_ThreadArgs[i].threadPolicy = SCHED_RR;
				g_ThreadArgs[i].threadPri = i;
			}

			g_ThreadArgs[i].processTime = -1;
			g_ThreadArgs[i].startTime = 0;
			g_ThreadArgs[i].endTime = 0;

		}
}
////////////////////////////////////////////////////////////////////////////////

//========================================================================================================================================================
void DisplayThreadSchdAttributes( pthread_t threadID, int policy, int priority )
{

	printf("\nDisplayThreadSchdAttributes:\n threadID = 0x%lx\n policy = %s\n priority = %d\n",
									threadID,
									(policy == SCHED_FIFO) ? "SCHED_FIFO" :
									(policy == SCHED_RR)	? "SCHED_RR" :
									(policy == SCHED_OTHER) ? "SCHED_OTHER" :
									"???",
									priority);
}

//========================================================================================================================================================
void DisplayThreadArgs(ThreadArgs*	myThreadArg)
{
int i,y;

if( myThreadArg )
{
  //Added print statements to fix bug
  //printf("\nstartTime = %s", ctime(&myThreadArg->startTime));
  //printf("endTime = %s", ctime(&myThreadArg->endTime));

	DisplayThreadSchdAttributes(myThreadArg->threadId, myThreadArg->threadPolicy, myThreadArg->threadPri);
	printf(" startTime = %s endTime = %s", ctime(&myThreadArg->startTime), ctime(&myThreadArg->endTime));
	printf(" TimeStamp [%"PRId64"]\n", myThreadArg->timeStamp[0] );

	for(y=1; y<MAX_TASK_COUNT+1; y++)
	{
		printf(" TimeStamp [%"PRId64"] Delta [%"PRId64"]us\n", myThreadArg->timeStamp[y],
		(myThreadArg->timeStamp[y]-myThreadArg->timeStamp[y-1]));
	}
}
}

//========================================================================================================================================================
void DoProcess(void)
{
	unsigned int longVar =1 ;

	while(longVar < 0xffffffff) longVar++;
}

//========================================================================================================================================================
void* threadFunction(void *arg)
{
	//1.	Typecast the argument to a �ThreadArgs*� variable
	ThreadArgs*	myThreadArg;
	myThreadArg = (ThreadArgs*)arg;

	int threadSchedParam;
	struct sched_param param;// creates param structure for priority

	if (myThreadArg->threadPolicy == SCHED_OTHER) { //set thread policy to OTHER
		param.sched_priority = myThreadArg->threadPri; //set the priority to 0
		threadSchedParam = pthread_setschedparam(pthread_self(), myThreadArg->threadPolicy, &param);
			if (threadSchedParam) {
        handle_error_en(threadSchedParam, "pthread_setschedparam");
			}
	} else if (myThreadArg->threadPolicy == SCHED_FIFO) {
		param.sched_priority = myThreadArg->threadPri;
		threadSchedParam = pthread_setschedparam(pthread_self(), myThreadArg->threadPolicy, &param); //set thread policy to FIFO
			if (threadSchedParam) {
        handle_error_en(threadSchedParam, "pthread_setschedparam");
			}
	} else {
		param.sched_priority = myThreadArg->threadPri;
		threadSchedParam = pthread_setschedparam(pthread_self(), myThreadArg->threadPolicy, &param);  //set thread policy to RR
		if (threadSchedParam) {
      handle_error_en(threadSchedParam, "pthread_setschedparam");
		}
	}
  //Lock mutex
int i = pthread_mutex_lock(&g_ThreadMutex);
if (i) {
  handle_error_en(i, "pthread_mutex_lock");
  } else {
  //printf("mutex locked successfully for thread: %d\n", myThreadArg->threadCount);
  }
condition2++; //update condition2 because mutex is locked
if (condition2 == 9) { //if all threads have locked
  i = pthread_cond_signal(&g_conditionVar2); //unblocks main thread blocked on g_conditionVar2
  if (i) {
    handle_error_en(i, "pthread_mutex_lock");
  } else {
    //printf("pthread signal\n");
  }
}

while (!condition) {
  i = pthread_cond_wait(&g_conditionVar, &g_ThreadMutex);
  if (i) {
    handle_error_en(i, "pthread_mutex_lock");
  }
}
i = pthread_mutex_unlock(&g_ThreadMutex);
if (i) {
  handle_error_en(i, "pthread_mutex_lock");
} else {
  //printf("\nmutex unlocked for thread: %d\n\n", myThreadArg->threadCount);
}
	//5.	Once condition variable is signaled, use the �time� function and the �clock_gettime(CLOCK_REALTIME, &tms)� to get timestamp
	myThreadArg->startTime = time(&myThreadArg->startTime);

  struct timespec tms;
  clock_gettime(CLOCK_REALTIME, &tms);
  myThreadArg->timeStamp[0] = tms.tv_sec *1000000;
  myThreadArg->timeStamp[0] += tms.tv_nsec/1000;
  if(tms.tv_nsec % 1000 >= 500 ) myThreadArg->timeStamp[0]++;

	//6.	Call �DoProcess� to run your task
  for(int k = 1; k <= MAX_TASK_COUNT; k++) {
    DoProcess();
  //7.	Use �time� and �clock_gettime� to find end time.
    clock_gettime(CLOCK_REALTIME, &tms);
    myThreadArg->timeStamp[k] = tms.tv_sec *1000000;
  	myThreadArg->timeStamp[k] += tms.tv_nsec/1000;
  	if(tms.tv_nsec % 1000 >= 500 ) myThreadArg->timeStamp[k]++;
  }
  myThreadArg->endTime = time(&myThreadArg->endTime);
}

//========================================================================================================================================================
int main (int argc, char *argv[])
{
	//1.	Call InitGlobals
	InitGlobals();
  int j; //use to get error code
	//2.	Create a number of threads (start with 1 and increase to 9) using �pthread_Create�
		pthread_attr_t threadAttrib;
		for (int i = 0; i < MAX_THREAD_COUNT; i++) {
      j = pthread_attr_init(&threadAttrib);
			if (j) {
        handle_error_en(i, "pthread_attr_t");
			}
      j = pthread_create(&g_ThreadArgs[i].threadId, &threadAttrib, &threadFunction, &g_ThreadArgs[i]);
			if (j) {
        handle_error_en(i, "pthread_create");
			}
			else
			{
				g_ThreadArgs[i].threadCount = i;
			}
      j = pthread_attr_destroy(&threadAttrib);
			if (j) {
        handle_error_en(i, "pthread_attr_destroy");
			}
    }

  j = pthread_mutex_lock(&g_ThreadMutex2);
  if (j) {
    handle_error_en(j, "pthread_mutex_lock");
  }
  while (condition2 < 9) { //Wait for threads to lock
    j = pthread_cond_wait(&g_conditionVar2, &g_ThreadMutex2);
    if (j) {
      handle_error_en(j, "pthread_cond_wait");
    }
  }
  j = pthread_mutex_unlock(&g_ThreadMutex2);
  if (j) {
    handle_error_en(j, "pthread_mutex_unlock");
  } else {
    //printf("mutex unlocked for MAIN\n");
  }

  pthread_mutex_lock(&g_ThreadMutex);
  condition++;
  j = pthread_cond_broadcast(&g_conditionVar);
  if (j) {
    handle_error_en(j, "pthread_cond_broadcast"); //unblocks threads currently blocked on g_conditionVar
  } else {
    //printf("PTHREAD BROADCAST\n");
  }
  pthread_mutex_unlock(&g_ThreadMutex);

	//5.	Call �pthread_join� to wait on the thread
	//6.	Display the stats on the threads
  for (int k = 0; k < MAX_THREAD_COUNT; k++) {
    j = pthread_join(g_ThreadArgs[k].threadId, NULL);
    if (j) {
      handle_error_en(j, "pthread_join");
    }
    DisplayThreadArgs(&g_ThreadArgs[k]);
  }
  return 0;
}
