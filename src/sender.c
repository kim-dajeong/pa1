/** @file sender.c
 *  @brief Client side for a more reliable file transfer using UDP Protocol. 
 *
 *  The client/sender part of the reliable file transfer using UDP. 
 *
 *  @author Ana Bandari (abandari)
 *          Dajeong Kim 
 * 
 *  @bug Steps: 
 *        1. Only works for small strings, test with large binary files? (binary is of type int!)
 *        2. Currently not using the bytes to transfer.
 *        3. Ack and Fin functionality/struct variables (one structure)
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
Sender Notes
    Inputs: hostname, hostUDP port, filename, 
    Outputs:

    Sender Algorithm Skeleton: 
        - Read from File 
        - Splice the file into sendable bits
        - create socket 
        - send the file bits over through the socket

    wget -O sender.c https://raw.githubusercontent.com/kim-dajeong/pa1/ana-test/src/sender.c
    wget -O readfile.txt https://raw.githubusercontent.com/kim-dajeong/pa1/ana-test/src/readfile.txt

    To use Makefile:
        wget -O Makefile https://raw.githubusercontent.com/kim-dajeong/pa1/test/Makefile
        for sender, enter: "make send" into command line
        for receiver: make recv

*/

#define PAYLOAD_SIZE 16 //! == payload
#define MAX_BUFFER_SIZE 20000
#define TIMEOUT 5000

void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) {

    // Initalizing file I/O and test that the file exists
    FILE *read_file = fopen(filename, "rb");
    if (read_file == NULL){
       printf("Error! Could not open file\n");
       exit(EXIT_FAILURE); // must include stdlib.h
    }

    //fgets(sender_message, max_buffer_size, read_file);
    //printf("String read: %s\n", sender_message);

    // Create socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in client_addr, server_addr;
    int struct_length = sizeof(server_addr);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(hostUDPport);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if (inet_aton(hostname, &server_addr.sin_addr) == 0) {
        fprintf(stderr, "inet_aton() failed\n");
        exit(EXIT_FAILURE);
    }
    
    printf("Socket created successfully\n");

    //initallize array for sender message
    char senderBuffer[MAX_BUFFER_SIZE];
    int bytesRead = 0;
    int index = 0;
    int byteNumber = 0;
    char sendermessage[MAX_BUFFER_SIZE];
    char* readStart;
    char indexPointer[4];

    initiate(bytesToTransfer, &client_addr);

    //senderBuffer[0] = bytesToTransfer;
    //sendto(socket_desc, senderBuffer, strlen(senderBuffer), 0, (struct sockaddr*)&server_addr, struct_length);

    while(bytesRead < bytesToTransfer) {

        // Determine number of bytes to read
        if (PAYLOAD_SIZE < (bytesToTransfer - bytesRead)){
            byteNumber = PAYLOAD_SIZE;
        }
        else {
            byteNumber = (bytesToTransfer - bytesRead);
        }
        //byteNumber = (PAYLOAD_SIZE < (bytesToTransfer - bytesRead)) ? PAYLOAD_SIZE : (bytesToTransfer - bytesRead);

        // Read byteNumber size of file
        fseek(read_file, bytesRead, SEEK_SET);
        fread(senderBuffer, sizeof(char), byteNumber, read_file);

        printf("message: %s, index: %d\n", senderBuffer, index);

        //indexPointer[0] = (char)index;
        //strcat(senderBuffer, indexPointer);

        // Send the message to server:
        sendto(socket_desc, senderBuffer, strlen(senderBuffer), 0, (struct sockaddr*)&server_addr, struct_length);

        index++;
        bytesRead += byteNumber;

    }

    
    printf("Bytes read: %d\n", bytesRead);


    close(socket_desc);
    printf("conneciton closed\n");
    fclose(read_file);

}

// init_buffer [ACK, SYN, FIN, RST, bytesToTransfer ......]
// Three way handshake?
void initiate(bytesToTransfer, struct sockaddr_in *client_addr) {

    char initbuffer[MAX_BUFFER_SIZE];
    char initrecvbuffer[MAX_BUFFER_SIZE];
    unsigned int client_struct_length = sizeof(client_addr);

    //char bytesToTransferinitiate;
    // Create socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    initbuffer[0] = 0;
    initbuffer[1] = 1;
    initbuffer[2] = 0;
    initbuffer[3] = 0;

    // Send SYN to receiver:
    sendto(socket_desc, initbuffer, strlen(initbuffer), 0, (struct sockaddr *)client_addr, sizeof(client_addr));

    while(1) {

        int checktime = timeout(TIMEOUT);
        int client_message = recvfrom(socket_desc, initrecvbuffer, sizeof(initrecvbuffer), 0, (struct sockaddr*)&address, &client_struct_length); 
 
        if(client_message > 0){
            break;
        }

        if(checktime == -1) {
            printf("connection failed to establish, receiver unresponsive");
            break;
        }

    }

    //Check for receiver SYN & ACK
    if(initrecvbuffer[0] == 1 && initrecvbuffer[1] == 1) {

            sendto(socket_desc, initbuffer, strlen(initbuffer), 0, (struct sockaddr *)client_addr, sizeof(client_addr));

    }
    else{

        //Try again - wait no
        sendto(socket_desc, initbuffer, strlen(initbuffer), 0, (struct sockaddr *)client_addr, sizeof(client_addr));

    }

    initbuffer[0] = 1;
    initbuffer[4] = bytesToTransfer;

    sendto(socket_desc, initbuffer, strlen(initbuffer), 0, (struct sockaddr *)client_addr, sizeof(client_addr));
    printf("Connection successfully established; Three way handshake completed. Data transfer starting...")

}


int timeout(timeouttime) {

    while(1) {

        clock_t start_time = clock();
        clock_t end_time;
        end_time = clock();

        // Calculate elapsed time in milliseconds
        unsigned int elapsed_time = (unsigned int)((end_time - start_time) * 1000 / CLOCKS_PER_SEC);

        // Check if the desired time has elapsed
        if (elapsed_time >= timeouttime) {
            
            return -1;
        }

    }

}


int main(int argc, char** argv) {
    // This is a skeleton of a main function.
    // You should implement this function more completely
    // so that one can invoke the file transfer from the
    // command line.
    int hostUDPport;
    unsigned long long int bytesToTransfer;
    char* hostname = NULL;
    char* filename;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }

    // Get values from commandline
    hostname = argv[1];
    hostUDPport = (unsigned short int) atoi(argv[2]);
    bytesToTransfer = atoll(argv[4]);

    // Call sender function
   rsend(hostname, hostUDPport, argv[3], bytesToTransfer);
   return(EXIT_SUCCESS);
} 
