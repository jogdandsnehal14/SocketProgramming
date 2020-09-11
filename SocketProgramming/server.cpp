/*
 * Sockets server - server program to demonstrate sockets usage
 * University of Washington Bothell
 * CSS503: System Programming
 * Program 4: Socket Programming
 * Submitted by Snehal Jogdand
 */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <netinet/tcp.h>
#include <sys/uio.h>
#include <stdio.h>
#include <stdlib.h>
#include <iostream>
using namespace std;

const int BUFFSIZE = 1500;
const int NUM_CONNECTIONS = 5;

// The thread function to service client request
void *connectionHandler(void *socketDesc)
{
    // Get the socket descriptor
   int newSD = *(int*)socketDesc;
   char databuf[BUFFSIZE];
   bzero(databuf, BUFFSIZE);

   int bytesRead = 0;
   
   // Receive a message by the client with the number of iterations to perform 
   int repetition = 0;
   bytesRead = read(newSD, &repetition, sizeof(repetition));
   if (bytesRead <= 0) {
     cout << "Error reading repetition";
     pthread_exit(0);
   }
   
   bytesRead = 0;

   // Read from the client the appropriate number of iterations of BUFSIZE amounts of data
   int readCount = 0;
   for (int i = 1; i <= repetition; i++) {
      // repeat calling read until we have read all the data
      while (bytesRead < BUFFSIZE * repetition) {
          bytesRead += read(newSD, databuf, BUFFSIZE);
          readCount += 1;
      }
   }
   
   // Send the number of read() calls made as an acknowledgment to the client
   int bytesWritten = write(newSD, &readCount, sizeof(readCount));
   if (bytesWritten <= 0) {
     cout << "Error writing read count";
     pthread_exit(0);
   }

   // Close this connection.
   close(newSD);

   // Terminate the thread
   pthread_exit(0);
}

int main(int argc, char *argv[])
{
   char *serverName;

   /* Argument validation */
   if (argc != 2)
   {
      cerr << "Usage: " << argv[0] 
           << " serverName port" 
           << endl;
      return -1;
   }
   
   /*  build address */
   int port = atoi(argv[1]);
   sockaddr_in acceptSocketAddress;
   bzero((char *)&acceptSocketAddress, sizeof(acceptSocketAddress));
   acceptSocketAddress.sin_family = AF_INET;
   acceptSocketAddress.sin_addr.s_addr = htonl(INADDR_ANY);
   acceptSocketAddress.sin_port = htons(port);

   /* open socket */
   int serverSD = socket(AF_INET, SOCK_STREAM, 0);
   const int on = 1;
   setsockopt(serverSD, SOL_SOCKET, SO_REUSEADDR, (char *)&on, sizeof(int));
  
   /* bind socket to port */
   int rc = bind(serverSD, (sockaddr *)&acceptSocketAddress, sizeof(acceptSocketAddress));
   if (rc < 0)
   {
      cerr << "Bind failed" << endl;
   }

   /* listen and accept (max 5 connections allowed) */
   if (listen(serverSD, NUM_CONNECTIONS) < 0)
   {
      cout << "Error: cannot listen on port " << port << endl;
      return -1;
   }

   pthread_t thread;
   while (true)
   {
   	  /* accept incoming connections */
      sockaddr_in newSockAddr;
      socklen_t newSockAddrSize = sizeof(newSockAddr);
      int newSD = accept(serverSD, (sockaddr *) &newSockAddr, &newSockAddrSize);
      if (newSD > 0)
      {
        /* start a new thread but do not wait for it */
        pthread_create(&thread, 0, connectionHandler, (void*) &newSD);
        pthread_detach(thread);
      } 
      else 
      {
        cout << "Error accepting new socket";
      }
   }
 
   close(serverSD);
   return 0;
}