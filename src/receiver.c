/** @file receiver.c
 *  @brief Server functionality for a more reliable file transfer using UDP Protocol. 
 *
 *  The server/reciever part of the reliable file transfer using UDP. 
 *
 *  @author Ana Bandari (anabandari)
 *  @bug Be able to recieve muliple packets
 *       End when a FIN is sent  
 */


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

/*!
Receiver Notes
    Inputs: UDP Port, pointer to destination file, write rate (assume 0 for now)

    Outputs: Write data to the destination file. 

Receiver Algorithm Skeleton: 
    - Open a file for writing <complete>
    - Create large buffer <complete>
        - Implementing a large static array.  
    - Create Socket to recieve infomration from sender <complete>

Loop: Start recieving packets
    - Check if packet is empty 
        - send negative ack 
    - Check if packet is in order 
        - add to buffer in correct position 
        - send ack
    - If nothing recieved in {timeout (~40ms)} send an ack for the last packet it has 

    wget -O receiver.c https://raw.githubusercontent.com/kim-dajeong/pa1/main/src/receiver.c
    gcc -o receiver receiver.c
    ./receiver 8000 destinationFile


*/

#define max_payload_size 1024 //! the total amount of data we want to send over our UDP socket in one packet

void rrecv(unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate){
    
    // Initalizing file I/O and test that the file exists
    FILE *write_file = fopen(destinationFile, "wb"); // write only
    if (write_file == NULL){  
        printf("Error! Could not open file\n");
        exit(EXIT_FAILURE); // must include stdlib.h
        }
    
    // initalizing address struct and the structure of the clients address for receiving
    struct sockaddr_in address, client_addr;
    address.sin_family = AF_INET;
    address.sin_port = htons(myUDPport);
    address.sin_addr.s_addr = htonl(INADDR_ANY);   
    unsigned int client_struct_length = sizeof(client_addr);

    // Create UDP socket and check it exists
    int socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        exit(EXIT_FAILURE);
    }

    printf("Socket created successfully\n");

    // Bind socket to the receive address
    if(bind(socket_desc, (struct sockaddr*)&address, sizeof(address)) < 0){
        printf("Couldn't bind to the port\n");
        exit(EXIT_FAILURE);
    }
    printf("Done with binding\n");

    printf("Listening for incoming messages...\n\n");

    uint8_t ack = 0;
    int index = 0;
    void* sendmemorypointer;
    void* receivedmemorypointer;
    void* ackpointer;
    void* finpointer;
    void* datapointer;
    void* indexpointer;

    //set size of buffer and assign memory block to pointer
    size_t buffer_size = 4000;  // Set the size of your buffer
    receivedmemorypointer = malloc(buffer_size);
    sendmemorypointer = malloc(buffer_size);

    ackpointer = sendmemorypointer;
    finpointer = ((char*)receivedmemorypointer + 1);
    indexpointer = ((char*)receivedmemorypointer + 2);
    datapointer = ((char*)receivedmemorypointer + 6);


    while(1){ 

        memset(sendmemorypointer, 0, buffer_size);
        memset(receivedmemorypointer, 0, buffer_size);

        // Receive sender's (client) message:
        size_t client_message = recvfrom(socket_desc, receivedmemorypointer, max_payload_size, 0, (struct sockaddr*)&address, &client_struct_length);  

        printf("client message: %ld\n", client_message);

        uint8_t fincomp;
        uint32_t indexcomp;
        memcpy(&fincomp, (uint8_t*)finpointer, 1);
        memcpy(&indexcomp, (uint8_t*)indexpointer, 4);


        if (client_message < 0){

            //error receiving message
            //set flag low - nack?
            ack = 0;
            memcpy(ackpointer, &ack, 1);

            //send nack to sender
            sendto(socket_desc, sendmemorypointer, buffer_size, 0, (struct sockaddr*)&address, client_struct_length);
            printf("nack sent\n");

        }
       
        else if (fincomp == 1) {

            printf("fin received, ending conenction\n");

            //set flag high - acknowledge fin
            ack = 1;
            memcpy(ackpointer, &ack, 1);

            //send ack to sender and exit while loop
            sendto(socket_desc, sendmemorypointer, buffer_size, 0, (struct sockaddr*)&address, client_struct_length);
            break;

        } 
        else if(indexcomp == index) {

            //print received message
            // char *strPointer = (int *)datapointer;
            // printf("%s\n", *strPointer);

            

            //write to file and check
            size_t written = fwrite(datapointer, 1, client_message-6, write_file);
            printf("index match writing now :)\n");
            
            // Write only the payload data to the file
            fseek(write_file, client_message-6, SEEK_CUR);
            if (written < client_message-6) {
                printf("Error during writing to file!");
            }

            //set flag high
            ack = 1;
             memcpy(ackpointer, &ack, 1);

            //send ack to sender
            sendto(socket_desc, sendmemorypointer, buffer_size, 0, (struct sockaddr*)&address, client_struct_length);
            //printf("ack sent\n");
            //increment index
            printf("ack sent\n");
            index++;
            printf("index: %d", index);


        }
        else {

            //set flag low - nack?
            ack = 0;
            memcpy(ackpointer, &ack, 1);
            memcpy(indexpointer, &index, 4);

            //send nack to sender
            sendto(socket_desc, sendmemorypointer, buffer_size, 0, (struct sockaddr*)&address, client_struct_length);
            printf("nack sent\n");

        }

    
        ack = 0;
    
    }
    

    // Must close file and socket
    fclose(write_file);
    close(socket_desc);

    //Condition to ignore writeRate for now
    if(writeRate != 0){
        printf("Error Have not yet implemented Task 8\n");
        exit(EXIT_FAILURE);
    }

}

/*! How To Run Main On Terminal:
    - Navigate to the repository 
    - cd to src folder
    - compile using: gcc -o receiver receiver.c
    - run using: ./receiver <UDP port number> destinationFile.txt
    - hostname -i
*/
int main(int argc, char** argv){
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.

    unsigned short int udpPort;

    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    // Parse the command-line arguments
    udpPort = (unsigned short int) atoi(argv[1]);
    char* destinationFile = argv[2];

    // Call the rrecv function with the provided arguments
    rrecv(udpPort, destinationFile, 0); // Pass 0 for writeRate, modify as needed

    return 0;
}
