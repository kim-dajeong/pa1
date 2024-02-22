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

#define max_packet_size 1024 //bytes
#define max_payload_size 1000 //bytes
#define max_buffer_size 100 //number of packets (array initalization)

void rsend(char* hostname,
    unsigned short int hostUDPport,
    char* filename,
    unsigned long long int bytesToTransfer)
{

    unsigned long long int bytesRead;
    char payload;
    char message[max_packet_size];


    // Open file for reading
    FILE* file = fopen(filename, "r"); // Open the file in binary mode for reading
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

    // Allocate a buffer to store the read bytes
    char* buffer = (char*)malloc(bytesToRead);
    if (buffer == NULL) {
        perror("Memory allocation error");
        fclose(file);
        exit(EXIT_FAILURE);
    }

    // Create UDP socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        return -1;
    }
    else
        printf("Socket created\n");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(hostUDPport); //Check later

    // Note the client/sender should not need to bind to anything:

    unsigned short orderIndex = 0;
    char buffer[max_packet_size];

        //send the number of packets to the server
    message = size;
    if (sendto(socket_desc, message, strlen(message), 0,
        (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) < 0) {
        printf("Unable to send message\n");
        return -1;
    }

    recvfrom(socket_desc, buffer, sizeof(buffer), 0, (struct sockaddr*)&address, &client_struct_length);
    //message = ACK;
    if(buffer[0] == "ACK") {
        
        sendto(socket_desc, message, strlen(message), 0, (struct sockaddr*)&server_addr, server_struct_length);
        // Clear the contents of the buffer
        memset(buffer, 0, sizeof(buffer));
 

    // Clean up
    free(buffer);
    fclose(file);

    //send FIN

    // Close the socket:
    close(socket_desc);

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
    filename = (char) argv[3];
    bytesToTransfer = atoll(argv[4]);

    // Call sender function
    if (rsend(hostname, hostUDPport, filename, bytesToTransfer)) == -1) {
        return (EXIT_FAILURE);
    }
    else
        return (EXIT_SUCCESS);
}
