#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>

#include <pthread.h>
#include <errno.h>

/*
Receiver Notes
    Inputs: UDP Port, pointer to destination file, write rate (assume 0 for now)

    Outputs: Write data to the destination file. 

Receiver Algorithm Skeleton: 
    - Open a file for writing <complete>
    - Create large buffer <complete>
        - Implementing a large static array.  
    - Create Socket to recieve infomration from sender <in progress>

Loop: Start recieving packets
    - Check if packet is empty 
        - send negative ack 
    - Check if packet is in order 
        - add to buffer in correct position 
        - send ack
    - If packet is unordered place in correct order in buffer
        - send ack 
    - If nothing recieved in {timeout (~40ms)} send an ack for the last packet it has 

*/

#define max_packet_size 1024 //bytes
#define max_buffer_size 100 //number of packets (array initalization)

void rrecv(unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate) {
    // Initalizing file I/O
    FILE *write_file = fopen(destinationFile, "w"); // write only

    //Static buffer for receiving data
    int buffer[max_buffer_size];
    
    // test for files not existing.
    if (write_file == NULL){  
        printf("Error! Could not open file\n");
        exit(-1); // must include stdlib.h
        }

    //Testing to ensure file writing is working as expected 
    int integer = 22;
    fprintf(write_file, "this is a test %d\n", integer); // write to file

    // Must close file
    fclose(write_file);

    //Condition to ignore writeRate for now
    if (writeRate != 0){
        printf("Error Have not yet implemented Task 8\n");
        exit(-1);
    }
}

/* How To Run Main On Terminal:
    - Navigate to the repository 
    - cd to src folder
    - compile using: gcc -o receiver receiver.c
    - run using: ./receiver <UDP port number> destinationFile.txt
*/

int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    udpPort = (unsigned short int) atoi(argv[1]);

    // Parse the command-line arguments
    udpPort = (unsigned short int) atoi(argv[1]);
    char* destinationFile = argv[2];

    // Call the rrecv function with the provided arguments
    rrecv(udpPort, destinationFile, 0); // Pass 0 for writeRate, modify as needed

    return 0;
}