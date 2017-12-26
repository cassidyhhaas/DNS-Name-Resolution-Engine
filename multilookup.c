// Cassidy Haas
// CSCI 3753 PA 3: DNS Name Resolution Engine
// Due 11/02/17


//header files:
#include <stdlib.h> //for exit
#include <stdio.h> //for fprintf()
#include <string.h>
#include <errno.h>
#include <pthread.h> //for mutex and thread operations
#include <unistd.h>
#include <sys/time.h> //for gettimeofday()

#include "multilookup.h"
#include "util.h" //dns lookup functionality

int totalInputFiles;
int inputFileCounter;

int filePointer;
int endOfFiles;
int checkPointer;
int filesLeft;
char* servicedFile;

void* Requester(void* tid) 
{
	struct threadStruct* thread = tid;
	char hostname[MAX_NAME_LENGTH];
	char* allocSpace;
	bool fileComplete=false;
	pthread_mutex_t* hostnameQueue_mutex = thread->hostnameQueue_mutex;
	pthread_mutex_t* service_mutex = thread->service_mutex;

	queue* hostnameQueue = thread->hostnameQueue;
	FILE* fp;
	FILE* inFilePointer;

	int thread_id = pthread_self();
	int numberservicedFile = 0;


	while (filePointer<endOfFiles)
	{
		inFilePointer = thread->fileArray[filePointer-5];
		while(fscanf(inFilePointer, INPUT_FILE_SIZE, hostname)>0)
		{
			while(fileComplete==false)
			{
				allocSpace = malloc(MAX_NAME_LENGTH);
				strncpy(allocSpace, hostname, MAX_NAME_LENGTH);
				pthread_mutex_lock(hostnameQueue_mutex); 
				while(queue_is_full(hostnameQueue)) 
				{
					pthread_mutex_unlock(hostnameQueue_mutex);
					usleep(rand() % 100000 + 10000); 
					pthread_mutex_lock(hostnameQueue_mutex);
				}
				queue_push(hostnameQueue, allocSpace); //format: (queue, source)
				pthread_mutex_unlock(hostnameQueue_mutex);
				fileComplete=true;
			}
			fileComplete = false;
		}
		numberservicedFile++;
		filePointer++;
		checkPointer++;
	}
	pthread_mutex_lock(service_mutex);
	if ((fp = fopen(servicedFile, "a")))
	{
		if(checkPointer<endOfFiles)
		{
			fprintf(fp, "Thread %d serviced %d files \n", thread_id, numberservicedFile);
		}
		else
		{
			fprintf(fp, "Thread %d serviced 0 files \n", thread_id);
		}
	}
	else
	{
		printf("The serviced file was not found. Please check to make sure it exists in your directory.\n");
	}
	fclose(fp);
	pthread_mutex_unlock(service_mutex);
	return NULL;
}

void* Resolver(void* tid)
{
	struct threadStruct* thread = tid;
	char* allocSpace;
	FILE* resultsFilePointer = thread->threadFile;
	pthread_mutex_t* hostnameQueue_mutex = thread->hostnameQueue_mutex;
	pthread_mutex_t* output_mutex = thread->output_mutex;
	queue* hostnameQueue = thread->hostnameQueue;
	char ip_address[MAX_IP_LENGTH];

	while(!queue_is_empty(hostnameQueue)||does_requester_exist)
	{
		if(!queue_is_empty(hostnameQueue))
		{
			pthread_mutex_lock(hostnameQueue_mutex);
			allocSpace = queue_pop(hostnameQueue);
			if (allocSpace == NULL) 
			{
				pthread_mutex_unlock(hostnameQueue_mutex);
				usleep(rand() % 100000 + 10000);
			}
			else
			{
				pthread_mutex_unlock(hostnameQueue_mutex);
				if(dnslookup(allocSpace, ip_address, sizeof(ip_address))==UTIL_FAILURE)
				{
					strncpy(ip_address, "", sizeof(ip_address));

				}
					printf("%s:%s\n", allocSpace, ip_address);
					pthread_mutex_lock(output_mutex);
					fprintf(resultsFilePointer, "%s,%s\n", allocSpace, ip_address);
					pthread_mutex_unlock(output_mutex);
				free(allocSpace);
			}
		}
	}
	return NULL;
};

/*
 *ADAPTED FROM:
 * File: queue.c
 * Author: Chris Wailes <chris.wailes@gmail.com>
 * Author: Wei-Te Chen <weite.chen@colorado.edu>
 * Author: Andy Sayler <andy.sayler@gmail.com>
 * Project: CSCI 3753 Programming Assignment 2
 * Create Date: 2010/02/12
 * Modify Date: 2011/02/04
 * Modify Date: 2012/02/01
 * Description:
 * 	This file contains an implementation of a simple FIFO queue.
 *  
 */

