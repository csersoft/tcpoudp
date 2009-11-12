/***************************************************
 * This is ToU library header file. Programs which 
 * want to use ToU library functions should include
 * this header file.
 **************************************************/
 
/******************************************************
 * Type define here
 * ***************************************************/
//Types
typedef	unsigned long 	u_long;
typedef	unsigned short	u_short;
typedef unsigned char	u_char;

//Timers
#define TOUT_TIMERS		4
#define TOUT_REXMIT		0
#define TOUT_PERSIST		1
#define TOUT_KEEP		2
#define TOUT_2MSL		3

//Ethernet 1500-20-24
#define TOU_MSS			100 
//Status
#define TOUS_CLOSED		0
#define TOUS_LISTEN		1
#define TOUS_SYN_SENT		2
#define TOUS_SYN_RECEIVED	3
#define TOUS_ESTABLISHED	4
#define TOUS_CLOSE_WAIT		5
#define TOUS_FIN_WAIT_1		6
#define TOUS_CLOSING		7
#define TOUS_LAST_ACK		8
#define TOUS_FIN_WAIT_2		9
#define TOUS_TIME_WAIT		10

/***************************************************
 * Include from STD library
 **************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sstream>

#include<vector>
/***************************************************
 * Include from BOOST library
 **************************************************/

/***************************************************
 * Include from self-define header
 **************************************************/
#include "timer.h"				//timer library
#include "trace.h"
//#include "touControlBlock.h"
#include "toucong.h"			//congestion
								//ToU Control Block
#include "circularbuffer.h"
#include "touheader.h"			//touheader


extern FILE *_fptrace;
extern boost::mutex soctabmutex;

/******************************************************
 * socket table
 * ***************************************************/

class sockTb {
  public:
    touCb	tc;			//tcp control block
    int		sockd;		//socket file descriptor
	int     sockstate;  //SOCK_CREATED, BIND, LISTEN, CONNECT, ESTABLISHED, TERMINATING
    u_short sport;
    u_short dport;      //destination port
    string  sip;
    string  dip;        //destination ip 
    int 	cid;        //connection id. probably dont need it
    int     tcpstate;   //state in which the connection is   
    CircularBuffer 	CbSendBuf;
    CircularBuffer 	CbRecvBuf;
	ssca	*sc;		//congestion control class

	/* TEST */
	int		ackcount;/* FOR TEST ONLY */

    sockTb() {
      /*
	  sip = (char*)malloc(sizeof(char)*30);
	  dip = (char*)malloc(sizeof(char)*30);
	  */
      CbSendBuf.setSize(TOU_MAX_CIRCULAR_BUF);
	  CbRecvBuf.setSize(TOU_MAX_CIRCULAR_BUF);
	  sc = new ssca(&tc); 
	}

	~sockTb() {
	  /*
	  free(sip);
	  free(dip);
	  */
	  delete sc;
	}
	void printall() {
	cout<<"sockd  : "<<sockd<<" sport :"<<sport<<" dport :"<<dport <<endl;
	cout<<"sip    : "<<sip<<" dip : "<<dip <<endl;
	cout<<"cc_state: "<<tc.cc_state<<endl;
	cout<<"snd_una: "<<tc.snd_una<<endl;
	cout<<"snd_nxt: "<<tc.snd_nxt<<endl;
	cout<<"snd_cwnd:"<<tc.snd_cwnd<<endl;
	cout<<"snd_awnd:"<<tc.snd_awnd<<endl;
	cout<<"snd_ssthresh:"<<tc.snd_ssthresh<<endl<<endl;
	}

};

extern vector<sockTb*> SS;
//int cid_ =0;
class sockMng {
  public:
    struct sockTb* getSocketTable(int);
    void setSocketTable(struct sockaddr_in *, int);
    void setSocketTableD(struct sockaddr_in *, int); 
    void setSocketTable(int );
    void delSocketTable(int );
	
	/* for test */
	void setSocketTable(struct sockaddr_in *, int sockfd, char * ip, unsigned short port);
  private:
    vector<sockTb*>::iterator stbiter;
    sockTb *s;
};

/******************************************************
 * tou main class
 * ***************************************************/
class touMain {

	private:
		
	unsigned long test;
	touPkg tp;
	sockTb s, *s1;
	int sd;
	int sd2;
	int yes;
	string touhstring;
	char buf[50],buf1[50],buf3[50],buf4[50],buf5[50];
	timerMng timermng;
	sockMng sm;
	//ioCongMng iocm;
	//touheaderack thack;
	
	public :
	
	int touSocket(int , int , int );
	int touAccept(int , struct sockaddr_in *, socklen_t * );
	int touConnect(int , struct sockaddr_in *, int );
	int touBind(int , struct sockaddr *, int );
	int touListen();
	int touSend(int , char *, int , int );
	int touRecv(int , char *, int , int );
	int touClose();
	void convertToByteOrder(touPkg);
	void convertFromByteOrder(touPkg);
	int timero(int , int);
	int assignaddr(struct sockaddr_in *, sa_family_t , char* , u_short);
	
};	
