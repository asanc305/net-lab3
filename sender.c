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

void makeHeader(char * header, int seq, off_t size)
{
  sprintf( header, "%i!E!%jd", seq, size ) ;  
}

int main(int argc, char* argv[])
{
  int sockfd, portno, n, file, base, next, j, i, temp ;
  off_t bytes_sent, fsize ;
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
  temp = 0 ;
  
  while( fsize > bytes_sent )
  {
    while( next < base + window_size && temp < fsize )
    {
      memset(buffer, '\0', sizeof(buffer)) ;
      
      n = read( file, packets[j], packet_size ) ;
      packets[j][n] = '\0' ;
      
      sprintf(buffer, "%010d%010jd%s", next, fsize, packets[j]) ; 

      //printf("Read %i bytes\n", n) ;
      i = sendto(sockfd, buffer, sizeof(buffer), 0, to, addrlen);
      //printf("Sent %i bytes\n", i) ;
      next++ ;
      j++ ;
      temp += n ;
      //return 1 ;
    }

    if (temp == fsize) next-- ;
    /*
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
          printf("ACK RECEIVED FOR SEQ %i base: %i next: %i\n", atoi(seqnumber), base, next) ;
          if ( (base % window_size) == 0 ) j = 0 ;
          base++ ;
          bytes_sent += 1024 ;
	      }
	    }
    } while( n >= 1 ) ;*/
    
    
    /*
    if ( base != next )
    {
      n = base ;
      j = n % window_size ;
      printf("Resending base:%i next:%i j:%i\n", base, next, j ) ;
      while (n <= next)
      {
        sprintf(buffer, "%010d%010jd%s", n, fsize, packets[j]) ;  

        sendto(sockfd, buffer, sizeof(buffer), 0, to, addrlen);
      
        n++ ;
        j++ ;
      }
      j = 0;
    }*/
  } 
  
  close(sockfd);
  return 0;
}
