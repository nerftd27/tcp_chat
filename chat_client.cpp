#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream> 
#include <string>
#include <unistd.h>	//close(socket)

int getConnectedSocket() {
	int sock;
		
	struct sockaddr_in peer;
	sock = socket( AF_INET, SOCK_STREAM, 0 );
	if (sock<0) {
		std::cout<<"cl.socket() error";
		return -1;
	}

	peer.sin_family = AF_INET;
	peer.sin_port = htons( 7500 );
	peer.sin_addr.s_addr = inet_addr( "127.0.0.1" );

	int rc = connect(sock,(struct sockaddr*) &peer,sizeof(peer));
	if ( rc ) {
		std::cout<<"cl.connect() error";
		return -2;
	}

	return sock;
	
}

int main() {
	int sock;
	int rc;
	char buf[1024];
	std::string msg;
	std::cout<<"Input msg:\n";
	std::cin>>msg;

	sock=getConnectedSocket();
	
	if (sock<0) {
		std::cout<<"cl.Cant get connected socket";       
		return 1;
	}
	
	
	rc = send(sock,msg.c_str(),sizeof(msg.c_str()),0);
	if (rc<=0) {
		std::cout<<"cl.send() error";
		return 1;
	}

	rc = recv(sock, buf, sizeof(buf), 0 );
	if ( rc <= 0 )
		std::cout<<"cl.recv() error";
	else
		std::cout<<"from srv: "<<buf;
	close(sock);

	return 0;;
}
