
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
	//int64_t timeStamp[MAX_TASK_COUNT+1];
	time_t startTime;
	time_t endTime;
} ThreadArgs;


//Globals

//Try to change this to use a single condition variable
pthread_mutex_t g_ThreadMutex [MAX_THREAD_COUNT]; //set thread policy to OTHERdMutex [MAX_THREAD_COUNT];
pthread_cond_t g_conditionVar [MAX_THREAD_COUNT];
ThreadArgs g_ThreadArgs[MAX_THREAD_COUNT];

//========================================================================================================================================================
void InitGlobals(void) //set thread policy to OTHER
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
				g_ThreadArgs[i].threadPri = 1; //Should we set the priority to any nonzero #? or does it have to be specific
				//printf("Inside2\n");
			} else { //Last 3 RR
				g_ThreadArgs[i].threadPolicy = SCHED_RR;
				g_ThreadArgs[i].threadPri = 2;
				//printf("Inside3\n");
			}

			g_ThreadArgs[i].processTime = -1;
			g_ThreadArgs[i].startTime = 0;
			g_ThreadArgs[i].endTime = 0;

		  //g_ThreadMutex[i] = ;??????????
			//g_conditionVar[i] = ;?????????
		}
// Initialize all globals

}

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
/*void DisplayThreadArgs(ThreadArgs*	myThreadArg)
{
int i,y;

if( myThreadArg )
{
	DisplayThreadSchdAttributes(myThreadArg->threadId, myThreadArg->threadPolicy, myThreadArg->threadPri);
	printf(" startTime = %s endTime = %s", ctime(&myThreadArg->startTime), ctime(&myThreadArg->endTime));
	printf(" TimeStamp [%"PRId64"]\n", myThreadArg->timeStamp[0] );

	for(y=1; y<MAX_TASK_COUNT+1; y++)
	{
		printf(" TimeStamp [%"PRId64"] Delta [%"PRId64"]us\n", myThreadArg->timeStamp[y],
		(myThreadArg->timeStamp[y]-myThreadArg->timeStamp[y-1]));
	}
}
*/
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
	ThreadArgs*	myThreadArg; //technique from below
	myThreadArg = (ThreadArgs*)arg;
	printf("Thread created for thread number %d\n", myThreadArg->threadCount);
	printf("myThreadArg->threadPri = %d, myThreadArg->threadPolicy = %d, Threadid = %d\n\n", myThreadArg->threadPri, myThreadArg->threadPolicy, pthread_self());


	//2.	Use the �pthread_setscheduleparam� API to set the thread policy
	int threadSchedParam;
	struct sched_param param;// creates param structure for priority

	if (myThreadArg->threadPolicy == SCHED_OTHER) { //set thread policy to OTHER
		//printf("Inside SCHED_OTHER Function!\n");
		param.sched_priority = myThreadArg->threadPri; //set the priority to 0
		threadSchedParam = pthread_setschedparam(pthread_self(), myThreadArg->threadPolicy, &param);
			if (threadSchedParam) {
				printf("SCHED_OTHER NOT WORKING!\n");
				//handle_error_en(threadSchedParam, "pthread_setschedparam");
			} else {
				DisplayThreadSchdAttributes(pthread_self(), myThreadArg->threadPolicy, param.sched_priority);

			}
	} else if (myThreadArg->threadPolicy == SCHED_FIFO) {
		//printf("Inside SCHED_FIFO Function!\n");
		param.sched_priority = myThreadArg->threadPri; //set the priority to 1;
		threadSchedParam = pthread_setschedparam(pthread_self(), myThreadArg->threadPolicy, &param); //set thread policy to FIFO
			if (threadSchedParam) {
				printf("SCHED_FIFO NOT WORKING!\n");
				//handle_error_en(threadSchedParam, "pthread_setschedparam");
			} else {
				DisplayThreadSchdAttributes(pthread_self(), myThreadArg->threadPolicy, param.sched_priority);
			}
	} else {
		//printf("Inside SCHED_RR Function!\n");
		param.sched_priority = myThreadArg->threadPri; //set the priority to 2;
		threadSchedParam = pthread_setschedparam(pthread_self(), myThreadArg->threadPolicy, &param);  //set thread policy to RR
		if (threadSchedParam) {
			printf("SCHED_RR NOT WORKING!\n");
			//handle_error_en(threadSchedParam, "pthread_setschedparam");
		} else {
			DisplayThreadSchdAttributes(pthread_self(), myThreadArg->threadPolicy, param.sched_priority);
		}
	}
	//3.	Init the Condition variable and associated mutex
	//if (pthread_cond_init(&g_conditionVar[????], NULL?)) {
		//printf("failed to initialize cond\n");
//	}
	//pthread_mutex_lock(&ThreadMutex[????]);
	//4.	Wait on condition variable
	//5.	Once condition variable is signaled, use the �time� function and the �clock_gettime(CLOCK_REALTIME, &tms)� to get timestamp
	//6.	Call �DoProcess� to run your task
	//7.	Use �time� and �clock_gettime� to find end time.
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
			printf("i = %d\n", i);
			if (pthread_attr_init(&threadAttrib) != 0) {
				printf("Error initalizing threads attribute object\n");
			}
			if (pthread_create(&g_ThreadArgs[i].threadId, &threadAttrib, &threadFunction, &g_ThreadArgs[i]) != 0) {
				printf("pthread_create failed for thread number %d : %d\n", i);
			}
			else
			{
				//printf("Inside else!");
				g_ThreadArgs[i].threadCount = i;

			}
			if (pthread_attr_destroy(&threadAttrib) != 0) {
				printf("Error destroying threads attribute object\n");
			}
			//break;
		}
		getchar();
		return 0;
	}
	//3.	Assign 3 threads to SCHED_OTHER, another 3 to SCHED_FIFO and another 3 to SCHED_RR //how come it says to assign the policy here and in thread function?
	//4.	Signal the condition variable
	//5.	Call �pthread_join� to wait on the thread
	//6.	Display the stats on the threads





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
