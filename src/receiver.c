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

    //Static buffer for receiving data
    void *buffer= malloc(max_payload_size);
    memset(buffer, 0, max_payload_size);

    int bytesRead = 0;   
    int byteNumber = 0;
    unsigned long long int bytesToTransfer = 0;
    void* acknowledgementpointer;
    void* receivedmemorypointer;

    //set size of buffer and assign memory block to pointer
    size_t buffer_size = 2000;  // Set the size of your buffer
    receivedmemorypointer = malloc(buffer_size);

    //start three way handshake

    //wait for syn from sender to initiate conenction
    while(*receivedmemorypointer < 1) {

        recvfrom(socket_desc, receivedmemorypointer, buffer_size, 0, (struct sockaddr*)&address, &client_struct_length); 
        bytesToTransfer = buffer[0];

    }
    
    //set flag high
    int ackflag = 1;
    acknowledgementpointer = &ackflag; 

    //send ack to sender
    sendto(socket_desc, acknowledgementpointer, sizeof(*acknowledgementpointer), 0, (struct sockaddr*)&address, client_struct_length);
    printf("ack sent\n");

    //wait for ack from sender
    while(*receivedmemorypointer < 1) {

        recvfrom(socket_desc, receivedmemorypointer, buffer_size, 0, (struct sockaddr*)&address, &client_struct_length); 
        bytesToTransfer = buffer[0];

    }

    //check flag from sender
    if(*receivedmemorypointer == 1) {

        //start data acquisition

    }

    else {

        //send rst and retry connection initiation
    }

    //end three way handshake


    int index;
    while(1){
    // Receive client's message:
    size_t client_message = recvfrom(socket_desc, buffer, max_payload_size, 0, (struct sockaddr*)&address, &client_struct_length);  
    if (client_message < 0){
    printf("Couldn't receive\n");
        exit(EXIT_FAILURE);
    }

    printf("%ld\n",(client_message));

    int Testval = 22;
    void* test_val = &Testval; 
    if (sendto(socket_desc, test_val, sizeof(test_val), 0, (struct sockaddr*)&address, client_struct_length) < 0) {
    printf("Unable to send ack message\n");
    exit(EXIT_FAILURE);
    }

    // references the first address of the data buffer
    if(*(int*)buffer == 3){ 
        break;
    }

    // Write only the payload data to the file
    int written = fwrite(buffer, 1, client_message, write_file);
    if (written < client_message) {
        printf("Error during writing to file!");
    }

    index++;
    
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

    udpPort = (unsigned short int) atoi(argv[1]);

    // Parse the command-line arguments
    udpPort = (unsigned short int) atoi(argv[1]);
    char* destinationFile = argv[2];

    // Call the rrecv function with the provided arguments
    rrecv(udpPort, destinationFile, 0); // Pass 0 for writeRate, modify as needed

    return 0;
}