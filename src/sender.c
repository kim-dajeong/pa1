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


*/

#define PAYLOAD_SIZE 1024 //! == payload
#define MAX_BUFFER_SIZE 20000

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

    //initallize array for sender message
    char senderBuffer[MAX_BUFFER_SIZE];
    int bytesRead = 0;
    int index = 0;
    int byteNumber = 0;

    //fgets(sender_message, max_buffer_size, read_file);
    //printf("String read: %s\n", sender_message);

    // Create socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        exit(EXIT_FAILURE);
    }

    struct sockaddr_in address, server_addr;
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


    while(bytesRead < bytesToTransfer) {

        // Determine number of bytes to read
        byteNumber = (PAYLOAD_SIZE < (bytesToTransfer - bytesRead)) ? PAYLOAD_SIZE : (bytesToTransfer - bytesRead);

        char* startRead = senderBuffer + byteNumber;

        // Read byteNumber size of file
        fread(startRead, sizeof(char), byteNumber, read_file);
        for (size_t i = 0; i < byteNumber; i++) {
            printf("%c", (unsigned char)startRead[i]);
        }

            // Send the message to server:
        sendto(socket_desc, startRead, strlen(startRead), 0, (struct sockaddr*)&server_addr, struct_length);

        index += index;

        bytesRead += byteNumber;

    }


    close(socket_desc);
    printf("conenciton closed");
    fclose(read_file);

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
