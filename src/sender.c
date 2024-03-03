/*@file sender.c
 *
 *  @brief Client side for a more reliable file transfer using UDP sockets. 
 *
 *  @author Ana Bandari (abandari)
 *  @author Dajeong Kim (dkim2)
 * 
 *  @bug No known bugs
 * 
 */


/*   Includes   */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>
#include <errno.h>
#include <time.h>




/*
Sender Notes
    Inputs: hostname, hostUDP port, filename, bytesToTransfer
    Outputs: no outputs

    Sender Algorithm Skeleton: 
        - Read from File (raw data)
        - Splice the file into sendable bits
        - create socket 
        - send the file bits over through the socket


*/

#define max_payload_size 1024 //! max data we can send over
#define max_data_size 1018 //six bytes less than max payload to make space for the header 

void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) {

    clock_t socket_open_time, socket_close_time; 
    double total_socket_open_time; 
    
    clock_t send_time_start, send_time_finish; 
    double total_send_socket_time = 0; 
    
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
    //socket_open_time = clock();

    // setting up hostname connection on sender
    struct sockaddr_in address, server_addr;
    unsigned int struct_length = sizeof(server_addr);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(hostUDPport);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Resolving hostname
    struct hostent *hp = gethostbyname(hostname);
    char ip_address[INET_ADDRSTRLEN];
    memset(ip_address, 0, INET_ADDRSTRLEN); 
    struct in_addr **p1 = NULL;

    switch (hp->h_addrtype) {
        case AF_INET:
            p1 = (struct in_addr **)hp->h_addr_list;

            // Use only the first resolved IP address
            if (p1[0] != NULL) {
                inet_ntop(AF_INET, p1[0], ip_address, INET_ADDRSTRLEN);
                printf("sending to: %s\n", ip_address);
            
                if (inet_aton(ip_address, &server_addr.sin_addr) == 0) {
                    fprintf(stderr, "inet_aton() failed\n");
                    exit(EXIT_FAILURE);
                }
            } 
            else{
            fprintf(stderr, "No IP address found for the hostname\n");
            exit(EXIT_FAILURE);
            }
            break;
        
        default:
            fprintf(stderr, "Unsupported address type\n");
            exit(EXIT_FAILURE);
    }



    // Set the receive timeout
    struct timeval timeout;
    timeout.tv_sec = 0;  // seconds
    timeout.tv_usec = 10000; // microseconds

    struct timeval start, end;
    double elapsed_time;

    if (setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        close(socket_desc);
        exit(EXIT_FAILURE);
    }
    
    printf("Socket created successfully\n");

    //sender_buffer total data with header
    void *sender_buffer = malloc(max_payload_size);
    memset(sender_buffer, 0, max_payload_size);
    if (sender_buffer == NULL) {
        fprintf(stderr, "Memory allocation failed\n");
        exit(EXIT_FAILURE);
    }

    //initallize void pointer for sender message to get raw bytes from the file (purpose is just to point to 1018 bytes of data)
    void *readfile_data = malloc(max_data_size);
    memset(readfile_data, 0, max_data_size);
    

    //receive buffer setup (buffer of max size to recieve the ack)
    void *ack_buffer= malloc(max_payload_size);
    memset(ack_buffer, 0, max_payload_size);

    //initallize the sending while loop
    int bytesRead = 0;
    useconds_t t = 1000;
    unsigned index = 0;
    int byteNumber = 0;

    socket_open_time = clock();

    while(bytesRead < bytesToTransfer) {
        // Determine number of bytes to read
        byteNumber = (max_data_size < (bytesToTransfer - bytesRead)) ? max_data_size : (bytesToTransfer - bytesRead);

        //initallize void pointer for sender message to get raw bytes from the file
        memset(readfile_data, 0, max_data_size);
        memset(sender_buffer, 0, max_payload_size);
        memset(ack_buffer, 0, max_payload_size);

        // Read byteNumber size of file
        fseek(read_file, bytesRead, SEEK_SET);
        fread(readfile_data, 1, byteNumber, read_file);
        
        // Copy the two uint8_t values to the start of the new buffer
        uint8_t ack_flag=0;
        uint8_t fin_flag=0;
        memcpy(sender_buffer, &ack_flag, 1);
        memcpy(sender_buffer+1, &fin_flag, 1);

        memcpy(sender_buffer+2, &index, 4);
        memcpy(sender_buffer+6, readfile_data, byteNumber);

        // Send the message to server:
        usleep(t);

        send_time_start = clock();
        if(sendto(socket_desc, sender_buffer, byteNumber+6, 0, (struct sockaddr*)&server_addr, struct_length)<0){
            printf("Unable to send message\n");
        }
        send_time_finish = clock();
        total_send_socket_time += ((double) (send_time_finish - send_time_start)) / CLOCKS_PER_SEC;
       
        
        size_t client_message = recvfrom(socket_desc, ack_buffer, max_payload_size, 0, (struct sockaddr*)&server_addr, &struct_length);  
        if (client_message < 0){
            printf("Couldn't receive\n");
        }
        //printf("ack buffer client message: %ld\n", client_message);
        
        uint8_t ack_message;
        memcpy(&ack_message, ack_buffer, 1);
        // testing an ack backbone 
        if(ack_message == 1){ 
            //printf("Hello I Hear You! For index: %d \n", index);
            index++;
            bytesRead += byteNumber;
            if(t>0){
                t = t/2;
                //printf("delay: %d\n", t);
            }
        }
        if(ack_message == 0){ 
            //printf("Oh No! Lost index: %d \n", index);
            //additive increase?
            if(t<1000000) {
                t += 1000;
            }
            printf("new timeout: %d\n", t);

        }

        
    }

    // Terminating connection 
    memset(sender_buffer, 0, max_payload_size);
    uint8_t ack_flag = 0;
    uint8_t fin_flag = 1;
    memcpy(sender_buffer, &ack_flag, 1);
    memcpy(sender_buffer+1, &fin_flag, 1);
    if (sendto(socket_desc, sender_buffer, 2, 0, (struct sockaddr*)&server_addr, struct_length) < 0) {
    printf("Unable to send message\n");
    exit(EXIT_FAILURE);
    }
    
    uint8_t ack_message;

    // Wait for ack from recevier
    while (ack_message != 1) {

        size_t client_message = recvfrom(socket_desc, ack_buffer, max_payload_size, 0, (struct sockaddr*)&server_addr, &struct_length);  
        if (client_message < 0){
            printf("Couldn't receive\n");
        }
        
        memcpy(&ack_message, ack_buffer, 1);

    }


    close(socket_desc);
    socket_close_time = clock(); 
    total_socket_open_time = ((double) (socket_close_time - socket_open_time)) / CLOCKS_PER_SEC;
    printf("The socket has been open for: %f seconds\n", total_socket_open_time);
    printf("The socket has been sending for: %f seconds\n", total_send_socket_time);

    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;

    printf("Elapsed time: %f seconds\n", elapsed_time);
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