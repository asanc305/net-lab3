#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

int main(int argc, char *argv[])
{
  int sockfd, portno, n, seq, file;
  struct sockaddr_in serv_addr, clt_addr; 
  socklen_t addrlen;
  char rbuffer[1056] ;
  char sbuffer[64];
  struct sockaddr* address ;
  char seqnumber [11] ;
  char filesize [11] ;
  char data [1024] ;
  mode_t mode = S_IRUSR | S_IWUSR | S_IRGRP | S_IROTH ;  

  if(argc != 3) 
  { 
    printf("Usage: %s <port> <filename>\n", argv[0]);
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
  file = open( argv[2], O_WRONLY | O_CREAT | O_EXCL, mode ) ;
  for(;;)
  {
    //printf("wait on port %d...\n", portno);
    memset(seqnumber, '\0', sizeof(seqnumber)) ;
    memset(filesize, '\0', sizeof(filesize)) ;
    memset(data, '\0', sizeof(data)) ;
    
    n = recvfrom(sockfd, rbuffer, 1056, 0, address, &addrlen); 
    rbuffer[n] = '\0' ;
    
    strncpy( seqnumber, rbuffer, 10 ) ;
    strncpy( filesize, rbuffer + 10, 10 ) ;
    
    //printf("Filesize %s\n", filesize) ;
    
    if ( atoi( seqnumber) == seq )
	  {
	    printf("ACK %i SENT\n", atoi(seqnumber)) ;
	    sprintf( sbuffer, "ACK%s", seqnumber ) ;
	    n = sendto(sockfd, sbuffer, sizeof(sbuffer), 0, address, addrlen) ;
	    
	    strcpy( data, rbuffer + 20 ) ;
	    write( file, data, strlen(data) );
	   
	    seq ++ ;
	  }

  }

  close(sockfd); 
  return 0;
}
