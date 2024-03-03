/**  @file sender.c
 *
 *  @brief Client side for a more reliable file transfer using UDP sockets. 
 *
 *  @author Ana Bandari (abandari)
 *  @author Dajeong Kim (dkim2)
 * 
 *  @bug When sending two competing UDP protocols in a lossy/noisy channel, the second socket entering experiences delays and a greater timeout. Works fine for no loss. 
 *  @bug Only accepts IPV4 Addresses, in whatever form they may be. Not meant for IPV6. 
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


/*   Defining Global Variables   */
#define max_payload_size 1024 /// The maximum payload size sent over through the socket. 
#define max_data_size 1018 /// The maximum payload size subtracted by the 6 byte header. 

/** @brief rsend() sends data reliably using UDP Sockets
 * 
 *  Inputs: hostname, hostUDP port, filename, bytesToTransfer
 *  Outputs: Void 
 *
 *   Sender Algorithm Skeleton: 
 *        - Read from File (raw data).
 *        - Splice the file into sendable bits.
 *        - Create socket.
 *        - Send the file bits over through the socket.
 *        - Check for ack and increase index, repeat if nack received. 
 *        - Terminate connection and close socket and file. 
 *
 *  @param hostname The hostname can be an IP Address or a fully-qualified name.
 *  @param hostUDPport The port which you are sending data over. 
 *  @param filename The a char pointer to the file you are reading from. 
 *  @param bytesToTransfer The number of bytes you want to read from filename. 
 *  @return void 
 */

