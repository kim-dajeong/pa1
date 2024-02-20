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
    socket_desc = socket(AF_INET, SOCK_DGRAM, 0);

    if (socket_desc < 0) {
        printf("Error while creating socket\n");
        return -1;
    }
    else
        printf("Socket created\n");

    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = hostUDPport; //Check later
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY); //Check later

    // Bind to the set port and IP:
    if (bind(socket_desc, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        printf("Couldn't bind to the port\n");
        return -1;
    }
    printf("Done with binding\n");

    unsigned short orderIndex = 0;

    //message = ACK;
    //sendto(socket_desc, message, strlen(message), 0, (struct sockaddr*)&server_addr, server_struct_length) < 0);

    while (bytesRead < bytesToTransfer) {

        // if( first byte of received message == ACK){

                // Read bytes from the file
                payload = fread(buffer, 1, max_payload_size, file)
                // Keep track of how many bytes were read
                bytesRead += max_payload_size;
        
                // Check for read errors
                if (ferror(file)) {
                    perror("Error reading from file");
                }
        
                // Add order index and concatenate message
                message = orderIndex;
                strcat(message, payload); //Check later
                orderIndex++;                
                
                // Send the message to server:
                if (sendto(socket_desc, message, strlen(message), 0,
                    (struct sockaddr*)&server_addr, server_struct_length) < 0) {
                    printf("Unable to send message\n");
                    return -1;
                }

        }

    //check for NACK(send again), timeout(send again), else wait
        //else if (time == TIMEOUT length) {
            
        }

    }

    

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
    filename = argv[3];
    bytesToTransfer = atoll(argv[4]);

    // Call sender function
    if (rsend(hostname, hostUDPport, filename, bytesToTransfer)) == -1) {
        return (EXIT_FAILURE);
    }
    else
        return (EXIT_SUCCESS);
}
