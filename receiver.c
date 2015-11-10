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
  int sockfd, portno, n, seq, file, fsize, recvd, retrans ;
  struct sockaddr_in serv_addr, clt_addr; 
  socklen_t addrlen;
  char rbuffer[1056] ;
  char sbuffer[64];
  struct sockaddr* address ;
  char seqnumber [11] ;
  char filesize [11] ;
  char datasize [11] ;
  char data [1024] ;
  fd_set readset ;
  struct timeval tv ;
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
  
  memset(seqnumber, '\0', sizeof(seqnumber)) ;
  memset(filesize, '\0', sizeof(filesize)) ;
  memset(datasize, '\0', sizeof(datasize)) ;
  memset(data, '\0', sizeof(data)) ;

  retrans = 0 ;
  seq = 0 ;
  recvd = 0 ;
  file = open( argv[2], O_WRONLY | O_CREAT | O_EXCL, mode ) ;
  
  do
  {
    n = recvfrom(sockfd, rbuffer, 1056, 0, address, &addrlen); 
    rbuffer[n] = '\0' ;
    
    strncpy( seqnumber, rbuffer, 10 ) ;
    strncpy( filesize, rbuffer + 10, 10 ) ;
    fsize = atoi(filesize) ;
    
    if ( atoi( seqnumber) == seq )
	  {
	    sprintf( sbuffer, "ACK%s", seqnumber ) ;
	    n = sendto(sockfd, sbuffer, sizeof(sbuffer), 0, address, addrlen) ;
	    
	    strncpy( datasize, rbuffer + 20, 10 ) ;
	    strcpy( data, rbuffer + 30 ) ;
	    write( file, data, atoi(datasize) );
	    recvd += atoi(datasize) ;
	    if (recvd == fsize) retrans = 1 ;

	    seq ++ ;
	  }
	  else
	  {
	    sprintf( sbuffer, "ACK%010d", seq-1 );
	    sendto(sockfd, sbuffer, sizeof(sbuffer), 0, address, addrlen) ;
    }
    
    printf("\rProgress %.0f%%", ((double)recvd / (double)fsize) *100 ) ;
    fflush(stdout) ;
    
    if ( retrans == 1 )
    {
      FD_ZERO(&readset) ;
	    FD_SET(sockfd, &readset) ;

	    tv.tv_sec = 60 ;
	    tv.tv_usec = 0 ;
	  
	    n = select(sockfd+1, &readset, NULL, NULL, &tv) ;
	    
	    if (n==0)
	    {
	      retrans = 0 ;
	      printf("\n") ;
	    }
    }
    
  }while( recvd != fsize || retrans ) ;

  close(sockfd); 
  return 0;
}