void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) {

    /// Initializing a timer for measuring the total length of time the socket has been open for
    clock_t socket_open_time, socket_close_time; 
    double total_socket_open_time;  
    
    /// Initalizing file I/O and test that the file exists
    FILE *read_file = fopen(filename, "rb");
    if (read_file == NULL){
       printf("Error! Could not open file\n");
       exit(EXIT_FAILURE); // must include stdlib.h
    }

    /// Determining the maximum value of the readfile to check that bytestotransfer doesnt exceed the file
    fseek(read_file, 0, SEEK_END);
    long readfile_size = ftell(read_file);
    fseek(read_file,0, SEEK_SET); 
    if(bytesToTransfer>readfile_size){
        printf("There are only %ld bytes in this file. Try again.\n", readfile_size);
        exit(EXIT_FAILURE);
    }

    /// Creating the socket
    int socket_desc = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if(socket_desc < 0){
        printf("Error while creating socket\n");
        exit(EXIT_FAILURE);
    }
    socket_open_time = clock();

    /// Initializing the structures needed for the socket connection
    struct sockaddr_in address, server_addr;
    unsigned int struct_length = sizeof(server_addr);
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(hostUDPport);
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    /// Here we resolve the hostname if it is fully defined
    struct hostent *hp = gethostbyname(hostname);
    char ip_address[INET_ADDRSTRLEN];
    memset(ip_address, 0, INET_ADDRSTRLEN); 
    struct in_addr **p1 = NULL;

    switch (hp->h_addrtype) {
        /// Only accepts IPV4 Addresses
        case AF_INET:
            p1 = (struct in_addr **)hp->h_addr_list;

            /// Making sure to only use the first resolved IP address
            if (p1[0] != NULL) {
                inet_ntop(AF_INET, p1[0], ip_address, INET_ADDRSTRLEN);
                printf("sending to: %s\n", ip_address);

                /// Inet_aton turns the IP address into bytes and adds it to the structure
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


    /// Initalizing a structure for timeout values
    struct timeval timeout;
    timeout.tv_sec = 0;  /// seconds
    timeout.tv_usec = 10000; /// microseconds
    struct timeval start, end;
    double elapsed_time;

    /// Utilizing the setsockopt() function to set a time out for the recvfrom
    if (setsockopt(socket_desc, SOL_SOCKET, SO_RCVTIMEO, (char*)&timeout, sizeof(timeout)) < 0) {
        perror("setsockopt failed");
        close(socket_desc);
        exit(EXIT_FAILURE);
    }
    
    printf("Socket created successfully\n");

    /// Initializing a sender buffer of max payload size which is the total data send with a header of 6 bytes
    void *sender_buffer = malloc(max_payload_size);
    memset(sender_buffer, 0, max_payload_size);
    if (sender_buffer == NULL) {
        fprintf(stderr, "Memory allocation failed for sender_buffer\n");
        exit(EXIT_FAILURE);
    }

    /// Initializing a void pointer for sender message to get raw bytes from the file (It's purpose is to point at 1018 bytes of data)
    void *readfile_data = malloc(max_data_size);
    memset(readfile_data, 0, max_data_size);
    if (readfile_data == NULL) {
        fprintf(stderr, "Memory allocation failed for readfile_data buffer\n");
        exit(EXIT_FAILURE);
    }

    /// Initializing a buffer of the maximum payload size to receieve acknowladgements from the receiver
    void *ack_buffer= malloc(max_payload_size);
    memset(ack_buffer, 0, max_payload_size);
    if (ack_buffer == NULL) {
        fprintf(stderr, "Memory allocation failed for ack_buffer\n");
        exit(EXIT_FAILURE);
    }

    /// Initallizing variables that will be used in the while loop below
    int bytesRead = 0; /// Number of bytes already read from the file
    useconds_t t = 1000; /// Time to wait in microseconds
    unsigned index = 0; /// The index of the packet. It needs to send and receive the index in order.
    int byteNumber = 0; /// Number of bytes to read in the current iteration of the while loop

    /** While the number of bytes already read is less than the number of bytes to transfer exectute the sending loop */
    while(bytesRead < bytesToTransfer) {
        /// Determine number of bytes to read based on how many unread bytes remain 
        byteNumber = (max_data_size < (bytesToTransfer - bytesRead)) ? max_data_size : (bytesToTransfer - bytesRead);

        /// Initialize void pointers to contain values of zero for every iteration of the loop
        memset(readfile_data, 0, max_data_size);
        memset(sender_buffer, 0, max_payload_size);
        memset(ack_buffer, 0, max_payload_size);

        /// Read 'byteNumber' of bytes from the read_file  
        fseek(read_file, bytesRead, SEEK_SET);
        fread(readfile_data, 1, byteNumber, read_file);
        
        /// Copy the two uint8_t values to the start of the new buffer that will be used to send data
        uint8_t ack_flag=0;
        uint8_t fin_flag=0;
        memcpy(sender_buffer, &ack_flag, 1);
        memcpy(sender_buffer+1, &fin_flag, 1);

        /// Copy the current index and the data to the file
        memcpy(sender_buffer+2, &index, 4);
        memcpy(sender_buffer+6, readfile_data, byteNumber);

        /// Slows down how fast our data is being sent
        usleep(t);
        
        /// Sends a message to the receiver 
        if(sendto(socket_desc, sender_buffer, byteNumber+6, 0, (struct sockaddr*)&server_addr, struct_length)<0){
            printf("Unable to send message\n");
        }

        /// Waits to receive an acknowlegement for the defined recvfrom timeout time specified prior
        size_t client_message = recvfrom(socket_desc, ack_buffer, max_payload_size, 0, (struct sockaddr*)&server_addr, &struct_length);  
        if (client_message < 0){
            printf("Couldn't receive\n");
        }

        /// Instantializes a variable for checking the ack_message from the received ack_buffer
        uint8_t ack_message;
        memcpy(&ack_message, ack_buffer, 1);
        
        /// Testing if the ack_message and only moving onto the next index if they match
        if(ack_message == 1){ 
            index++;  /// Incrementing index if the ack messages match
            bytesRead += byteNumber; /// Stating the bytes read from the file 
            
            /// Using a multiplicative decrease to reduce our socket waiting time. 
            if(t>0){
                t = t/2;
            }
        }

        /// If a negative acknowlegement is recieved then it will resend the current packet until it gets a positive acknowladgement 
        if(ack_message == 0){ 
            /// We implement an additive increase to reduce the sending time if there is packet loss to try and mitigate packet loss. 
            /// Note that these times are in microseconds
            if(t<1000000) {
                t += 1000;
            }
        }

        
    }

    /// Setting up all the variables needed to terminate the connection
    memset(sender_buffer, 0, max_payload_size);
    uint8_t ack_flag = 0;
    uint8_t fin_flag = 1;

    /// Raising FIN Flag HIGH
    memcpy(sender_buffer, &ack_flag, 1);
    memcpy(sender_buffer+1, &fin_flag, 1);
    
    /// Sending the FIN message over through the socket to terminate the connection
    if (sendto(socket_desc, sender_buffer, 2, 0, (struct sockaddr*)&server_addr, struct_length) < 0) {
    printf("Unable to send message\n");
    exit(EXIT_FAILURE);
    }
    
    /// Instantializing the ack_message variable outside of the while loop
    uint8_t ack_message;

    /// Waiting for a FIN ack from recevier
    while (ack_message != 1) {
        size_t client_message = recvfrom(socket_desc, ack_buffer, max_payload_size, 0, (struct sockaddr*)&server_addr, &struct_length);  
        if (client_message < 0){
            printf("Couldn't receive\n");
        }
        memcpy(&ack_message, ack_buffer, 1);
    }

    /// Closing socket and file and noting the time the socket was open for 
    close(socket_desc);
    socket_close_time = clock(); 
    fclose(read_file);
   
   total_socket_open_time = ((double) (socket_close_time - socket_open_time)) / CLOCKS_PER_SEC;
    printf("The socket has been open for: %f seconds\n", total_socket_open_time);

    elapsed_time = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
    printf("The total elapsed time is: %f seconds\n", elapsed_time);
}



/** @brief  main function made to allow for the invoking of the file transfer from the command line
 *
 * @return Returns an int of 1 if there is an error
 */
int main(int argc, char** argv) {
    int hostUDPport;
    unsigned long long int bytesToTransfer;
    char* hostname = NULL;

    if (argc != 5) {
        fprintf(stderr, "usage: %s receiver_hostname receiver_port filename_to_xfer bytes_to_xfer\n\n", argv[0]);
        exit(1);
    }

    /// Get values from commandline
    hostname = argv[1];
    hostUDPport = (unsigned short int) atoi(argv[2]);
    bytesToTransfer = atoll(argv[4]);

    /// Call sender function
   rsend(hostname, hostUDPport, argv[3], bytesToTransfer);
   return(EXIT_SUCCESS);
} 