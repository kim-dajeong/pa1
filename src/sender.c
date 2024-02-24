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

#define PACKET_SIZE = 1024;
#define BUFFER_SIZE = 10000;

void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) 
{

    char senderbuffer[BUFFER_SIZE];
    char receiverbuffer[BUFFER_SIZE];

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(hostUDPport); //Check later
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

       // Open file for reading
    FILE* file = fopen(filename, "r");
    if (file == NULL) {
        perror("Error opening file");
        exit(EXIT_FAILURE);
    }

        // Create UDP socket:
    int socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if (socket_desc < 0) {
        printf("Error while creating socket\n");
    }
    else
        printf("Socket created\n");


        int index = 0;

        // Read from file into buffer
    senderbuffer[index] = fread(senderbuffer, 1, PACKET_SIZE, filename);

        // Send packet
    if (sendto(socket_desc, senderbuffer, PACKET_SIZE, 0,
        (struct sockaddr*)&server_addr, sizeof(struct sockaddr_in)) == -1) {
            fprint("Error sending packet");
        }
    else fprint("Successfully sent message");
    
    close(socket_desc);
    fclose(file);

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

    hostname = argv[1];
    hostUDPport = (unsigned short int) atoi(argv[2]);
    filename = argv[3];
    bytesToTransfer = atoll(argv[4]);

    // Call sender function
   rsend(hostname, hostUDPport, filename, bytesToTransfer);
}