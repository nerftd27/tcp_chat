#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream> 
#include <string>
#include <unistd.h>	//close(socket)
#include <stdio.h> 	//STDIN_FILENO
#include "json.hpp"
#define BUF_SIZE 1024

using json = nlohmann::json;

enum commands {hello=0, login=1, message=2, ping=3, logout=4};

struct chat_user { 
        char login[BUF_SIZE]; 
        char password[BUF_SIZE]; 
};

class chat_client {
	int unsigned id_msg;
	long uuid_session;
	int sock;
	chat_user usr;

	char request[BUF_SIZE];
	char response[BUF_SIZE];
	
	void assemblyRequest(int command=0);
	void sendRequest();
	void recieveResponse();
	commands parseResponse();
public:
	chat_client();
	~chat_client();
	void processing();
};

void chat_client::assemblyRequest(int command) {
	std::string s;
	json j;
	if (commands::hello==command) {
		j["id"]=id_msg;
		j["command"]=commands::hello;
	}
	if (commands::login==command) {
		j["id"]=id_msg;
		j["command"]=commands::login;
		j["login"]=usr.login;
		j["password"]=usr.password;
	}
	if (commands::message==command) {
		j["id"]=id_msg;
		j["command"]=commands::message;
		j["body"]="body_msg";
		j["session"]=uuid_session;
	}
	if (commands::ping==command) {
		j["id"]=id_msg;
		j["command"]=commands::ping;
		j["session"]=uuid_session;
	}
	if (commands::logout==command) {
		j["id"]=id_msg;
		j["command"]=commands::logout;
		j["session"]=uuid_session;
	}

	s=j.dump();
	std::copy(s.begin(),s.end(),request);
	request[s.size()]='\0';
}

commands chat_client::parseResponse() {
	json j=json::parse(response);
	commands cmnd=static_cast<commands>(j.value("command",0));
	std::cout<<"Command:"<<cmnd<<std::endl;
	return cmnd;
}

void chat_client::processing() {
	int command=0;
	while(1) {	
		//fork for real-time displaying messages from server.
		//here we obtain raw messages, parsing it, and execute needed actions
		switch (fork()) {
		case -1:
			perror("fork error");
			break;
		case 0:
			while(1) {
				int bytes_read=recv(sock,response,BUF_SIZE,0);
				if (bytes_read<=0) break;
				parseResponse();		
				//std::cout<<"recieved from server:"<<response<<std::endl;
			}
			exit(0);
		default:
			break;
		}
	
		//sending message
		//	0 - HELLO
		//	1 - login
		//	2 <msg> - message
		//	3 - ping
		//	4 - logout
		//      5 - force exit
		std::cin>>command;		//fixme - no check input
		if (5==command)  exit(0);
		assemblyRequest(command);
		command=-1;
		sendRequest();
		}
}

chat_client::chat_client() {
	id_msg=1;
	strcpy(usr.login,"111");
	strcpy(usr.password,"111");
	uuid_session=999;


	struct sockaddr_in peer;
	sock = socket( AF_INET, SOCK_STREAM, 0 );
	if (sock<0) {
		perror("client.socket() error");
		std::exit(-1);
	}
	

	peer.sin_family = AF_INET;
	peer.sin_port = htons( 7500 );
	peer.sin_addr.s_addr = inet_addr( "127.0.0.1" );

	int res = connect(sock,(struct sockaddr*) &peer,sizeof(peer));
	if ( res ) {
		perror("client.connect() error\n");
		std::exit(-2);
	}

}

chat_client::~chat_client() {
	if (sock) close(sock);
}


void chat_client::sendRequest() {
	int res = send(sock,request,sizeof(request),0);
	if (res<=0) {
		perror("client.send() error");
	}
	std::cout<<"request !"<<request<<"! sended\n";	

	id_msg++;
}

void chat_client::recieveResponse() {
	int res = recv(sock, response, BUF_SIZE, 0 );
	if ( res <= 0 )
		perror("client.recv() error");
	else
		std::cout<<"responce!"<<response<<"!  recieved\n ";
}


int main() {
	chat_client client1;
	client1.processing();

	return 0;;
}
