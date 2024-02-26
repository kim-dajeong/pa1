/* Testing Notes:

wget -O base-receiver.c https://raw.githubusercontent.com/kim-dajeong/pa1/main/sample-test/base-receiver.c

wget -O base-sender.c https://raw.githubusercontent.com/kim-dajeong/pa1/test/sample-test/base-sender.c

gcc -o base-sender base-sender.c

gcc -o base-receiver base-receiver.c

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

#define max_packet_size 1024 //total bytes for a udp data packet
#define max_buffer_size 1024 //number of data bytes 

void rrecv(unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate) {
    // Initalizing file I/O an testing for files not existing
    FILE *write_file = fopen(destinationFile, "w"); // write only
    if (write_file == NULL){  
        printf("Error! Could not open file\n");
        exit(-1); // must include stdlib.h
        }
    

    //Static buffer for receiving data
    char buffer[max_buffer_size];
    

    // initalizing address struct and the structure of the clients address for receiving
    struct sockaddr_in address, client_addr;
    address.sin_family = AF_INET;
    address.sin_port = htons(myUDPport);
    address.sin_addr.s_addr = htonl(INADDR_ANY);   
    int client_struct_length = sizeof(client_addr);


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
        fclose(write_file);
        close(socket_desc);
        exit(EXIT_FAILURE);
    }
    printf("Done with binding\n");

    printf("Listening for incoming messages...\n\n");
/*
    // Receive client's message
    while (1) {
        int recv_size = recvfrom(socket_desc, buffer, sizeof(buffer), 0, (struct sockaddr*)&client_addr, &client_struct_length);  
        printf("%d",recv_size);
        if (recv_size < 0) {
            perror("Error receiving message");
            fclose(write_file);
            close(socket_desc);
            exit(EXIT_FAILURE);
        } else {
            printf("Received message from IP: %s and port: %d\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));
            fwrite(buffer, 1, recv_size, write_file);
        }
    }*/

 // Receiving the data and writing it into the file.

    int addr_size = sizeof(socklen_t);
    int n = recvfrom(socket_desc, buffer, max_buffer_size, 0, (struct sockaddr*)&client_addr, &addr_size);

    if (strcmp(buffer, "END") == 0)
    {
      exit(EXIT_FAILURE);
    }

    printf("[RECEVING] Data: %s", buffer);
    fprintf(write_file, "%s", buffer);
    bzero(buffer, max_buffer_size);

    //Testing to ensure file writing is working as expected 
    //int integer = 22;
    //fprintf(write_file, "%d", integer); // write to file

    // Must close file
    fclose(write_file);
    close(socket_desc);

    //Condition to ignore writeRate for now
    if(writeRate != 0){
        printf("Error Have not yet implemented Task 8\n");
        exit(-1);
    }
}

/* How To Run Main On Terminal:
    - Navigate to the repository 
    - cd to src folder
    - compile using: gcc -o base-receiver base-receiver.c
    - run using: ./base-receiver 8000 destinationFile.txt
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