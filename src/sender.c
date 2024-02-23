/** @file sender.c
 *  @brief Client side for a more reliable file transfer using UDP Protocol. 
 *
 *  The client/sender part of the reliable file transfer using UDP. 
 *
 *  @author Ana Bandari (abandari)
 *          Dajeong Kim 
 *  @bug No known bugs.
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
    Outputs:

    Sender Algorithm Skeleton: 
        - Read from File 
        - Splice the file into sendable bits
        - 

*/


void rsend(char* hostname, 
            unsigned short int hostUDPport, 
            char* filename, 
            unsigned long long int bytesToTransfer) {

        /* Open the file */
    FILE *read_file = fopen(filename, "rb");
    if (read_file == NULL){
       printf("Error! Could not open file\n");
       exit(EXIT_FAILURE); // must include stdlib.h
    }

    


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
   rsend(hostname, hostUDPport, filename, bytesToTransfer);
   return (EXIT_SUCCESS);
} 
