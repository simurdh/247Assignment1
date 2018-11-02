
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
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)


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
	printf("initGlobals\n");
		for(int i=0;i<MAX_THREAD_COUNT; i++)
		{
			g_ThreadArgs[i].threadCount = -1;
			g_ThreadArgs[i].threadId = "noThreadId";
			if (i <= 2) { //First 3 OTHER
				g_ThreadArgs[i].threadPolicy = SCHED_OTHER;
				g_ThreadArgs[i].threadPri = 0;
				//printf("Inside1\n");
			} else if (i <= 5) { //Next 3 FIFO
				g_ThreadArgs[i].threadPolicy = SCHED_FIFO;
				g_ThreadArgs[i].threadPri = i; //Should we set the priority to any nonzero #? or does it have to be specific
				//printf("Inside2\n");
			} else { //Last 3 RR
				g_ThreadArgs[i].threadPolicy = SCHED_RR;
				g_ThreadArgs[i].threadPri = i;
				//printf("Inside3\n");
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
  printf("\nstartTime = %s", ctime(&myThreadArg->startTime));
  printf("endTime = %s", ctime(&myThreadArg->endTime));

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

	//2.	Use the �pthread_setscheduleparam� API to set the thread policy
	int threadSchedParam;
	struct sched_param param;// creates param structure for priority

	if (myThreadArg->threadPolicy == SCHED_OTHER) { //set thread policy to OTHER
		param.sched_priority = myThreadArg->threadPri; //set the priority to 0
		threadSchedParam = pthread_setschedparam(pthread_self(), myThreadArg->threadPolicy, &param);
			if (threadSchedParam) {
				handle_error_en(threadSchedParam, "pthread_setschedparam");
			} else {
      //print error
			}
	} else if (myThreadArg->threadPolicy == SCHED_FIFO) {
		param.sched_priority = myThreadArg->threadPri; //set the priority to 1;
		threadSchedParam = pthread_setschedparam(pthread_self(), myThreadArg->threadPolicy, &param); //set thread policy to FIFO
			if (threadSchedParam) {
				printf("SCHED_FIFO NOT WORKING!\n");
				handle_error_en(threadSchedParam, "pthread_setschedparam");
			} else {
        //printf("Thread created for thread number %d\n", myThreadArg->threadCount);
			}
	} else {
		param.sched_priority = myThreadArg->threadPri; //set the priority to 2;
		threadSchedParam = pthread_setschedparam(pthread_self(), myThreadArg->threadPolicy, &param);  //set thread policy to RR
		if (threadSchedParam) {
			printf("SCHED_RR NOT WORKING!\n");
			handle_error_en(threadSchedParam, "pthread_setschedparam");
		} else {
      //printf("Thread created for thread number %d\n", myThreadArg->threadCount);
		}
	}

  //Lock mutex
if (pthread_mutex_lock(&g_ThreadMutex)) {
    printf("mutex lock failed\n");
    //handle_error_en(threadSchedParam, "pthread_mutex_lock");
  } else {
  printf("mutex locked successfully for thread: %d\n", myThreadArg->threadCount);
  }
condition2++;
if (pthread_cond_signal(&g_conditionVar2)) { //unblocks threads currently blocked on g_conditionVar
  printf("error using pthread_cond_signal\n");
  //handle_error_en(threadSchedParam, "pthread_cond_broadcast");
} else {
  printf("first pthread broadcast\n");
}

while (!condition){
  if (pthread_cond_wait(&g_conditionVar, &g_ThreadMutex)) {
    printf("pthread_cond_wait failed\n");
    //handle_error_en(threadSchedParam, "pthread_mutex_lock");
  } else {
    printf("\npthread cond wait successful for thread: %d\n", myThreadArg->threadCount);
  }
}
if (pthread_mutex_unlock(&g_ThreadMutex)) {
  printf("mutex unlock failed\n");
  //handle_error_en(threadSchedParam, "pthread_mutex_lock");
} else {
  printf("\nmutex unlocked for thread: %d\n\n", myThreadArg->threadCount);
}
	//4.	Wait on condition variable
	//5.	Once condition variable is signaled, use the �time� function and the �clock_gettime(CLOCK_REALTIME, &tms)� to get timestamp
	myThreadArg->startTime = time(&myThreadArg->startTime);

  struct timespec tms;
  clock_gettime(CLOCK_REALTIME, &tms);
  myThreadArg->timeStamp[0] = tms.tv_sec *1000000;
  myThreadArg->timeStamp[0] += tms.tv_nsec/1000;
  if(tms.tv_nsec % 1000 >= 500 ) myThreadArg->timeStamp[0]++;

	//6.	Call �DoProcess� to run your task
  for(int i = 1; i <= MAX_TASK_COUNT; i++) {
    DoProcess();
  //7.	Use �time� and �clock_gettime� to find end time.
    clock_gettime(CLOCK_REALTIME, &tms);
    myThreadArg->timeStamp[i] = tms.tv_sec *1000000;
  	myThreadArg->timeStamp[i] += tms.tv_nsec/1000;
  	if(tms.tv_nsec % 1000 >= 500 ) myThreadArg->timeStamp[i]++;
  }
  myThreadArg->endTime = time(&myThreadArg->endTime);

	//8.	You can repeat steps 6 and 7 a few times if you wish
}

//========================================================================================================================================================
int main (int argc, char *argv[])
{
	//1.	Call InitGlobals
	InitGlobals();
	//2.	Create a number of threads (start with 1 and increase to 9) using �pthread_Create�
		pthread_attr_t threadAttrib;
		for (int i = 0; i < MAX_THREAD_COUNT; i++) {
			if (pthread_attr_init(&threadAttrib) != 0) {
				printf("Error initalizing threads attribute object\n");
			}
			if (pthread_create(&g_ThreadArgs[i].threadId, &threadAttrib, &threadFunction, &g_ThreadArgs[i]) != 0) {
				printf("pthread_create failed for thread number %d : %d\n", i); //need to fix
			}
			else
			{
				//printf("Inside else!");
				g_ThreadArgs[i].threadCount = i;

			}
			if (pthread_attr_destroy(&threadAttrib) != 0) {
				printf("Error destroying threads attribute object\n");
			}
    }
	//4.	Signal the condition variable

  if (pthread_mutex_lock(&g_ThreadMutex2)) {
      printf("mutex lock failed\n");
      //handle_error_en(threadSchedParam, "pthread_mutex_lock");
    } else {
    printf("mutex locked successfully for MAIN\n");
    }
  //condition++;
  //printf("thread %d incremented condition var to: %d\n", myThreadArg->threadCount, condition);
  while (condition2 < 9) {
    if (pthread_cond_wait(&g_conditionVar2, &g_ThreadMutex2)) {
      printf("pthread_cond_wait failed\n");
      //handle_error_en(threadSchedParam, "pthread_mutex_lock");
    } else {
      printf("pthread cond wait successful for MAIN\n");
    }
  }
  if (pthread_mutex_unlock(&g_ThreadMutex2)) {
    printf("mutex unlock failed\n");
    //handle_error_en(threadSchedParam, "pthread_mutex_lock");
  } else {
    printf("mutex unlocked for MAIN\n");
  }

  pthread_mutex_lock(&g_ThreadMutex);
  condition++;
  printf("condition var = %d\n", condition);
  if (pthread_cond_broadcast(&g_conditionVar)) { //unblocks threads currently blocked on g_conditionVar
    printf("error using pthread_cond_broadcast\n");
    //handle_error_en(threadSchedParam, "pthread_cond_broadcast");
  } else {
    printf("PTHREAD BROADCAST\n");
  }
  pthread_mutex_unlock(&g_ThreadMutex);

  //getchar();
	//5.	Call �pthread_join� to wait on the thread
	//6.	Display the stats on the threads
  int var;
  for (int j = 0; j < MAX_THREAD_COUNT; j++) {
    //printf("waiting for thread: %d\n", g_ThreadArgs[j].threadCount);
    var = pthread_join(g_ThreadArgs[j].threadId, NULL);
    if (var) {
      printf("pthread_join error\n");
    }
    DisplayThreadArgs(&g_ThreadArgs[j]);
  }
  return 0;
}



//========================================================================================================================================================



/*

************* HINTS ******************

========================================================================================================================================================
Every time you run into issues with usage of an API, please look up samples on how that API is used here...

http://www.yolinux.com/TUTORIALS/LinuxTutorialPosixThreads.html

========================================================================================================================================================


Please check the return values from all system calls and print an error message in all error cases including the error code.. That will help catch errors quickly.
========================================================================================================================================================


You can use the following technique to pass the address of the element corresponding to a particular thread to the thread function...

	void* threadFunction(void *arg)
	{
		ThreadArgs*	myThreadArg;

		myThreadArg = (ThreadArgs*)arg;

	}


	int main (int argc, char *argv[])
	{

		while(threadCount < MAX_THREAD_COUNT)
		{
		...
			if( pthread_create(&(g_ThreadArgs[threadCount].threadId), &threadAttrib, &threadFunction, &g_ThreadArgs[threadCount]) )
		...

		}
	}
========================================================================================================================================================

Here is the usage for clock_gettime�

	clock_gettime(CLOCK_REALTIME, &tms);
	myThreadArg->timeStamp[y+1] = tms.tv_sec *1000000;
	myThreadArg->timeStamp[y+1] += tms.tv_nsec/1000;
	if(tms.tv_nsec % 1000 >= 500 ) myThreadArg->timeStamp[y+1]++;

========================================================================================================================================================

Here is how you wait on a condition event�

	pthread_mutex_lock ( &g_ThreadMutex[myThreadArg->threadCount] );
	pthread_cond_wait ( &g_conditionVar[myThreadArg->threadCount], &g_ThreadMutex[myThreadArg->threadCount] );
	pthread_mutex_unlock( &g_ThreadMutex[myThreadArg->threadCount] );

========================================================================================================================================================

Note that this sample is changing the policy of the current thread... so if you follow this sample, make sure you are making the call from the thread function.


	http://man7.org/linux/man-pages/man3/pthread_setschedparam.3.html

	if (main_sched_str != NULL) {
	if (!get_policy(main_sched_str[0], &policy))
		usage(argv[0], "Bad policy for main thread (-m)\n");
		param.sched_priority = strtol(&main_sched_str[1], NULL, 0);

	s = pthread_setschedparam(pthread_self(), policy, &param);
	if (s != 0)
		handle_error_en(s, "pthread_setschedparam");
	}

========================================================================================================================================================
For those confused about my comment on trying to using a single Condition variable instead of an array... please read the following...

http://pubs.opengroup.org/onlinepubs/9699919799/functions/pthread_cond_signal.html

You can use the broadcast API to wake multiple threads waiting on the same condition variable.

For those who really like to go deeper, know that you have access to the code for most of the Linux system APIs... here is the code pthread_cond_broadcast...

https://code.woboq.org/userspace/glibc/nptl/pthread_cond_broadcast.c.html

========================================================================================================================================================
*/
