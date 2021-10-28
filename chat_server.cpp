#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h> //close socket
#include <vector>
#include "json.hpp"
#include <fcntl.h> // fd_set
#include <algorithm>

using json = nlohmann::json;

#define BUF_SIZE 1024

struct chat_user {
	char* login;
	char* password;
};


struct chat_session {
	long UUID;
	chat_user user;
	int client_socket;
};
	
class chat_server {
	int listener;
	std::vector<int> accepted_sockets;
	std::vector<chat_session> sessions;
	char request[BUF_SIZE];
	char response[BUF_SIZE];
	int bytes_read;
	void getListenSocket();
	void broadcastSend();
public:
	chat_server();
	~chat_server();
	void Processing();
};

chat_server::chat_server() {
	getListenSocket();
	accepted_sockets.clear();
}

chat_server::~chat_server() {
	for(auto i:accepted_sockets) {
		close(i);
	}
	if (listener) close(listener);
}

void chat_server::broadcastSend() {
	for (auto i:accepted_sockets) {
		send(i,request,bytes_read,0);	
		std::cout<<"sended to "<<i<<" data :"<<request<<std::endl;

	}
}

void chat_server::getListenSocket() {
	int result;
	sockaddr_in local;
	
	listener= socket(AF_INET, SOCK_STREAM, 0);
	if (listener<0) {
		perror("server.socket() error");
		std::exit(1);
	}
	
	fcntl(listener, F_SETFL, O_NONBLOCK);
		
	local.sin_family = AF_INET;
	local.sin_port = htons(7500);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	
	result = bind( listener, (sockaddr*) &local, sizeof(local) );
	if (result<0) {
		perror("server.bind() error");
		std::exit(1);
	}
		
	result = listen(listener,5);
	if (result) {
		perror("server.listen() error");
		std::exit(1);
	}

}


void chat_server::Processing() {
	while(1) {
		fd_set set1;			//set of non-blocking sockets
		FD_ZERO	(&set1);		//clearing set1
		FD_SET(listener, &set1);	//listen socket added to set1
	
		for(auto i:accepted_sockets) {
			FD_SET(i, &set1);	//adding all accepted sockets from vector to set1
		}
				
		
		timeval timeout;		//for select()
		timeout.tv_sec=30;
		timeout.tv_usec=0;
		
		//awaiting clients sockets
		int mx=listener;
		if (accepted_sockets.size()) {
			mx=std::max(listener,*std::max_element(accepted_sockets.begin(),accepted_sockets.end()));
		}

		if (0 >= select (mx+1, &set1, NULL, NULL, &timeout)) {	//checking sockets in set1 which  need processing
			perror("srv.select() error\n");
			exit(3);
		}
		
		//porcessing new client
		if (FD_ISSET(listener, &set1)) {
			
			int sock=accept(listener, NULL, NULL);
			if(sock<0) {
				perror("srv.accept() error");
				exit(4);
			}
			fcntl(sock, F_SETFL, O_NONBLOCK);
			accepted_sockets.push_back(sock);
			std::cout<<"new client:"<<sock<<std::endl;
		}
		
		//processing each socket in accepted_sockets
		
		for (auto i=accepted_sockets.begin();i!=accepted_sockets.end();i++) {
			if (FD_ISSET(*i,&set1)) {			//if current socket in set1 - its need to read it
				bytes_read=recv(*i,request,BUF_SIZE,0);
				std::cout<<"recieved from "<<*i<<" data:"<<request<<std::endl;
				if (bytes_read<=0) {			//no more bytes to read, close socket
					close(*i);
				//	auto iter_temp=i;
					std::cout<<"closed connection client "<<*i<<std::endl; 
					accepted_sockets.erase(i);
					break;				//not processing left iterations for() for dismiss segfault 
					//if (0==accepted_sockets.size()) break; 
					//continue;
				}
			
			//	send(*i,request,bytes_read,0);		
			//	std::cout<<"sended to "<<*i<<" data "<<request<<std::endl;
				broadcastSend();
			}
		}
		
	}
}
		

int main() {
	chat_server srv1;
	srv1.Processing();
	return 0;
}
