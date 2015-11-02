#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <fcntl.h>

const int window_size = 100 ;
const int packet_size = 1024 ;

void syserr(char* msg) { perror(msg); exit(-1); }

int main(int argc, char* argv[])
{
  int sockfd, portno, n, file, base, next, j, i, resend ;
  off_t bytes_sent, fsize ;
  struct hostent* server;
  struct sockaddr_in serv_addr;
  struct sockaddr* to ;
  socklen_t addrlen;
  int window [window_size] ;
  char packets [window_size] [packet_size] ;
  char buffer[1056] ;
  struct stat f_stat ;
  fd_set readset ;
  struct timeval tv ;
  const char del[2] = " " ;
  char * token ;
  char seqnumber [10] ;
  

  if(argc != 4) {
    printf("Usage: %s <reciever ip> <port> <filename>\n", argv[0]);
    return -1;
  }
  server = gethostbyname(argv[1]);
  
  if(!server) 
  {
    printf("ERROR: no such host: %s\n", argv[1]);
    return -2;
  }
  portno = atoi(argv[2]);
  
  sockfd = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
  if(sockfd < 0) syserr("can't open socket");
  printf("create socket...\n");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
  serv_addr.sin_port = htons(portno);
  addrlen = sizeof(serv_addr);
  to = (struct sockaddr*)&serv_addr ; 
  
  //connected sendto recvfrom
  
  //initialize array
  for(i = 0; i<= window_size -1; i++) window[i] = 0 ;
  for(i = 0; i<= window_size -1; i++) packets[i][0] = '\0' ;
  
  file = open(argv[3], O_RDONLY) ;
  if ( file == -1) printf("error\n") ;
  fstat(file, &f_stat) ;
  fsize = f_stat.st_size ;
  bytes_sent = 0 ;
  printf("%jd\n", fsize) ;
  j = 0 ;
  base = 0 ;
  next = 0 ;
  
  while( fsize > bytes_sent )
  {
    while( next < base + window_size - 1 )
    {
      printf("ASSEMBLING PACKET # %i !!!\n", next) ;
      n = read( file, packets[j], packet_size ) ;
      packets[j][n] = '\0' ;
      sprintf(buffer, "%i %jd %s", next, fsize, packets[j]) ; 

      n = sendto(sockfd, buffer, sizeof(buffer), 0, to, addrlen);
      //printf("ASSEMBLING PACKET # %i !!!\n", next) ;
      next++ ;
      j++ ;
      
    }
    
    printf("HEERRRR\n") ;
    do
    {
      FD_ZERO(&readset) ;
	    FD_SET(sockfd, &readset) ;
	  
	    tv.tv_sec = 0 ;
	    tv.tv_usec = 10 ;
	  
	    n = select(sockfd+1, &readset, NULL, NULL, &tv) ;

	    if (n > 0) 
	    {
	      n = recvfrom(sockfd, buffer, 64, 0, to, &addrlen) ; 
	      buffer[n] = '\0' ;
	      token = strtok(buffer, del) ;
	      
	      if ( strcmp( token, "ACK" ) == 0 )
	      {
	        sprintf( seqnumber, "%i", base ) ;
	        token = strtok(NULL, del) ;
	        if ( strcmp(token, seqnumber ) == 0)
	        {
	          printf("ACK RECEIVED FOR SEQ %s\n", seqnumber) ;
	          if ( (base % window_size) == 0 ) j = (j + 1) % window_size ;
	          base++ ;
	          bytes_sent += 1024 ;
	        }
	      }
	    }
    } while( n >= 1 ) ;
    
    printf("GOT HERE BASE: %i NEXT: %i\n", base, next) ;
    if ( base != next )
    {
      n = base ;
      j = n % window_size ;
      while (n <= next)
      {
        sprintf(buffer, "%i %jd %s", next, fsize, packets[j]) ; 

        sendto(sockfd, buffer, sizeof(buffer), 0, to, addrlen);
      
        n++ ;
        j++ ;
      }
    }
  } 
  
  close(sockfd);
  return 0;
}