int queue_init(queue* q, int size){
    
    int i;

    /* user specified size or default */
    if(size>0) {
	   q->maxSize = size;
    }
    else {
	   q->maxSize = QUEUEMAXSIZE;
    }

    /* malloc array */
    q->array = malloc(sizeof(queue_node) * (q->maxSize));
        if(!(q->array)){	
    	perror("Error on queue Malloc");
    	return QUEUE_FAILURE;
    }

    /* Set to NULL */
    for(i=0; i < q->maxSize; ++i){
	   q->array[i].payload = NULL;
    }

    /* setup circular hostnameQueue values */
    q->front = 0;
    q->rear = 0;

    return q->maxSize;
}


int queue_is_empty(queue* q){
    if((q->front == q->rear) && (q->array[q->front].payload == NULL)){
	   return 1;
    }
    else{
	   return 0;
    }
}

int queue_is_full(queue* q){
    if((q->front == q->rear) && (q->array[q->front].payload != NULL)){
	   return 1;
    }
    else{
	   return 0;
    }
}

void* queue_pop(queue* q){
    void* ret_payload;
	
    if(queue_is_empty(q)){
	   return NULL;
    }
	
    ret_payload = q->array[q->front].payload;
    q->array[q->front].payload = NULL;
    q->front = ((q->front + 1) % q->maxSize);

    return ret_payload;
}

int queue_push(queue* q, void* new_payload){
    
    if(queue_is_full(q)){
	   return QUEUE_FAILURE;
    }

    q->array[q->rear].payload = new_payload;

    q->rear = ((q->rear+1) % q->maxSize);

    return QUEUE_SUCCESS;
}

void queue_cleanup(queue* q)
{
    while(!queue_is_empty(q)){
	   queue_pop(q);
    }

    free(q->array);
}


int main(int argc, char* argv[])
{
	struct timeval begin, end;
	gettimeofday(&begin, NULL);
	queue hostnameQueue; 
	int requester_threads=atoi(argv[1]);
	FILE* resultsFilePointer = NULL;
	FILE* inputFilePointer[requester_threads];
	int resolver_threads = atoi(argv[2]);

	servicedFile = argv[4]; 
	pthread_t input_requests[requester_threads]; 
	pthread_t input_resolvers[resolver_threads]; 
	pthread_mutex_t hostnameQueue_mutex;
	pthread_mutex_t output_mutex;
	pthread_mutex_t service_mutex;

	struct threadStruct requester_information; 
	struct threadStruct resolver_information[resolver_threads]; 
	
	totalInputFiles=argc-5;
	inputFileCounter = 0;
	
	filePointer = 5;
	endOfFiles = argc;
	checkPointer = 0;
	filesLeft = totalInputFiles;

	printf("This is the number of requester threads: %d\n", requester_threads);
	printf("This is the number of resolver threads: %d\n", resolver_threads);


	resultsFilePointer = fopen(argv[3], "w");
	if (!resultsFilePointer)
	{
		perror("Error opening output file.\n");
		return EXIT_FAILURE;
	}
	for (int i=0; i<(totalInputFiles); i++)
	{
		inputFilePointer[i] = fopen(argv[i+5], "r");
	}
	queue_init(&hostnameQueue, MAX_INPUT_FILES); 
	pthread_mutex_init(&hostnameQueue_mutex, NULL);
	pthread_mutex_init(&output_mutex, NULL);
	pthread_mutex_init(&service_mutex, NULL);

	requester_information.numOfInput = totalInputFiles;
	requester_information.hostnameQueue_mutex = &hostnameQueue_mutex;
	requester_information.service_mutex = &service_mutex;
	requester_information.output_mutex = &output_mutex; 
	requester_information.hostnameQueue= &hostnameQueue;

	for (int i=0; i<totalInputFiles; i++)
	{
		requester_information.fileArray[i]=inputFilePointer[i];
	}
	for (int i=0; i<requester_threads; i++) 
	{
		pthread_create(&(input_requests[i]), NULL, Requester, &(requester_information));
	}

	for (int i=0; i<resolver_threads; i++) 
	{
		resolver_information[i].threadFile = resultsFilePointer;
		resolver_information[i].hostnameQueue_mutex = &hostnameQueue_mutex;
		resolver_information[i].output_mutex = &output_mutex; 
		resolver_information[i].hostnameQueue = &hostnameQueue;
		resolver_information[i].service_mutex = &service_mutex;
		pthread_create(&(input_resolvers[i]), NULL, Resolver, &(resolver_information[i]));
	}

	for (int i=0; i<requester_threads; i++)
	{
		pthread_join(input_requests[i], NULL);
	}
	does_requester_exist = false;
	for (int i=0; i<resolver_threads; i++)
	{
		pthread_join(input_resolvers[i], NULL);
	}
	queue_cleanup(&hostnameQueue);
	fclose(resultsFilePointer);
	for (int i=0; i<totalInputFiles; i++)
	{
		fclose(inputFilePointer[i]);
	}
	pthread_mutex_destroy(&hostnameQueue_mutex);
	pthread_mutex_destroy(&output_mutex);
	pthread_mutex_destroy(&service_mutex);
	gettimeofday(&end, NULL);
	long long time = 1000000 * (end.tv_sec - begin.tv_sec) + (end.tv_usec - begin.tv_usec);
	printf("The program took %lld milliseconds to run the program.\n", time);
	printf("Program exiting succesfully.\n");
	return 0;
}



