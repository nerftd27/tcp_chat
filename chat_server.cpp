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
#include "agg_client.h"

using json = nlohmann::json;
	
class chat_server {
	int listener;
	std::vector<agg_client> accepted_sockets;
	
	char request[BUF_SIZE];
	char response[BUF_SIZE];
	
	int bytes_read;
	int broadcast;				//triger for broadcast sending
	int id_cl_msg;
	std::vector<chat_user> dbUsers;		//storage here references logins and passwords 
	std::string msg_body;			//for broadcast sending
	std::string sender_login;		//for broadcast sending

	int getListenSocket();
	commands parseRequest(std::vector<agg_client>::iterator it);
	int sendResponse(std::vector<agg_client>::iterator it);
	int checkUUID(long uuid_replied);	//checking authorization with replied uuid
public:
	chat_server();
	~chat_server();
	void Processing();
};

chat_server::chat_server() {
	getListenSocket();
	accepted_sockets.clear();

	for (int i=0;i<MAX_USERS;i++)
		dbUsers.push_back(initUser(i+1));
}

chat_server::~chat_server() {
	for(auto i:accepted_sockets) {
		close(i.sock);
	}
	if (listener) close(listener);
}

commands chat_server::parseRequest(std::vector<agg_client>::iterator it) {
	broadcast=0;
	json j_request=json::parse(request);
      	json j_response;
      	j_response["id"]=it->id_msg;	
	
	id_cl_msg=j_request.value("id",0);
        int cmnd=j_request.value("command",0);

        switch (cmnd) {
		case (commands::hello): {
			j_response["command"]=commands::hello_reply;
			j_response["auth_method"]="plain_text";
		} break;
		case (commands::login): {
			j_response["command"]=commands::login_reply;
			chat_user temp;
			temp.login=j_request.value("login","oops");
			temp.password=j_request.value("password","oops");
			if (authorizeUser(dbUsers.begin(),&temp)) {
				it->user=temp;
				it->UUID_session=rand();	
				j_response["status"]=1;
				j_response["session"]=it->UUID_session;
			} else {
				j_response["status"]=0;
				j_response["message"]="authorization failed! dont know why...\n";
			}				
		} break;
		case (commands::message): {
			msg_body=j_request.value("body","oops");
			long uuid_replied=j_request.value("session",0);
			j_response["command"]=commands::message_reply;
			
//			std::cout<<"\nUUID:"<<it->UUID_session;					//for debug
//			std::cout<<"\nuuid_replied:"<<uuid_replied<<std::endl;
			
			if ((it->UUID_session)and(it->UUID_session==uuid_replied)) {		//checking authorization with replied uuid session
				j_response["status"]=1;
				j_response["client_id"]=id_cl_msg;
				sender_login=it->user.login;
				broadcast=1;			//next sendRequest will be broadcast	
			} else {
				j_response["status"]=0;
				j_response["message"]="sending message to chat failed! problem with authorization...\n";
			}		 									  					  				  
		} break;
		/*case (commands::message_reply): {		//no need reponse, its confirmation from clients
			std::cout<<"confirmation recieved from "<<it->user.login<<std::endl;	//implement here needed logic				       
			return static_cast<commands>(cmnd);	
		}*/
		case (commands::ping): {
			j_response["command"]=commands::ping_reply;
			long uuid_replied=j_request.value("session",0);
			if ((it->UUID_session)and(it->UUID_session==uuid_replied)) {		//checking authorization with replied uuid session
				j_response["status"]=1;
	
			} else {
				j_response["status"]=0;
				j_response["message"]="ping failed! problem with authorization...\n";
			}		
		} break;
		case (commands::logout): {
                        j_response["command"]=commands::logout_reply;
                        //long uuid_replied=j_request.value("session",0);	//not used, status always return ok
			j_response["status"]=1;
			//delete almost all data (except socket).Also unmarking flag authorized in dbUsers 
			//We left only 1 field - its  socket in *it, and it will be erased when bytes_read<0 in processing()
			clearAgg_client(dbUsers.begin(),*it);			
		} break;

	}	
	
	std::string s=j_response.dump();
        std::copy(s.begin(),s.end(),response);
        response[s.size()]='\0';
	return static_cast<commands> (cmnd);
}


int chat_server::sendResponse(std::vector<agg_client>::iterator cl) {
	int r =	send(cl->sock,response,sizeof(response),0);	
	std::cout<<"sended to "<<cl->sock<<" data :"<<response<<std::endl;
	
	//if we have msg to chat, need to assebmly new json and send it to all users
	if (broadcast) {		
		json j;
		for (auto i:accepted_sockets) {
			if ((i.UUID_session)and(i.user.login!=sender_login)) {		//send msg only authorized session, except sender user
				j["id"]=i.id_msg;
				j["command"]=commands::message;
				j["body"]=msg_body;
				j["sender_login"]=sender_login;
			
				std::string s=j.dump();
        			std::copy(s.begin(),s.end(),response);
        			response[s.size()]='\0';
				
				r = send(i.sock,response,sizeof(response),0);
				std::cout<<"BROADCAST sended to "<<i.sock<<" data :"<<response<<std::endl;
				i.id_msg++;		
			}						
		}
	broadcast=0;
	}
	cl->id_msg++;	
	return r;

}

int chat_server::getListenSocket() {
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
	return 0;
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
		timeout.tv_sec=180;
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
		}

		if (0 >= select (mx+1, &set1, NULL, NULL, &timeout)) {	//checking sockets in set1 which  need processing
			perror("srv.select() error\n");
			exit(3);
		}
		
		//porcessing new client socket
		if (FD_ISSET(listener, &set1)) {
			
			int sock1=accept(listener, NULL, NULL);
			if(sock1<0) {
				perror("srv.accept() error");
				exit(4);
			}
			fcntl(sock1, F_SETFL, O_NONBLOCK);
			
			accepted_sockets.push_back(agg_client(sock1));	
			std::cout<<"new socket client:"<<sock1<<std::endl;
		}
		
		//processing each socket in accepted_sockets
		for (auto i=accepted_sockets.begin();i!=accepted_sockets.end();i++) {
			if (FD_ISSET(i->sock,&set1)) {			//if current socket in set1 - its need to read it
				bytes_read=recv(i->sock,request,BUF_SIZE,0);
				if (bytes_read<=0) {			//no more bytes to read, close socket
					close(i->sock);
					std::cout<<"------\nconnection closed by client "<<i->sock<<std::endl; 
					accepted_sockets.erase(i);	
					break;				//not processing left iterations for except segfault 	
				} else {
					 std::cout<<"------\nrecieved from "<<i->sock<<" data:"<<request<<std::endl;
				}
				
				parseRequest(i);
				sendResponse(i);				
				*request=0;
			}
		}
		
	}
}
		

int main() {
	chat_server srv1;

	srv1.Processing();
	return 0;
}
