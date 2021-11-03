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

enum commands {hello=0, login=1, message=2, ping=3, logout=4,
		hello_reply=5, login_reply=6, message_reply=7, ping_reply=8, logout_reply=9,
		chat_exit=10};

struct chat_user { 
	std::string login; 
	std::string password; 
};

class chat_client {
	int unsigned id_msg;
	int unsigned id_srv_msg;
	long uuid_session;
	int sock;
	chat_user usr;
	
	char request[BUF_SIZE];
	char response[BUF_SIZE];
	
	int assemblyRequest(int command=0);
	void sendRequest();
	int recieveResponse();
	std::string parseResponse();
public:
	chat_client();
	~chat_client();
	void processing();
};

int chat_client::assemblyRequest(int command) {
	std::string s;
	json j;
	switch (command) {			//fixme - implement separate funcs according commands with array of pointers to this funcs
		case (commands::hello): {
			j["id"]=id_msg;
			j["command"]=commands::hello;
			break;
		}
		case (commands::login): {
			j["id"]=id_msg;
			j["command"]=commands::login;
			j["login"]=usr.login;
			j["password"]=usr.password;
			break;
		}
		case (commands::message): {
			j["id"]=id_msg;
			j["command"]=commands::message;
			j["body"]="body_msg";
			j["session"]=uuid_session;
			break;
		}
		case (commands::ping): {
			j["id"]=id_msg;
			j["command"]=commands::ping;
			j["session"]=uuid_session;
			break;
		}
		case (commands::logout): {
			j["id"]=id_msg;
			j["command"]=commands::logout;
			j["session"]=uuid_session;
			break;
		}
		case (commands::message_reply): { 	//responce to server to confirming recieving command::message. Not enable to user
			j["id"]=id_msg;
			j["command"]=commands::message_reply;
			j["status"]=1;
			j["client_id"]=id_msg;		//?
			break;
		}

		default:
			//not parse unknow command because command resets in processing loop all time
			break;
		}
	s=j.dump();
	std::copy(s.begin(),s.end(),request);
	request[s.size()]='\0';
	return (s.size()+1);
}

std::string chat_client::parseResponse() {
	json j=json::parse(response);
	id_srv_msg=j.value("id",0);
	int cmnd=j.value("command",0);
	std::string s="id_srv_msg:"+std::to_string(id_srv_msg)+" command:"+ std::to_string(cmnd);
	switch (cmnd) {
		case (commands::hello_reply): {
			s+=" auth_method:"+j.value("auth_method","oops");
			break;    
		}
		case (commands::login_reply): {
			int status=j.value("status",0);
			if (status) {
				uuid_session=j.value("session",0);
				s+=" ok. UUID_session="+std::to_string(uuid_session);
			} else {
				s+=" failed. "+j.value("message","oops");
			}
			break;
		}
		case (commands::message_reply): {
			int status=j.value("status",0);
			if (status) {
				int id_cl_msg=j.value("client_id",0);
				s+=" ok. id_cl_msg="+std::to_string(id_cl_msg);
			} else {
				s+=" failed. "+j.value("message","oops");
			}
			break;
		}
		case (commands::message): {		//server sended us some msg from other user
			s+=" usr "+j.value("sender_login","oops")+" msg:"+j.value("body","oops");	//+UUID_session
			//todo - add responce to server about confirming recieving
			break;
		}
		case (commands::ping_reply): {
			int status=j.value("status",0);
			if (status) {
				s+=" ok";
			} else {
				s+=" failed. "+j.value("message","oops");
			}
			break;
		}
		case (commands::logout_reply): {
			int status=j.value("status",0);
			if (status) {
				s+=" ok";
			}
	
			break;
		}


	
	}
	
	
	
	
	std::cout<<s<<std::endl;
	return s;
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
				int bytes_read=recieveResponse();//recv(sock,response,BUF_SIZE,0);
				if (bytes_read<=0) break;
				parseResponse();		
				//std out server response
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
		//      10 - exit
		std::cin>>command;		//fixme - no check input
		if (commands::chat_exit==command)  exit(0);
		assemblyRequest(command);
		command=255;			//reset command
		sendRequest();
		}
}

chat_client::chat_client() {
	id_msg=1;
	usr.login="user1";
	usr.password="111";
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

int chat_client::recieveResponse() {
	int res = recv(sock, response, BUF_SIZE, 0 );
	if ( res <= 0 )
		perror("client.recv() error");
	return res;
	
	/*else
		std::cout<<"responce!"<<response<<"!  recieved\n ";*/
}


int main() {
	chat_client client1;
	client1.processing();

	return 0;;
}
