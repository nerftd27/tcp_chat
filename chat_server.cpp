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

int getListenSocket() {
	int sock;
	int result;
	sockaddr_in local;
	
	sock= socket(AF_INET, SOCK_STREAM, 0);
	if (sock<0) {
		std::cout<<"server.socket() error";
		std::exit(1);
	}
	
	local.sin_family = AF_INET;
	local.sin_port = htons(7500);
	local.sin_addr.s_addr = htonl(INADDR_ANY);
	
	
	result = bind( sock, (sockaddr*) &local, sizeof(local) );
	if (result<0) {
		std::cout<<"server.bind() error";
		std::exit(1);
	}
		
	result = listen(sock,5);
	if (result) {
		std::cout<<"server.listen() error";
		std::exit(1);
	}
	
	return sock;
}


std::string ProcessingJson(int& sock) {

	char temp[BUF_SIZE];
	recv(sock,temp,BUF_SIZE, 0);
	json j = json::parse(temp);
	
	std::string req=j.value("field2","oops");
	std::cout<<"Recieved: "<<req<<std::endl;
	return req;
}


void ProcessingMsg(int& sock) {
	std::cout<<"start ProcessingMsg()\n";
	std::string ans=ProcessingJson(sock);

	std::vector<char> buf(ans.begin(), ans.end());
	buf.push_back('\0');
	send(sock,&buf[0],BUF_SIZE, 0);

	std::cout<<"exit from ProcessingMsg()\n";
}

int main() {
	int listener;
	int sock;
	int res;
	char buf[BUF_SIZE];
	int bytes_read=0;
	std::string req="";
	std::string ans="";

	listener=getListenSocket();
	
	while(1) {
        	sock = accept(listener, NULL, NULL);
	        if(sock < 0)
        	{
			std::cout<<"server.accept() error";
            		exit(3);
        	}

        	ProcessingMsg(sock);
		close(sock);
    	}
	return 0;
}
