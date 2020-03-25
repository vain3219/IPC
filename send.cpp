#include <sys/shm.h>
#include <sys/msg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "msg.h"    /* For the message struct */

/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void* sharedMemPtr;

/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the shared memory
 */

void init(int& shmid, int& msqid, void*& sharedMemPtr) {
    /* TODO:
        1. Create a file called keyfile.txt containing string "Hello world" (you may do
             so manually or from the code).
        2. Use ftok("keyfile.txt", 'a') in order to generate the key.
        3. Use the key in the TODO's below. Use the same key for the queue
            and the shared memory segment. This also serves to illustrate the difference
            between the key and the id used in message queues and shared memory. The id
            for any System V objest (i.e. message queues, shared memory, and sempahores)
            is unique system-wide among all SYstem V objects. Two objects, on the other hand,
            may have the same key.
     */
    std::fstream file("keyfile.txt", std::fstream::out);
    file << "Hello World!";
    file.close();

    key_t key = ftok("keyfile.txt", 'a');
    
    /* TODO: Get the id of the shared memory segment. The size of the segment must be SHARED_MEMORY_CHUNK_SIZE */
    shmid = shmget(key, SHARED_MEMORY_CHUNK_SIZE, IPC_CREAT | 0666);
    if (shmid < 0) {
         printf("shmget error\n");
         exit(1);
    }
    /* TODO: Attach to the shared memory */
    sharedMemPtr = (char*)shmat(shmid, (void*)0, 0);
    if ((long)sharedMemPtr == -1) {
         printf("*** shmat error (server) ***\n");
         exit(1);
    }
    /* TODO: Attach to the message queue */
    msqid = msgget(key, IPC_CREAT | 0666);
    if(msqid < 0) {
        perror("msgget");
        exit(1);
    }
    /* Store the IDs and the pointer to the shared memory region in the corresponding parameters */
    std::cout << "Shared memory: " << shmid << "and message queue: " << msqid << "created\n";
}

/**
 * Performs the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */

void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr) {
    /* TODO: Detach from shared memory */
    std::cout << "Cleaning up shared memory\n";
    shmdt(sharedMemPtr);
}

/**
 * The main send function
 * @param fileName - the name of the file
 */
void send(const char* fileName) {
    /* Open the file for reading */
    std::cout << "Preparing to send message\n";
    FILE* fp = fopen(fileName, "r");
    
    /* A buffer to store message we will send to the receiver. */
    message sndMsg;
    /* A buffer to store message received from the receiver. */
    message rcvMsg;
    
    /* Was the file open? */
    if(!fp) {
        perror("fopen");
        exit(1);
    }
    
    /* Read the whole file */
    while(!feof(fp)) {
        std::cout << "Entered file loop\n";
        /* Read at most SHARED_MEMORY_CHUNK_SIZE from the file and store them in shared memory.
          * fread will return how many bytes it has actually read (since the last chunk may be less
          * than SHARED_MEMORY_CHUNK_SIZE).
          */
        if((sndMsg.size = fread(sharedMemPtr, sizeof(char), SHARED_MEMORY_CHUNK_SIZE, fp)) < 0) {
            perror("fread");
            exit(1);
        }
        std::cout << "msg.size = " << sndMsg.size << std::endl;
        /* TODO: Send a message to the receiver telling him that the data is ready
          * (message of type SENDER_DATA_TYPE)
          */
        std::cout << "Sending data ready message\n";
        sndMsg.mtype = SENDER_DATA_TYPE;
        if(msgsnd(msqid, &sndMsg, sizeof(sndMsg), 0) < 0) {
            perror("sned(): msgsnd 1");
            exit(1);
        }
        
        /* TODO: Wait until the receiver sends us a message of type RECV_DONE_TYPE telling us
          * that he finished saving the memory chunk.
          */
        std::cout << "Waiting for receiver message\n";
        do {
            msgrcv(msqid, &rcvMsg, sizeof(rcvMsg), RECV_DONE_TYPE, 0);
            std::cout << rcvMsg.size << " " << rcvMsg.mtype << std::endl;
        }while(rcvMsg.mtype != RECV_DONE_TYPE);
        std::cout << "Receiver message aquired\n";
    }
    
    /** TODO: once we are out of the above loop, we have finished sending the file.
       * Lets tell the receiver that we have nothing more to send. We will do this by
       * sending a message of type SENDER_DATA_TYPE with size field set to 0.
      */
    std::cout << "Signaling end of data transmission\n";
    sndMsg.size = 0;
    sndMsg.mtype = SENDER_DATA_TYPE;
    if(msgsnd(msqid, &sndMsg, 0, 0) < 0) {
        perror("send(): msgsnd 2");
        exit(1);
    }
    std::cout << "Message sent\n";
    /* Close the file */
    fclose(fp);
}


int main(int argc, char** argv) {
    /* Check the command line arguments */
    if(argc < 2) {
        fprintf(stderr, "USAGE: %s <FILE NAME>\n", argv[0]);
        exit(-1);
    }
        
    /* Connect to shared memory and the message queue */
    std::cout << "init()\n";
    init(shmid, msqid, sharedMemPtr);
    
    /* Send the file */
    std::cout << "send()\n";
    send(argv[1]);
    
    /* Cleanup */
    std::cout << "cleanUp()\n";
    cleanUp(shmid, msqid, sharedMemPtr);
        
    return 0;
}