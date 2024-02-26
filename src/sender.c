/** @file sender.c
 *  @brief Client side for a more reliable file transfer using UDP Protocol. 
 *
 *  The client/sender part of the reliable file transfer using UDP. 
 *
 *  @author Ana Bandari (abandari)
 *          Dajeong Kim 
 * 
 *  @bug Steps: 
 *        1. Only works for small strings, test with raw data type
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
    Outputs: N/A

    Sender Algorithm Skeleton: 
        - Read from File (raw data)
        - Splice the file into sendable bits
        - create socket 
        - send the file bits over through the socket

    wget -O sender.c https://raw.githubusercontent.com/kim-dajeong/pa1/main/src/sender.c
    wget -O readfile https://raw.githubusercontent.com/kim-dajeong/pa1/main/src/readfile
    gcc -o sender sender.c
    ./sender 130.127.132.208 8000 readfile 100

*/

#define max_payload_size 1024 //! max data we can send over

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

    //determining max readfile bytes to check that bytes to transfer doesnt exceed the limit
    fseek(read_file, 0, SEEK_END);
    long readfile_size = ftell(read_file);
    fseek(read_file,0, SEEK_SET); 
    if(bytesToTransfer>readfile_size){
        printf("There are only %ld bytes in this file. Try again.\n", readfile_size);
        exit(EXIT_FAILURE);
    }

    // Create socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        exit(EXIT_FAILURE);
    }

    // setting up hostname connection on sender
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


    //initallize void pointer for sender message to get raw bytes from the file
    void *readfile_message = malloc(max_payload_size);
    memset(readfile_message, 0, max_payload_size);

    //initallize the sending while loop
    int bytesRead = 0;
    int index = 0;
    int byteNumber = 0;

    while(bytesRead < bytesToTransfer) {
        // Determine number of bytes to read
        byteNumber = (max_payload_size < (bytesToTransfer - bytesRead)) ? max_payload_size : (bytesToTransfer - bytesRead);

        // Read byteNumber size of file
        fseek(read_file, bytesRead, SEEK_SET);
        fread(readfile_message, 1, byteNumber, read_file);

        // Send the message to server:
        if(sendto(socket_desc, readfile_message, strlen(readfile_message), 0, (struct sockaddr*)&server_addr, struct_length)<0){
            printf("Unable to send message\n");
            exit(EXIT_FAILURE);
        }

        index++;
        bytesRead += byteNumber;
        memset(readfile_message, 0, max_payload_size);
    }

    //terminating connection 
    const char *terminate[1];
    terminate[0] = "FIN";
    if(sendto(socket_desc, terminate[0], strlen(terminate[0]), 0, (struct sockaddr*)&server_addr, struct_length)<0){
            printf("Unable to send message\n");
            exit(EXIT_FAILURE);
        }


    close(socket_desc);
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