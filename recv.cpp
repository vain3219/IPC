#include <sys/shm.h>
#include <sys/msg.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <iostream>
#include <fstream>
#include "msg.h"    /* For the message struct */
using namespace std;

/* The size of the shared memory chunk */
#define SHARED_MEMORY_CHUNK_SIZE 1000

/* The ids for the shared memory segment and the message queue */
int shmid, msqid;

/* The pointer to the shared memory */
void *sharedMemPtr;

/* The name of the received file */
const char recvFileName[] = "recvfile";


/**
 * Sets up the shared memory segment and message queue
 * @param shmid - the id of the allocated shared memory
 * @param msqid - the id of the shared memory
 * @param sharedMemPtr - the pointer to the shared memory
 */

void init(int& shmid, int& msqid, void*& sharedMemPtr) {
    /* TODO: 1. Create a file called keyfile.txt containing string "Hello world" (you may do
             so manually or from the code).
             2. Use ftok("keyfile.txt", 'a') in order to generate the key.
         3. Use the key in the TODO's below. Use the same key for the queue
            and the shared memory segment. This also serves to illustrate the difference
            between the key and the id used in message queues and shared memory. The id
            for any System V object (i.e. message queues, shared memory, and sempahores)
            is unique system-wide among all System V objects. Two objects, on the other hand,
            may have the same key.
     */
    fstream file("keyfile.txt", fstream::out);
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
    cout << "Shared memory: " << shmid << "and message queue: " << msqid << "created\n";
}
 

/**
 * The main loop
 */
void mainLoop() {
    /* The size of the mesage */
    size_t msgSize = 0;
    
    /* Open the file for writing */
    FILE* fp = fopen(recvFileName, "w");
        
    /* Error checks */
    if(!fp) {
        perror("fopen");
        exit(1);
    }
        
    /* TODO: Receive the message and get the message size. The message will
     * contain regular information. The message will be of SENDER_DATA_TYPE
     * (the macro SENDER_DATA_TYPE is defined in msg.h).  If the size field
     * of the message is not 0, then we copy that many bytes from the shared
     * memory region to the file. Otherwise, if 0, then we close the file and
     * exit.
     *
     * NOTE: the received file will always be saved into the file called
     * "recvfile"
     */
    cout << "Receiving message size 1\n";
    message snd;
    message rcv;
    msgSize = msgrcv(msqid, &rcv, sizeof(rcv), SENDER_DATA_TYPE, 0);
    /* Keep receiving until the sender set the size to 0, indicating that
      * there is no more data to send
      */
    cout << "Enter msg loop\n";
    while(msgSize != 0) {
        /* If the sender is not telling us that we are done, then get to work */
        if(msgSize != 0) {
            /* Save the shared memory to file */
            cout << "Saving to file from shared memory\n";
            if(fwrite(sharedMemPtr, sizeof(char), msgSize, fp) < 0) {
                perror("fwrite");
            }
            
            /* TODO: Tell the sender that we are ready for the next file chunk.
              * I.e. send a message of type RECV_DONE_TYPE (the value of size field
              * does not matter in this case).
              */
            cout << "Ready for next file chunk\n";
            snd.mtype = RECV_DONE_TYPE;
            msgsnd(msqid, &snd, sizeof(snd), 0);
        }
        /* We are done */
        else {
            /* Close the file */
            fclose(fp);
        }
        cout << "Receiving message size 2\n";
        msgSize = msgrcv(msqid, &rcv, sizeof(rcv), SENDER_DATA_TYPE, 0);
    }
}



/**
 * Perfoms the cleanup functions
 * @param sharedMemPtr - the pointer to the shared memory
 * @param shmid - the id of the shared memory segment
 * @param msqid - the id of the message queue
 */
void cleanUp(const int& shmid, const int& msqid, void* sharedMemPtr) {
    /* TODO: Detach from shared memory */
    shmdt(sharedMemPtr);
    /* TODO: Deallocate the shared memory chunk */
    shmctl(shmid, IPC_RMID, NULL);
    /* TODO: Deallocate the message queue */
    msgctl(msqid, IPC_RMID, NULL);
    cout << "Resources released\n";
}

/**
 * Handles the exit signal
 * @param signal - the signal type
 */

void ctrlCSignal(int signal) {
    /* Free system V resources */
    cleanUp(shmid, msqid, sharedMemPtr);
    exit(signal);
}

int main(int argc, char** argv) {
    
    /* TODO: Install a singnal handler (see signaldemo.cpp sample file).
      * In a case user presses Ctrl-c your program should delete message
      * queues and shared memory before exiting. You may add the cleaning functionality
      * in ctrlCSignal().
      */
    signal(SIGINT, ctrlCSignal);
                
    /* Initialize */
    cout << "init()\n";
    init(shmid, msqid, sharedMemPtr);
    
    /* Go to the main loop */
    cout << "mainLoop()\n";
    mainLoop();
    cout << "Outputing file conents\n";
    string str;
    ifstream ifile(recvFileName);
    if(ifile.is_open()) {
        while(getline(ifile, str))
            cout << str << endl;
        
        ifile.close();
    } else {
        perror("ifile");
    }
    /** TODO: Detach from shared memory segment, and deallocate shared memory and message queue (i.e. call cleanup) **/
    cout << "cleanUp()\n";
    cleanUp(shmid, msqid, sharedMemPtr);
    return 0;
}
