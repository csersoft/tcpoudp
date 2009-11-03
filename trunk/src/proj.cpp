#include "tou.h"
#include <fstream>

#define	MAXMSG	1024
#define ERROR	1
#define	SUCCESS	0

int main(int argc, char* argv[])
{
  int                   sockfd;
  struct addrinfo       *adinfo;
  struct sockaddr_in    cliaddr;
  struct sockaddr_in    svraddr;
  //struct in_addr        a;
  struct hostent        *he;
  char 					recvbuf[MAXMSG] = "";
  char 					sendbuf[MAXMSG] = "";
  int					n = 0;
  touMain				tm;
  ifstream 				indata;

/* Init local network status */
  struct addrinfo       curinfo;
  int                   error;
  memset(&curinfo, 0, sizeof(curinfo));
  curinfo.ai_family = AF_INET;
  curinfo.ai_socktype = SOCK_DGRAM;
  curinfo.ai_flags = AI_PASSIVE;

/* Get the parameters and decide whether it's a server or
 * client process */
  if( argc==2 && !strncmp(argv[1], "-s", 2) )
  {/* server side */
  
    /* get the local network information */
    error = getaddrinfo(NULL, "8888", &curinfo, &adinfo);
    if (0 != error){
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(error));
      printf("error in server's getaddrinfo");
    }
	
	/* socket initialization  | type: adinfo->ai_protoco*/
    if( -1 == (sockfd = tm.touSocket(adinfo->ai_family, adinfo->ai_socktype, 0)))
      printf("error in server's socket creation");
    std::cout << "SERVER socket function returned" << std::endl;

    /* bind */
    if( 0 != tm.touBind(sockfd, adinfo->ai_addr, adinfo->ai_addrlen))
      printf("error in server binding");
    std::cout << "SERVER bind function returned" << std::endl;


    printf("[Welcome to the ToU Server Mode.]\n");
	/* keep waiting for incoming connection */
	socklen_t len = (int)sizeof(cliaddr);
	if (1 == tm.touAccept(sockfd,(struct sockaddr_in*)&cliaddr , &len )) {
		std::cout << "SERVER touAccept return on SUCCESS !! " << std::endl;
	}

	/* waiting for incoming msg */
	while(1) {
		n = tm.touRecv(sockfd, recvbuf, MAXMSG, 0); 
		std::cout<< "# of bytes read: "<< n << std::endl;
		std::cout<< recvbuf <<std::endl;
		memset(recvbuf, 0, MAXMSG);
	}

  }else if ( argc==6 && !strncmp(argv[1], "-c", 2) ){
   /* client side */
    error = getaddrinfo(NULL, argv[5], &curinfo, &adinfo);
    if( 0 != error ){
      fprintf(stderr, "getaddrinfo error: %s\n", gai_strerror(error));
      printf("error in client's getaddrinfo");
    }

	/* socket |type: adinfo->ai_protocol*/
    if( -1 == (sockfd = tm.touSocket(adinfo->ai_family, adinfo->ai_socktype, 0)))
      printf("error in server's socket creation");
	  
	/* client bind with local machine */
    if( 0 != tm.touBind(sockfd, adinfo->ai_addr, adinfo->ai_addrlen))
      printf("error in client binding");
	
	//Set socket structures
	memset(&svraddr, 0, sizeof(svraddr));
	svraddr.sin_family = AF_INET;
	inet_pton(AF_INET,argv[3],&(svraddr.sin_addr));
	svraddr.sin_port = htons(atoi(argv[4]));
  
	/* connect to client server */
	if( -1 == tm.touConnect(sockfd,(struct sockaddr_in*)&svraddr,sizeof(svraddr)))
	  printf("error in client connect");
	std::cout << "Connect returns successfully" <<std::endl;
	
	/* reading file */
	indata.open(argv[2]); // opens the file
    if(!indata) { // file couldn't be opened
      cerr << "Error: file could not be opened" << endl;
      exit(1);
    }
	
	std::cout<< "Now it's reading the data from text file" << std::endl;
    indata.read(sendbuf, sizeof(sendbuf));
	while( !indata.eof() ){
		/* int sd, char *sendBufer, int len1, int flags */
	    n = tm.touSend(sockfd, sendbuf, strlen(sendbuf), 0); 
		std::cout<< "There's are "<<n<<" data has been sent"<<std::endl;
		indata.read(sendbuf, sizeof(sendbuf));
	}
	
	}else{
		std::cout<< "Format Error" <<std::endl;
	}
	indata.close();
	close(sockfd);
	return 0;	
}
