# PA1 - Reliable UDP Data Transfer (Reinventing the Wheel At Its Finest)

How to Run 
 - Use the makefile to Make all
 - Run the sender on one terminal (Be sure to specify a file to open (we have provided a test file called "readfile"))
 - Run the receiver on another terminal


**An Overview of Our Approach**

Our approach focuses on sending packets in order and as such, does not worry about duplicate acknowledgments or out of order packets. 

Sender 
1. Create Socket 
2. Get IP Address from Hostname
3. Read "bytestoTransfer" from the file
4. Send data in 1018 byte packet over a socket with a payload of 1024 bytes
5. Check for acknowledgments and move on
6. Resend packets if received out of order
7. Send a termination message
8. Wait for ack from receiver then end the connection

Note: All the conjestion control takes place on the sender side where we use a AIMD similar process to slow down or speed up the datarate of our sender. 
eg. if negative-ack received -> additive increase (slow down)
    if postive-ack received  -> multiplicative decrease (speed up)

Receiver 
1. Create Socket
2. Bind to Port
3. Listen for messages
4. Write to file and send acknowledgments  
5. If a terminate message is sent stop listening and send a termination acknowledgment 
