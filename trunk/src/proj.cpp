/*******************************************************************************
 * test case
 * @ Nov 24, 2009 (sol) - basic test case.
 * @ Jan 09, 2010 (sol) - adding some logging mech.
 *		
 * ****************************************************************************/

#include "tou.h"
#include <iostream>
#include <fstream>
#include <sys/socket.h>
#include <netdb.h>

//#define MAXSNDBUF		1048576	//1mb
//#define MAXSNDBUF 1501036 //1000 * 1448bytes
#define MAXSNDBUF 2000000 
#define TIMEOUTVAL	4				//four seconds
using namespace std;


/* For midterm demo: 
 * relate code: proj.cpp main; ofile
 * touprocess.cpp send ofile (includeing etern)
 */
FILE *ofile;
FILE *ofile_cwnd;

int assignaddr(struct sockaddr_in *sockaddr, sa_family_t sa_family, string ip, unsigned short port){
	bzero(sockaddr, sizeof(*sockaddr));
	sockaddr->sin_family = sa_family;
	sockaddr->sin_port = htons(port);
	if( 1 != inet_pton(sa_family, ip.c_str(), &sockaddr->sin_addr) )
	  return 0;
	return 1;
};

int main(int argc, char* argv[]){
	touMain							tm;
	struct sockaddr_in	socket1;
	struct sockaddr_in	socket2;
	char								send_data[MAXSNDBUF],recv_data[MAXSNDBUF];
	ifstream            indata;
	int									readsize; //how much data been read
	int									sendsize; //how much data been sent
	int									totalsendsize=0;
	long								timedif, tsecs, tusecs;
	struct timeval			tstart;

	//LOGFLAG = TOULOG_PTSRN;
	//LOGFLAG = TOULOG_ALL;
	LOGFLAG = 0;
	LOGON = true;

	//* for midterm demo //
	ofile =  fopen("seqnumber.txt", "w");
	ofile_cwnd = fopen("seqcwnd.txt", "w");


  if(argc == 4 && !strncmp( argv[3], "-c", 2)) /* ./tou 127.0.0.1 ./test_file -c */
	{
		cout << "############ Client mode ############ " << endl;
		int selectval = 0;
		struct in_addr ipv4addr;
		int sd,sockd;
		int bytes_recieved; 
		struct hostent *h;

		//Set up select func socket
    fd_set socks;
 		struct timeval tim;
		bool sentelapse = false; 

		//Get host name
		inet_pton(AF_INET, argv[1], &ipv4addr);
		h = gethostbyaddr(&ipv4addr, sizeof ipv4addr, AF_INET);
    if (h == NULL)  perror("[Server]gethostbyaddr: ");

    //Set socket 1: set up Server socket info
    memset(&socket1, 0, sizeof(socket1));
   	socket1.sin_family = h->h_addrtype;
   	memcpy((char *) &socket1.sin_addr.s_addr, h->h_addr_list[0], h->h_length);
    socket1.sin_port = htons(1500);

		/* assign server's addr */
		assignaddr(&socket1, AF_INET, argv[1], 1500);
		cout<<"# Sending msg to: "<< inet_ntoa(socket1.sin_addr)<< " " <<htons(socket1.sin_port) << endl;
		
		//Client socket
		sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
		cout << "touSocket returns: " << sockd << endl;
	
		//set up Client socket info 
		memset(&socket2,0,sizeof(socket2));
		socket2.sin_family = AF_INET;
		socket2.sin_addr.s_addr = INADDR_ANY;
		socket2.sin_port = htons(1501);
	
		//BIND
		sd = tm.touBind(sockd,(struct sockaddr*) &socket2,sizeof socket2);
		cout << "touBind returns: "<< sd << endl;
	
		//CONNECT
		sd = tm.touConnect(sockd,(struct sockaddr_in*)&socket1,sizeof(socket1));
		cout << "touConnect returns : "<< sd << endl;
		
		//Reading the file
		indata.open(argv[2]); // opens the file
		if(!indata) { // file couldn't be opened
			cerr << "Error: file could not be opened" << endl;
			exit(1);
		}

		if( !indata.eof() ) {
		  cerr << "Reading data from file: "<< argv[2] << endl << endl;
			indata.read(send_data, MAXSNDBUF);
			readsize = indata.gcount();

      //SEND: touSend
			cerr << endl << "|||TIMESTAMP||| " << timestamp() << endl;
			gettimeofday(&tstart, NULL);//start counting

			sendsize = tm.touSend(sockd,send_data, readsize ,0);
			totalsendsize += sendsize;
			cout << "touSend: Trying to send "<< readsize << "bytes ";
			cout << "touSend returns "<<sendsize << " bytes" << endl; 
		}

		while(true){
			FD_ZERO(&socks);
			FD_SET(sockd, &socks);
			tim.tv_sec = TIMEOUTVAL;
			tim.tv_usec = 0;

			if( !indata.eof() && (tm.sm.s->CbSendBuf.getAvSize()>1500000) ) {
		  //cerr << "Reading data from file: "<< argv[2] << endl << endl;
			indata.read(send_data, 1500000);
			readsize = indata.gcount();

      //SEND: touSend
			sendsize = tm.touSend(sockd,send_data, readsize ,0);
			totalsendsize += sendsize;
			}


			selectval = select(sockd+1, &socks, NULL, NULL, &tim);
			//cout<< "\n >>> NEW ITERATION >>> Select LOOP: select returns: "<<selectval<< endl; 
			if (selectval){
				//cout<< "Execute processTou->RUN: recv ACK"<< endl;
				tm.ptou->run(sockd);

				//throughput test
				/*
				if (tm.ptou->tend.tv_sec >= tstart.tv_sec && sentelapse) {
					tsecs  = tm.ptou->tend.tv_sec  - tstart.tv_sec;
					tusecs = tm.ptou->tend.tv_usec - tstart.tv_usec;
					timedif = (tsecs) * 1000 + tusecs/1000.0;
					cerr << endl << "It took you " << timedif << " milliseconds to send the "<< totalsendsize<< " bytes data." << endl;
					sentelapse = false;
				}
				*/

			}else if(selectval == 0){
				cerr<< ">> Select timeout: leave Select LOOP"<< endl;
				break;
			}else{
			break;
			}

			if ( totalsendsize < readsize ) {
				sendsize = tm.touSend(sockd,send_data, readsize ,0);
				totalsendsize += sendsize;
			}

		}// while(true)  

		cout << "Exit!" << endl;
		cout << "Total send: "<< totalsendsize << endl;
		exit(1);
		tm.touClose(sockd);
		/* End of Client Code */
	
	}else if ( argc==2 && !strncmp(argv[1], "-s", 2)){
		cout << "############ Sever Mode ############"<< endl;
		int sd,bytes_recieved,lis_return;
		int sockd;
		//Check if it is tou socket
    fd_set socks;
 		struct timeval tim;
		int recv_cnt = 0;


		//CREATE 
		sockd = tm.touSocket(AF_INET,SOCK_DGRAM,0);
		cout << "touSocket returns: " << sockd << endl;

		//Set socket structures
		memset(&socket1,0,sizeof(socket1));
		socket1.sin_family = AF_INET;
		socket1.sin_addr.s_addr = INADDR_ANY;
		socket1.sin_port = htons(1500);
		memset(&socket2,0,sizeof(socket2));

		//BIND
		sd = tm.touBind(sockd,(struct sockaddr*) &socket1,sizeof socket1);
		cout << "touBind returns: "<< sd << endl;

		//LISTEN
		lis_return = tm.touListen(sockd,1);
		cout << "touListen returns: "<< lis_return << endl;

		//Testing the throughput: 5% loss rate.
		int* pthroughput = tm.ptou->setThroughput(0, 100); 
		for(int i=0; i<100; i++) if (pthroughput[i] == 1) cout <<i << ", ";

		socklen_t sinlen = sizeof(socket2);
		tm.ptou->run(sockd);
		sd = tm.touAccept(sockd,(struct sockaddr_in*)&socket2,&sinlen);
		cout << "touAccept returns: "<< sd << endl;

		while(true){    
			FD_ZERO(&socks);
			FD_SET(sockd, &socks);
			tim.tv_sec = TIMEOUTVAL;

			if (select(sockd+1, &socks, NULL, NULL, &tim)){
				tm.ptou->run(sockd);
				
				//RECV: touRecv data
				memset(recv_data, 0, sizeof(recv_data));
				sd = tm.touRecv(sockd,recv_data,MAXSNDBUF,0);
				recv_cnt += sd;
				//cout << "touRecv returns: "<< sd << " recv buffer size: " << sizeof recv_data << endl;
				//if (sd == 0) break;

			}else{
				cout << "Select Timeout: Exit!" << endl;
				break;
			}      
		}
		

		cout << "Inside Close...... " << endl;
		cout << "# of recv_cnt: " << recv_cnt << endl;
		exit(1);
		tm.ptou->run(sockd);
		tm.touClose(sockd);
       
  }/*END OF SERVER PART*/

	return 0;
}
