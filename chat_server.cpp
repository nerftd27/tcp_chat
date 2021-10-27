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
	char req[BUF_SIZE];
	char ans[BUF_SIZE];


	void getListenSocket();
public:
	chat_server();
	~chat_server();
	void Processing();
};

chat_server::chat_server() {
	getListenSocket();
}

chat_server::~chat_server() {
	for(auto i:accepted_sockets) {
		close(i);
	}
	if (listener) close(listener);
}


void chat_server::getListenSocket() {
	int result;
	sockaddr_in local;
	
	listener= socket(AF_INET, SOCK_STREAM, 0);
	if (listener<0) {
		std::cout<<"server.socket() error";
		std::exit(1);
	}
	
	local.sin_family = AF_INET;
	local.sin_port = htons(7500);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	

	
	result = bind( listener, (sockaddr*) &local, sizeof(local) );
	if (result<0) {
		std::cout<<"server.bind() error";
		std::exit(1);
	}
		
	result = listen(listener,5);
	if (result) {
		std::cout<<"server.listen() error";
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
		timeout.tv_sec=15;
		timeout.tv_usec=0;
		
		//awaiting clients sockets
		int mx=listener;
		if (accepted_sockets.size()) {
			mx=std::max(listener,*std::max_element(accepted_sockets.begin(),accepted_sockets.end()));
		}

		//std::max(listener,*std::max_element(accepted_sockets.begin(),accepted_sockets.end())); //for select()
		if (0 >= select (mx+1, &set1, NULL, NULL, &timeout)) {	//checking sockets in set1 which  need processing
			std::cout<<"srv.select() error\n";
			exit(3);
		}
		
		//porcessing new client
		if (FD_ISSET(listener, &set1)) {
			
			int sock=accept(listener, NULL, NULL);
			if(sock<0) {
				perror("srv.accept() error");
				exit(3);
			}
			fcntl(sock, F_SETFL, O_NONBLOCK);
			accepted_sockets.push_back(sock);
		}
		
		//processing each socket in accepted_sockets
		for (auto i=accepted_sockets.begin();i!=accepted_sockets.end();i++) {
			if (FD_ISSET(*i,&set1)) {			//if current socket in set1 - its need to read it
				int bytes_read=recv(*i,req, BUF_SIZE,0);
				if (bytes_read<=0) {			//no more bytes to read, close socket
					close(*i);
					accepted_sockets.erase(i);
					if (0==accepted_sockets.size()) break; //segfault
				
					continue;
				}
				send(*i,req, bytes_read,0);		//pesponse for client
			}
		}
	}
}
		

int main() {
	chat_server srv1;
	srv1.Processing();
	return 0;
}
