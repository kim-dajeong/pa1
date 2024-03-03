/**
 * @file receiver.c
 *  @brief Server functionality for a more reliable file transfer using UDP sockets. 
 *
 *  @author Ana Bandari (anabandari)
 *  @author Dajeong Kim (dkim2)
 * 
 *  @bug No writeRate implemented
 * 
 */



/*   Includes   */
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



/// the total amount of data we want to send over our UDP socket payload in one packet
#define max_payload_size 1024 


/**
 * @brief receiver function for receiving data packets and sending acknowledgements back to client
 * 
 * @param myUDPport hostport
 * @param destinationFIle pointer to destinationFile where received ata will be written
 * @param writeRate not used
 * 
 * @return void
 * 
 * 
*/
void rrecv(unsigned short int myUDPport, 
            char* destinationFile, 
            unsigned long long int writeRate){
    
    ///  Initalizing file I/O and test that the file exists, opening in write only mode
    FILE *write_file = fopen(destinationFile, "wb");
    if (write_file == NULL){  
        printf("Error! Could not open file\n");
        exit(EXIT_FAILURE); 
        }

    /// Initalizing address struct and the structure of the clients address for receiving
    struct sockaddr_in address, client_addr;
    address.sin_family = AF_INET;
    address.sin_port = htons(myUDPport);
    address.sin_addr.s_addr = htonl(INADDR_ANY);   
    unsigned int client_struct_length = sizeof(client_addr);


    /// Create UDP socket and check it exists
 
    int socket_desc = socket(AF_INET, SOCK_DGRAM, 0);
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        exit(EXIT_FAILURE);
    }

    /// Bind socket to the receive address
    if(bind(socket_desc, (struct sockaddr*)&address, sizeof(address)) < 0){
        printf("Couldn't bind to the port\n");
        exit(EXIT_FAILURE);
    }

    /// acknowledgement flag value holder, initialized to 0
    uint8_t ack = 0;
    /// index for data packets, initialized to 0
    int index = 0;
    /// pointer to memory for storing data to send
    void* sendmemorypointer;
    /// pointer to memory for storing data received
    void* receivedmemorypointer;
    /// pointer to address of acknowledgement flag
    void* ackpointer;
    /// pointer to address of finish flag
    void* finpointer;
    /// pointer to address of data
    void* datapointer;
    /// pointer to address of data index
    void* indexpointer;

    /// Set size of buffer and assign memory block to pointers
    size_t buffer_size = 4000; 
    receivedmemorypointer = malloc(buffer_size);
    sendmemorypointer = malloc(buffer_size);

    /// Set acknowledgement flag pointer to first byte of memory storing data to send
    ackpointer = sendmemorypointer;

    /// Set pointers to respective byte of memory storing received data
    finpointer = ((char*)receivedmemorypointer + 1);
    indexpointer = ((char*)receivedmemorypointer + 2);
    datapointer = ((char*)receivedmemorypointer + 6);

    /// While loop to continue receiving data and sending acknowledgements until a finish flag is received
    while(1){ 

        /// reset memory blocks to null
        memset(sendmemorypointer, 0, buffer_size);
        memset(receivedmemorypointer, 0, buffer_size);

        /// Wait for sender to send message, and store size of message in variable client_message
        size_t client_message = recvfrom(socket_desc, receivedmemorypointer, max_payload_size, 0, (struct sockaddr*)&address, &client_struct_length);  

        /// Variable to hold value of finish flag received
        uint8_t fincomp;
        /// Variable to hold value of index received
        uint32_t indexcomp;

        /// Copy data stored at address of finpointer and indexpointer into variable to use for comparisons
        memcpy(&fincomp, (uint8_t*)finpointer, 1);
        memcpy(&indexcomp, (uint8_t*)indexpointer, 4);

        /// Check if the recvfrom function received has size greater than 0
        if (client_message < 0){

            /// Set acknowledgement to 0, indicating a nack flag meaning data transfer was not successful
            /// Copy value of acknowledgement flag into memory
            ack = 0;
            memcpy(ackpointer, &ack, 1);


            /// Send nack flag to sender
            sendto(socket_desc, sendmemorypointer, buffer_size, 0, (struct sockaddr*)&address, client_struct_length);

        }
        /// Check if value of finish flag is set to 1, in which case the while loop must be exited
        else if (fincomp == 1) {

            /// Set acknowledgement flag high, to indicate that the finish flag was received to the sender
            ack = 1;
            /// Copy value of acknowledgement flag into memory
            memcpy(ackpointer, &ack, 1);

            /// Send the acknowledgement to the sender, then exit the while loop
            sendto(socket_desc, sendmemorypointer, buffer_size, 0, (struct sockaddr*)&address, client_struct_length);
            break;

        } 
        /// Check if the index of the data is equal to the index count of the receiver
        else if(indexcomp == index) {

            /// Write the data from the data portion of the received memory
            size_t written = fwrite(datapointer, 1, client_message-6, write_file);

            /// Check if write was successful
            if (written < client_message-6) {
                printf("Error during writing to file!");
            }

            /// Set acknowledgement flag high, to indicate that the correct index was received to the sender
            ack = 1;
            /// Copy value of acknowledgement flag into memory
            memcpy(ackpointer, &ack, 1);

            /// Send the acknowledgement to the sender
            sendto(socket_desc, sendmemorypointer, buffer_size, 0, (struct sockaddr*)&address, client_struct_length);

            /// Increment the index keeping count of how many successful data packets were received and written to the destination
            index++;

        }
        /// If none of the above conditions were met, assume that the index of the data packet was incorrect or no data was received during the bounds of the timeout period
        else {

            /// Set acknowledgement flag low, to indicate that an incorrect index was received to the sender
            ack = 0;
            /// Copy value of acknowledgement flag into memory
            memcpy(ackpointer, &ack, 1);
            /// Copy value of current index to be received into memory
            memcpy(indexpointer, &index, 4);

            /// Send the nack to the sender
            sendto(socket_desc, sendmemorypointer, buffer_size, 0, (struct sockaddr*)&address, client_struct_length);
        }

        /// Set the acknowledgement flag low
        ack = 0;

        /// This is the end of the while loop. 
    
    }

    /// Close the destination file opened for writing 
    fclose(write_file);
    /// Close the socket conneciton
    close(socket_desc);

    /// Write rate not implemented
    if(writeRate != 0){
        printf("Error Have not yet implemented Task 8\n");
        exit(EXIT_FAILURE);
    }

}

/**
 * @brief main function to call receiver function and pass UDPport and destination filename 
 * 
 * @param arcg argument count, indicating number of arguments passed from the command line
 * @param argv argument vector, pointing to an array of strings, each of which contains an argument passed from the command line
 * 
 * @return 0
 * 
*/
int main(int argc, char** argv){

    ///initialize variable to hold udpPort name passed from the command line
    unsigned short int udpPort;

    /// Check if all three arguments were passed from the command line
    if (argc != 3) {
        fprintf(stderr, "usage: %s UDP_port filename_to_write\n\n", argv[0]);
        exit(1);
    }

    /// Parse the command-line arguments
    udpPort = (unsigned short int) atoi(argv[1]);
    char* destinationFile = argv[2];

    /// Call the rrecv function with the provided arguments, passing 0 for writeRate
    rrecv(udpPort, destinationFile, 0);

    /// return 0 and end
    return 0;
}
