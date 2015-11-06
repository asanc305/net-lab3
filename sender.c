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

off_t size(const char *filename) {
    struct stat st; 

    if (stat(filename, &st) == 0)
        return st.st_size;

    return -1; 
}

int main(int argc, char* argv[])
{
  int sockfd, portno, n, base, next, j, i ;
  size_t x ;
  off_t bytes_sent, fsize, temp ;
  struct hostent* server;
  struct sockaddr_in serv_addr;
  struct sockaddr* to ;
  socklen_t addrlen;
  char packets [window_size] [packet_size] ;
  char buffer[1056] ;
  char smallbuffer[64] ;
  struct stat f_stat ;
  fd_set readset ;
  struct timeval tv ;
  const char del[2] = " " ;
  char * token ;
  char seqnumber [11] ;
  char header [20] ;
  FILE *file ;
  

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
  if(sockfd < 0) return -1 ;
  printf("create socket...\n");

  memset(&serv_addr, 0, sizeof(serv_addr));
  serv_addr.sin_family = AF_INET;
  serv_addr.sin_addr = *((struct in_addr*)server->h_addr);
  serv_addr.sin_port = htons(portno);
  addrlen = sizeof(serv_addr);
  to = (struct sockaddr*)&serv_addr ; 
  
  //connected sendto recvfrom
  
  //initialize array
  for(i = 0; i<= window_size -1; i++) memset(packets[i], '\0', packet_size) ;
  
  file = fopen(argv[3], "rb") ;
  if ( file == NULL) printf("error\n") ;

  fsize = size(argv[3]) ;

  bytes_sent = 0 ;
  j = 0 ;
  base = 0 ;
  next = 0 ;
  temp = 0 ;
  
  while( fsize > bytes_sent )
  {
    while( next < base + window_size && temp < fsize )
    {
      memset(buffer, '\0', sizeof(buffer)) ;
      
      x = fread( packets[j], 1024, 1, file ) ;
      printf("Read %zu bytes\n", x) ;
      packets[j][n] = '\0' ;
      printf("Size of packet: %zu\n", strlen(packets[j]) );
      
      sprintf(buffer, "%010d%010jd%s", next, fsize, packets[j]) ; 
      
      i = sendto(sockfd, buffer, sizeof(buffer), 0, to, addrlen);
      //printf("Sent %i bytes\n", i) ;
      next++ ;
      j++ ;
      temp += x ;
      //return -1 ;
    }

    if (temp == fsize) next-- ;
    
    do
    {
      FD_ZERO(&readset) ;
	    FD_SET(sockfd, &readset) ;
	  
	    tv.tv_sec = 0 ;
	    tv.tv_usec = 10 ;
	  
	    n = select(sockfd+1, &readset, NULL, NULL, &tv) ;

	    if (n > 0) 
	    {
	      n = recvfrom(sockfd, smallbuffer, sizeof(smallbuffer), 0, to, &addrlen) ; 
	      smallbuffer[n] = '\0' ;
	      
	      memset(seqnumber, '\0', sizeof(seqnumber)) ;
	      strncpy( seqnumber, smallbuffer+3, 10 ) ;
	      
        if ( base == atoi(seqnumber) )
        {
          //printf("ACK RECEIVED FOR SEQ %i base: %i next: %i\n", atoi(seqnumber), base, next) ;
          if ( (base % window_size) == 0 ) j = 0 ;
          base++ ;
          bytes_sent += 1024 ;
	      }
        else printf("WRONG ACK RECEIVED\n") ;
	    }
    } while( n >= 1 ) ;
    
    
    
    if ( base != next )
    {
      n = base ;
      j = n % window_size ;
      //printf("Resending base:%i next:%i j:%i\n", base, next, j ) ;
      while (n <= next)
      {
        sprintf(buffer, "%010d%010jd%s", n, fsize, packets[j]) ;  

        sendto(sockfd, buffer, sizeof(buffer), 0, to, addrlen);
      
        n++ ;
        j = (j + 1) % window_size ;
      }
      j = 0;
    }
  } 
  
  close(sockfd);
  return 0;
}
