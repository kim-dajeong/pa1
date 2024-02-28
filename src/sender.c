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
    ./sender 130.127.132.208 8000 readfile 2000

*/

#define max_payload_size 1024 //! max data we can send over
#define max_data_size 1018 //six bytes less than max payload to make space for the header 

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

    //sender_buffer total data with header
    void *sender_buffer = malloc(max_payload_size+1);
    memset(sender_buffer, 0, max_payload_size+1);
    if (sender_buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    //initallize void pointer for sender message to get raw bytes from the file
    void *readfile_data = malloc(max_data_size);
    memset(readfile_data, 0, max_data_size);

    //receive buffer setup
    void *ack_buffer= malloc(max_payload_size);
    memset(ack_buffer, 0, max_payload_size);

    //initallize the sending while loop
    int bytesRead = 0;
    int index = 0;
    unsigned int byteNumber = 0;

    while(1) {
        // Determine number of bytes to read
        byteNumber = (max_data_size < (bytesToTransfer - bytesRead)) ? max_data_size : (bytesToTransfer - bytesRead);

        //initallize void pointer for sender message to get raw bytes from the file
        memset(readfile_data, 0, byteNumber);
        memset(sender_buffer, 0, byteNumber+6);

        // Read byteNumber size of file
        fseek(read_file, bytesRead, SEEK_SET);
        fread(readfile_data, 1, byteNumber, read_file);

        // Copy the two uint8_t values to the start of the new buffer
        uint8_t *flag_ptr;
        flag_ptr[0] = 0;
        flag_ptr[1] = 0;
        memcpy(sender_buffer, flag_ptr, 2);
        memcpy(sender_buffer+2, &index, 4);
        memcpy(sender_buffer+6, readfile_data, byteNumber);

        printf("%d", index);
        printf("%hhn",flag_ptr);

        // Send the message to server:
        if(sendto(socket_desc, sender_buffer, byteNumber+6, 0, (struct sockaddr*)&server_addr, struct_length)<0){
            printf("Unable to send message\n");
            exit(EXIT_FAILURE);
        }

        size_t client_message = recvfrom(socket_desc, ack_buffer, max_payload_size, 0, (struct sockaddr*)&server_addr, &struct_length);  
        if (client_message < 0){
            printf("Couldn't receive\n");
        exit(EXIT_FAILURE);
        }

        // testing an ack backbone 
        if(*(int*)ack_buffer == 22){ 
            printf("Hello I Hear You!\n");
        }

        index++;
        bytesRead += byteNumber;
    }

    // Terminating connection 
    void* terminate; 
    uint8_t *flag_ptr;
    flag_ptr[0] = 0;
    flag_ptr[1] = 1;
    memcpy(terminate, flag_ptr, 2);
    if (sendto(socket_desc, terminate, 2, 0, (struct sockaddr*)&server_addr, struct_length) < 0) {
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