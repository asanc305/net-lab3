#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int main(int argc, char *argv[])
{
  int sockfd, portno, n, seq;
  struct sockaddr_in serv_addr, clt_addr; 
  socklen_t addrlen;
  char rbuffer[1024] ;
  char sbuffer[32];
  struct sockaddr* address ;
  char *token ;
  const char del[2] = " " ;
  char seqnumber [10] ;
  

  if(argc != 2) 
  { 
    printf("Usage: %s <port>\n", argv[0]);
    return 1;
  } 
  portno = atoi(argv[1]);

  sockfd = socket(AF_INET, SOCK_DGRAM, 0); 
  if(sockfd < 0) return -1 ;
  printf("create socket...\n");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr.s_addr = INADDR_ANY;
  serv_addr.sin_port = htons(portno);

  if(bind(sockfd, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) < 0) 
    return -1 ;
  printf("bind socket to port %d...\n", portno);
  addrlen = sizeof(clt_addr); 
  address = (struct sockaddr*) &clt_addr ;

  seq = 0 ;
  for(;;)
  {
    //printf("wait on port %d...\n", portno);
    n = recvfrom(sockfd, rbuffer, 1024, 0, address, &addrlen); 
    
    sprintf( seqnumber, "%i", seq ) ;
    token = strtok(rbuffer, del) ;    
    if ( strcmp(token, seqnumber) == 0 )
	  {
	    printf("ACK SENT!!!!!\n") ;
	    sprintf( sbuffer, "ACK %i", seq ) ;
	    n = sendto(sockfd, sbuffer, sizeof(sbuffer), 0, address, addrlen);
	    seq ++ ;
	  }

  }

  close(sockfd); 
  return 0;
}
