/** @file sender.c
 *  @brief Client side for a more reliable file transfer using UDP Protocol. 
 *
 *  The client/sender part of the reliable file transfer using UDP. 
 * 
 *   HARD CODING THE READFILE JUST WANT TO TEST THE FUNCTION OF THE STRUCTURED PACKETS!!!!!!
 *
 *  @author Ana Bandari (abandari)
 *          Dajeong Kim 
 * 
 *  @bug Steps: 
 *        1. Only works for small strings, test with large chars/bytes. 
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

#include <math.h>

/*!
Sender Notes
    Inputs: hostname, hostUDP port, filename, 
    Outputs:

    Sender Algorithm Skeleton: 
        - Read from File 
        - Splice the file into sendable bits
        - create socket 
        - send the file bits over through the socket

    Test Commands:
    wget -O sender.c https://raw.githubusercontent.com/kim-dajeong/pa1/ana-struct-test/src/sender.c
    wget -O readfile.txt https://raw.githubusercontent.com/kim-dajeong/pa1/ana-struct-test/src/readfile
    gcc -o sender sender.c
    ./sender hostname 8000 readfile bytes to transfer

*/

#define max_buffer_size 1024 //Is equal to the bytes of the max UDP data payload we'll use

struct senderPacket     
  {
       char messageType[1];                           /* the type of message being sent  */
       // mesage type 1 ==data,  2== ack,  3==nack, 4 ==fin 
       char packetOrder[1];                           /* the order of packet being sent  */
       // whatever order you so choose idk what to do past 9??? 
       char packetData[1023];                           /* Data from the readFile in 1022 byte bites!*/
  };

void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) {

    struct senderPacket newpack;
    // Initalizing file I/O and test that the file exists
    FILE *read_file = fopen(filename, "rb");
    if (read_file == NULL){
       printf("Error! Could not open file\n");
       exit(EXIT_FAILURE); // must include stdlib.h
    }

    //initallize array for sender message
    char sender_message[2000];

    int total_num_packets = ceil(1.0 * bytesToTransfer / max_buffer_size);
    printf("The total number of packets needed to send your message is: %d", total_num_packets);
    
    int packet_index = 0;

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


    while(packet_index <= total_num_packets){
    // Move the file pointer to the desired position
    if (fseek(read_file, packet_index*1022, SEEK_SET) != 0) { // SEEK_SET sets the offset from the beginning of the file
        perror("Error seeking in file");
        fclose(read_file);
        exit(EXIT_FAILURE);
    }
    size_t bytes_read;

    // Read data from the current position
    bytes_read = fread(newpack.packetData, 1, sizeof(buffer) - 1, read_file);
    if (bytes_read == 0) {
        if (feof(read_file)) {
            printf("Reached end of file\n");
        } else {
            perror("Error reading file");
        }
    } else {
        // Null-terminate the buffer to use it as a string if needed
        buffer[bytes_read] = '\0';
        printf("Read %zu bytes: %s\n", bytes_read, buffer);
    }
    
    strcat(sender_message, newpack.packetData);
    
    // Send the message to server:
    if(sendto(socket_desc, sender_message, strlen(sender_message), 0,(struct sockaddr*)&server_addr, struct_length) < 0){
        printf("Unable to send message\n");
        exit(EXIT_FAILURE);
    }

    printf("Socket sent successfully\n");
    packet_index++; 
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
