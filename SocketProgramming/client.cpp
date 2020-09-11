/*
 * Sockets client - client program to demonstrate sockets usage
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
#include <chrono>

using namespace std;
using namespace std::chrono; 

int main(int argc, char *argv[])
{
   char *serverName;
   char *serverPort;
   int repetition;
   int nbufs;
   int bufsize;
   int type;
   struct addrinfo hints;
   struct addrinfo *result, *rp;
   int clientSD = -1;

   /* Argument validation */
   if (argc != 7)
   {
      cerr << "Usage: " << argv[0] 
           << " serverName port repetition nbufs bufsize type" 
           << endl;
      return -1;
   }
   
   // set the values from the arguments
   serverName = argv[1];
   serverPort = argv[2];
   repetition = atoi(argv[3]);
   nbufs = atoi(argv[4]);
   bufsize = atoi(argv[5]);
   type = atoi(argv[6]);

   memset(&hints, 0, sizeof(struct addrinfo));
   hints.ai_family = AF_UNSPEC;			    /* Allow IPv4 or IPv6 */
   hints.ai_socktype = SOCK_STREAM;     /* TCP */
   hints.ai_flags = 0;                  /* Optional Options */
   hints.ai_protocol = 0;               /* Allow any protocol */
   int rc = getaddrinfo(serverName, serverPort, &hints, &result);
   if (rc != 0)
   {
      cerr << "ERROR: " << gai_strerror(rc) << endl;
      exit(EXIT_FAILURE);
   }

   /* Iterate through addresses and connect */
   for (rp = result; rp != NULL; rp = rp->ai_next)
   {
      clientSD = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
      if (clientSD == -1)
      {
         continue;
      }
   	
      /* A socket has been successfully created */
      rc = connect(clientSD, rp->ai_addr, rp->ai_addrlen);
      if (rc < 0)
      {
         cerr << "Connection Failed" << endl;
         close(clientSD);
         return -1;
      }
      else // success
      {
         break;
      }
   }

   if (rp == NULL)
   {
      cerr << "No valid address" << endl;
      exit(EXIT_FAILURE);
   }
  
   freeaddrinfo(result);
   
   // first send number of repetitions to server
   write(clientSD, &repetition, sizeof(repetition));

   // data buffer to send to server 
   char databuf[nbufs][bufsize];
   for (int i = 0; i < nbufs; i++)
      for (int j = 0; j < bufsize; j++)
         databuf[i][j] = 'x';

   double elapsedMicroseconds;
   auto start = steady_clock::now();     // start timer

   switch (type) 
   {
      case 1: // Multiple writes
         {
            for (int i = 1; i <= repetition; i++)
               for (int j = 0; j < nbufs; j++) 
                  write(clientSD, databuf[j], bufsize);
            break;
         }
      
      case 2: // writev
         {
            struct iovec vector[nbufs];
            for (int j = 0; j < nbufs; j++) 
            {
               vector[j].iov_base = databuf[j];
               vector[j].iov_len = bufsize;
            }

            for (int i = 1; i <= repetition; i++)
               writev(clientSD, vector, nbufs);             

            break;
         }
      
      case 3: // single write
         {
            for (int i = 1; i <= repetition; i++)
               write(clientSD, databuf, nbufs * bufsize);

            break;
         }
      
      default:  // invalid type
         cout << "Type not supported: " << type << endl;
         return -1;
   }
  
   // read the numbre of server reads made to read the sent databuf
   int readCount = 0;
   read(clientSD, &readCount, sizeof(readCount));
   
   auto end = steady_clock::now();       // end timer
   elapsedMicroseconds = duration_cast<microseconds>(end - start).count();

   // write the output to console
   cout << "Test " << type 
        << ": time = " << elapsedMicroseconds << " usec"
        << ", #reads = " << readCount 
        << ", throughput = " << (nbufs * bufsize * repetition * .000000008) / (elapsedMicroseconds * 0.000001) << " Gbps"
        << endl << endl;

   // close the connection
   close(clientSD);
   return 0;
}