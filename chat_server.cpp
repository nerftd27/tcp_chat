#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <iostream>
#include <cstdlib>
#include <unistd.h> //close socket
#include <vector>
#include "json.hpp"

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
	//getting new socket
	int sock = accept(listener, NULL, NULL);
	if(sock < 0)
        {
		std::cout<<"server.accept() error";
            	exit(3);
        }
	accepted_sockets.push_back(sock);


	//processing each opened socket
	for (auto i:accepted_sockets) {

		recv(i,req,BUF_SIZE,0);
		std::cout<<"Recieved msg :"<<req<<std::endl;
	
		*ans='Z';
		send(i,ans,BUF_SIZE, 0);
	
		std::cout<<"Sended msg:"<<ans<<std::endl;
	}

	

	//free res
	//close(*accepted_sockets.end());
	//accepted_sockets.pop_back();
}



/*std::string ProcessingJson(int& sock) {

	char temp[BUF_SIZE];
	recv(sock,temp,BUF_SIZE, 0);
	json j = json::parse(temp);
	
	std::string req=j.value("field2","oops");
	std::cout<<"Recieved: "<<req<<std::endl;
	return req;
}*/




int main() {
	chat_server srv1;
	
	while(1) {
        	srv1.Processing();
	
    	}
	return 0;
}
