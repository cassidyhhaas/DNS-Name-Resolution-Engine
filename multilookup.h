// Cassidy Haas
// CSCI 3753 PA 3: DNS Name Resolution Engine
// Due 11/02/17
// Description:This is the C file for the multi lookup implementation
//think about whether you want a seperate c file for implementing a queue, or whether you want to put it on the bottom
//Does header file need to contain declarations for functions in main
//thread about potentially putting all functions but main in another c file and simply creating a header file for that

//header files:
#ifndef MULTI_LOOKUP_H 

#define MULTI_LOOKUP_H

#include <stdlib.h> //for exit
#include <stdio.h> //for fprintf()
#include <string.h>
#include <errno.h>
#include <pthread.h> //for mutex and thread operations
#include <unistd.h> //for fork
#include <time.h> //for gettimeofday()


#include "util.h" //dns lookup functionality

//define values
#define MAX_INPUT_FILES 10
#define USAGE "<inputFilePath> <outputFilePath>"
#define MAX_RESOLVER_THREADS 10
#define MAX_REQUESTER_THREADS 5
#define MAX_NAME_LENGTH 1025
#define MIN_ARGS 3
#define MIN_RESOLVER_THREADS
#define INPUT_FILE_SIZE "%1024s"
#define MAX_FILE_ARRAY_SIZE 5
#define MAX_IP_LENGTH INET6_ADDRSTRLEN

// queue hostnameQueue;
#define QUEUEMAXSIZE 50

#define QUEUE_FAILURE -1
#define QUEUE_SUCCESS 0


typedef int bool;
#define true 1
#define false 0 

extern bool fileComplete;
bool does_requester_exist = true;

typedef struct queue_node_s{
    void* payload;
} queue_node;

typedef struct queue_s{
    queue_node* array;
    int front;
    int rear;
    int maxSize;
} queue;
//Thread Structs

//creating a struct for the request thread function that contains needed variables
struct threadStruct {
	FILE * threadFile;
	queue* hostnameQueue;
	FILE* fileArray[MAX_FILE_ARRAY_SIZE];
	int numOfInput;
	pthread_mutex_t* service_mutex;
	pthread_mutex_t* hostnameQueue_mutex;
	pthread_mutex_t* output_mutex;
};


extern char* servicedFile;

extern bool does_requester_exist;

extern int totalInputFiles;

extern int inputFileCounter;

extern int filePointer;

extern int endOfFiles;

extern int checkPointer;

extern int filesLeft;

// extern int inputFilesInt;

// //global var
// extern int debug;

void* Requester(void* threadarg);



void* Resolver(void* threadarg);


void init_lookup();


void exit_lookup();



/* Function to initilze a new queue
 * On success, returns queue size
 * On failure, returns QUEUE_FAILURE
 * Must be called before queue is used
 */
int queue_init(queue* q, int size);

/* Function to test if queue is empty
 * Returns 1 if empty, 0 otherwise
 */
int queue_is_empty(queue* q);

/* Function to test if queue is full
 * Returns 1 if full, 0 otherwise
 */
int queue_is_full(queue* q);

/* Function add payload to end of FIFO queue
 * Returns QUEUE_SUCCESS if the push successeds.
 * Returns QUEUE_FAILURE if the push fails
 */
int queue_push(queue* q, void* payload);

/* Function to return element from queue in FIFO order
 * Returns NULL pointer if queue is empty
 */
void* queue_pop(queue* q);

/* Function to free queue memory */
void queue_cleanup(queue* q);

#endif




