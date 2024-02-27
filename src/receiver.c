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
#include <time.h>

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

    wget -O receiver.c https://raw.githubusercontent.com/kim-dajeong/pa1/ana-test/src/receiver.c


*/

#define max_packet_size 1024 //!<bytes
#define MAX_BUFFER_SIZE 1000 //! == payload
#define PAYLOAD_SIZE 16 //! == payload
#define TIMEOUT 10000

//#define bytesToTransfer 400 // value needs to be received from sender


//timeout function - wait for TIMEOUT amount of milisecs
int timeout(int timeouttime) {

//    char buffer[MAX_BUFFER_SIZE];

//    size_t client_message = recvfrom(socket_desc, buffer, sizeof(buffer), 0, (struct sockaddr*)&address, &client_struct_length); 

    while(1) {

//        client_message = recvfrom(socket_desc, buffer, sizeof(buffer), 0, (struct sockaddr*)&address, &client_struct_length); 

        clock_t start_time = clock();
        clock_t end_time;
        end_time = clock();

        // Calculate elapsed time in milliseconds
        unsigned int elapsed_time = (unsigned int)((end_time - start_time) * 1000 / CLOCKS_PER_SEC);

        // Check if the desired time has elapsed
        if (elapsed_time >= timeouttime) {
//            buffer[1] = 1;
            return -1;
        }

    }

//    return client_message;

}

// init_buffer [ACK, SYN, FIN, RST, bytesToTransfer ......]
// Three way handshake?
//return bytesToTransfer
int initiate(struct sockaddr_in *address) {

    char mybuffer[MAX_BUFFER_SIZE];
    char otherbuffer[MAX_BUFFER_SIZE];
    unsigned int client_struct_length = sizeof(address);
    int bytesToTransfer;
    int checktime = 0;
    int client_message = 0;

    //char bytesToTransferinitiate;
    // Create socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    mybuffer[0] = 0;
    mybuffer[1] = 0;
    mybuffer[2] = 0;
    mybuffer[3] = 0;

    //wait for SYN from sender
    while(client_message == 0) {

        printf("star checking for message\n");
        //checktime = timeout(TIMEOUT);
        client_message = recvfrom(socket_desc, otherbuffer, sizeof(otherbuffer), 0, (struct sockaddr*)&address, &client_struct_length); 
 
        if(client_message > 0){
            
            printf("first syn received\n");
            break;

        }

        if(client_message == -1) {
            printf("connection failed to establish, receiver unresponsive\n");
            break;
        }
        
        printf("message not received yet\n");

    }

    //Check sender SYN
    if(otherbuffer[1] == 1) {

        mybuffer[0] = 1;
        mybuffer[1] = 1;
        sendto(socket_desc, mybuffer, strlen(mybuffer), 0, (struct sockaddr *)address, sizeof(address));

    }
    else{
        
            printf("connection failed to establish, SYN not received from sender\n");
            return -1;
        //Try again - need to implement
        //sendto(socket_desc, mybuffer, strlen(mybuffer), 0, (struct sockaddr *)client_addr, sizeof(client_addr));

    }

    //wait for ACK and bytesToTransfer from sender
    while(1) {

        int checktime = timeout(TIMEOUT);
        int client_message = recvfrom(socket_desc, otherbuffer, sizeof(otherbuffer), 0, (struct sockaddr*)&address, &client_struct_length); 
 
        if(client_message > 0){
            break;
        }

        if(checktime == -1) {
            printf("connection failed to establish, receiver unresponsive\n");
            break;
        }

        return 1;

    }


    //Check sender ACK & bytesToTransfer
    if(otherbuffer[0] == 1) {

        bytesToTransfer = otherbuffer[4];
    
    }
    else{
        
            printf("connection failed to establish, ACK not received from sender\n");
            return -1;
        //Try again - need to implement
        //sendto(socket_desc, mybuffer, strlen(mybuffer), 0, (struct sockaddr *)client_addr, sizeof(client_addr));

    }
    
    printf("Connection successfully established; Three way handshake completed. Data transfer starting...\n");

    return bytesToTransfer;

}


void rrecv(unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate) {
    
    // Initalizing file I/O and test that the file exists
    FILE *write_file = fopen(destinationFile, "wb"); // write only
    if (write_file == NULL){  
        printf("Error! Could not open file\n");
        exit(EXIT_FAILURE); // must include stdlib.h
        }

    //Static buffer for receiving data
    char buffer[max_packet_size];
    
    // initalizing address struct and the structure of the clients address for receiving
    struct sockaddr_in address, client_addr;
    address.sin_family = AF_INET;
    address.sin_port = htons(myUDPport);
    address.sin_addr.s_addr = htonl(INADDR_ANY);   
    unsigned int client_struct_length = sizeof(address);

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
    
    //printf("initiate startubng\n");
    //int bytesToTransfer = initiate(&address);
    unsigned long long int bytesToTransfer = 0; //400;
    buffer[0] = 0;


    printf("Listening for incoming messages...\n\n");

    int bytesRead = 0;   
    int byteNumber = 0;

    while(bytesToTransfer < 1) {

        recvfrom(socket_desc, buffer, sizeof(buffer), 0, (struct sockaddr*)&address, &client_struct_length); 
        bytesToTransfer = ntohl(buffer[0]);
    }

    printf("bytesToTransfer: %lld\n", bytesToTransfer);

    buffer[0] = 1;
    sendto(socket_desc, buffer, sizeof(buffer), 0, (struct sockaddr*)&address, client_struct_length);
    printf("ack sent\n");

    while( bytesRead < bytesToTransfer){
        printf("bytesRead: %d\n", bytesRead);

    // Determine number of bytes to read
    //byteNumber = (PAYLOAD_SIZE < (bytesToTransfer - bytesRead)) ? PAYLOAD_SIZE : (bytesToTransfer - bytesRead);
    if (PAYLOAD_SIZE < (bytesToTransfer - bytesRead)){
            byteNumber = PAYLOAD_SIZE;
        }
        else {
            byteNumber = (bytesToTransfer - bytesRead);
        }

    // Receive client's message:
    size_t client_message = recvfrom(socket_desc, buffer, sizeof(buffer), 0, (struct sockaddr*)&address, &client_struct_length); 
    printf("message: %s\n", buffer);

    if (client_message < 0){
    printf("Couldn't receive\n");
        exit(EXIT_FAILURE);
    }

    // Write only the payload data to the file
    fseek(write_file, bytesRead, SEEK_SET);

    int written = fwrite(buffer, sizeof(char), client_message, write_file);
    if (written < client_message) {
        printf("Error during writing to file!\n");
    }

    bytesRead += byteNumber;

    }


    printf("Bytes read: %d", bytesRead);

    // else 
    // check order (first 2 byes of the package)
    // if order doesnt match then send nack
    // if order matches send ack 
        // write to file
        // wipe buffer 

    // Must close file and socket
    fclose(write_file);
    close(socket_desc);

    //Condition to ignore writeRate for now
    if (writeRate != 0){
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