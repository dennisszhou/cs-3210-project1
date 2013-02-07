
/* Includes */
#include <unistd.h>     /* Symbolic Constants */
#include <sys/types.h>  /* Primitive System Data Types */ 
#include <errno.h>      /* Errors */
#include <stdio.h>      /* Input/Output */
#include <stdlib.h>     /* General Utilities */
#include <pthread.h>    /* POSIX Threads */
#include <string.h>     /* String handling */

#define FILE_PATH "/proc/lfprng"

/* prototype for thread routine */
void print_message_function ( void *ptr );

/* struct to hold data to be passed to a thread
   this shows how multiple data items can be passed to a thread */
typedef struct str_thdata
{
    int thread_no;
    char message[100];
} thdata;

int main()
{
    pthread_t thread1, thread2, thread3;  /* thread variables */
    thdata data1, data2, data3;         /* structs to be passed to threads */
    
    /* initialize data to pass to thread 1 */
    data1.thread_no = 1;
    strcpy(data1.message, "Hello!");

    /* initialize data to pass to thread 2 */
    data2.thread_no = 2;
    strcpy(data2.message, "Hi!");

	data3.thread_no = 3;
    strcpy(data3.message, "Hello!");

	FILE *fp;
	fp = fopen(FILE_PATH, "w");
	fprintf(fp, "%d 3", 1);
	fclose(fp);
    
    /* create threads 1 and 2 */    
    pthread_create (&thread1, NULL, (void *) &print_message_function, (void *) &data1);
    pthread_create (&thread2, NULL, (void *) &print_message_function, (void *) &data2);
	pthread_create (&thread3, NULL, (void *) &print_message_function, (void *) &data3);

    /* Main block now waits for both threads to terminate, before it exits
       If main block exits, both threads exit, even if the threads have not
       finished their work */ 
    pthread_join(thread1, NULL);
    pthread_join(thread2, NULL);
	pthread_join(thread3, NULL);
              
    /* exit */  
    exit(0);
} /* main() */

/**
 * print_message_function is used as the start routine for the threads used
 * it accepts a void pointer 
**/
void print_message_function ( void *ptr )
{
	unsigned long long myrand;
	int i;
    thdata *data;
    data = (thdata *) ptr;  /* type cast to a pointer to thdata */

	FILE *fp;
	for(i = 0; i < 3; i++) {
		fp = fopen(FILE_PATH, "r");
		fscanf(fp, "%llu", &myrand);
		fclose(fp);
    /* do the work */
    printf("Thread %d says %s for %llu\n", data->thread_no, data->message, myrand);
	}

    pthread_exit(0); /* exit */
} /* print_message_function ( void *ptr ) */
