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

const int BUF_SIZE=1024;
const int MAX_USERS=5;

enum commands {hello=0, login=1, message=2, ping=3, logout=4,
                hello_reply=5, login_reply=6, message_reply=7, ping_reply=8, logout_reply=9,
                chat_exit=10};

struct chat_user {
	std::string login;
	std::string password;
	int authorized;
};

void initUser(chat_user& usr,int k) {
	usr.authorized=0;
	usr.login="user"+std::to_string(k);
	usr.password=std::to_string(k*111);
	std::cout<<"init "<<usr.login<<" pwd:"<<usr.password<<std::endl;
}

int authorizeUser(chat_user* dbUsers, chat_user* guest) {
	for (;dbUsers;dbUsers++) 
		if ((dbUsers->login==guest->login) and (dbUsers->password==guest->password) and (!dbUsers->authorized)) {
			dbUsers->authorized=1;
			return 1 ;
		}
	return 0;
}

struct client {
	long UUID_session;
	chat_user user;
	int sock;
	unsigned int id_msg;
};
	
class chat_server {
	int listener;
	unsigned int id_cl_msg;
	std::vector<client> accepted_sockets;
	char request[BUF_SIZE];
	char response[BUF_SIZE];
	int bytes_read;
	int broadcast;				//triger for broadcast sending responses
	void getListenSocket();
	void broadcastSend();
	void parseRequest(std::vector<client>::iterator cl);
	void sendResponse();
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
		close(i.sock);
	}
	if (listener) close(listener);
}

void chat_server::parseRequest(std::vector<client>::iterator cl) {
	broadcast=0;
	json j_request=json::parse(request);
      	json j_response;
      	j_response["id"]=cl->id_msg;	
	
	id_cl_msg=j_request.value("id",0);
        int cmnd=j_request.value("command",0);
       // std::string s="id_cl_msg:"+std::to_string(id_cl_msg)+" command:"+ std::to_string(cmnd);
        switch (cmnd) {
		case (commands::hello): {
			j_response["command"]=commands::hello_reply;
			j_response["auth_method"]="plain_text";
			break;
		}
		case (commands::login): {
			j_response["command"]=commands::login_reply;
			chat_user us;
			us.login=j_request.value("login","oops");
			us.password=j_request.value("password","oops");
			break;
		}
	}	
	
	
	
	std::string s=j_response.dump();
        std::copy(s.begin(),s.end(),response);
        response[s.size()]='\0';
/*
                case (commands::hello_): {

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
                case (commands::message): {             //server sended us some msg from other user
                        s+=" usr "+j.value("sender_login","oops")+" msg:"+j.value("body","oops");       //+UUID_session
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

*/


}


void chat_server::sendResponse() {
}

void chat_server::broadcastSend() {
	for (auto i:accepted_sockets) {
		send(i.sock,response,sizeof(response),0);	
		std::cout<<"sended to "<<i.sock<<" data :"<<response<<std::endl;

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
			FD_SET(i.sock, &set1);	//adding all accepted sockets from vector to set1
		}
				
		
		timeval timeout;	
		timeout.tv_sec=30;
		timeout.tv_usec=0;
		
		//awaiting clients sockets
		int mx=listener;
		if (accepted_sockets.size()) {
			//findix max num socket in accepted_sockets
			int max_sock=accepted_sockets[0].sock;
			for (auto i:accepted_sockets) 
				if (max_sock<i.sock) 
					max_sock=i.sock;
			mx=std::max(listener, max_sock);
			//mx=std::max(listener,*std::max_element(accepted_sockets.begin(),accepted_sockets.end()));
		}

		if (0 >= select (mx+1, &set1, NULL, NULL, &timeout)) {	//checking sockets in set1 which  need processing
			perror("srv.select() error\n");
			exit(3);
		}
		
		//porcessing new client
		if (FD_ISSET(listener, &set1)) {
			
			int sock1=accept(listener, NULL, NULL);
			if(sock1<0) {
				perror("srv.accept() error");
				exit(4);
			}
			fcntl(sock1, F_SETFL, O_NONBLOCK);
			accepted_sockets.push_back({0,{0,0,0},sock1,0});	//blanked only 1 field - sock
			std::cout<<"new socket client:"<<sock1<<std::endl;
		}
		
		//processing each socket in accepted_sockets
		for (auto i=accepted_sockets.begin();i!=accepted_sockets.end();i++) {
			if (FD_ISSET(i->sock,&set1)) {			//if current socket in set1 - its need to read it
				bytes_read=recv(i->sock,request,BUF_SIZE,0);
				parseRequest(i);
				std::cout<<"recieved from "<<i->sock<<" data:"<<request<<std::endl;
				if (bytes_read<=0) {			//no more bytes to read, close socket
					close(i->sock);
				
					std::cout<<"connection closed by client "<<i->sock<<std::endl; 
					accepted_sockets.erase(i);	
					break;				//not processing left iterations for except segfault 
					
				}
									
			//	broadcastSend();
			}
		}
		
	}
}
		

int main() {
	chat_server srv1;
	chat_user users[MAX_USERS];
	for (int i=1;i<=MAX_USERS;i++)
		initUser(users[i],i);
	srv1.Processing();
	return 0;
}
