#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream> 
#include <string>
#include <unistd.h>	//close(socket)
#include <fcntl.h>
#include <stdio.h> 	//STDIN_FILENO
#include "json.hpp"
#define BUF_SIZE 1024

using json = nlohmann::json;

class chat_client {
	int unsigned id_msg;
	int sock;
	int bytes_read;
	char req[BUF_SIZE];
	char ans[BUF_SIZE];
	std::string temp;
public:
	chat_client();
	~chat_client();
	void processing();
	void sendRequest();
	void recieveAnswer();
	void assemblyRequest(int command=0);
};


void chat_client::processing() {
	while (1) {
		fd_set set1;
		FD_ZERO(&set1);
		FD_SET(sock,&set1);
		timeval timeout;                //for select()
                timeout.tv_sec=5;
                timeout.tv_usec=0;
		
                while (0 < select (sock+1, &set1, NULL, NULL, &timeout)) { 
        		if (FD_ISSET(sock,&set1)) {
			
					bytes_read=recv(sock,ans,BUF_SIZE,0);
					if (bytes_read>0) std::cout<<"recieved from server:"<<ans<<std::endl;
				
			}               
                }
			
		
	
		std::cin>>req;
		if ('R'!=req[0]) {
			send(sock,req,BUF_SIZE,0);
			std::cout<<"sended to server:"<<req<<std::endl;
		}
		req[0]='R';
	
		
		
		
		
		
		/*bytes_read=recv(sock,ans,BUF_SIZE,0);
		if (bytes_read >= 0) std::cout<<"recieved from server:"<<ans<<std::endl;
		bytes_read=-1;

		std::cout<<"input msg:";
		std::cin>>req;

		if ('R'!=req[0]) {
			req[0]='R';
			send(sock,req,BUF_SIZE,0);
			std::cout<<"sended to server:"<<req<<std::endl;
			if ('q'==req[0]) break;
		}*/
		
	}
}
void chat_client::assemblyRequest(int command) {
	std::cout<<"command:"<<command<<std::endl;
	std::string s;
	json j;
	if (0==command) {
		j["id"]=id_msg;
		j["command"]="HELLO";
	}
	if (1==command) {
		j["id"]=id_msg;
		j["command"]="login";
		j["login"]="1";
		j["password"]="2";
	}
	if (2==command) {
		j["id"]=id_msg;
		j["command"]="message";
		j["body"]="test_msg";
		j["session"]=999;
	}
	if (3==command) {
		j["id"]=id_msg;
		j["command"]="ping";
		j["session"]=999;
	}
	if (4==command) {
		j["id"]=id_msg;
		j["command"]="logout";
		j["session"]=999;
	}

	s=j.dump();
	std::copy(s.begin(),s.end(),req);
	req[s.size()]='\0';
}




chat_client::chat_client() {
	id_msg=1;
	req[0]='R';
	req[1]='\0';
	temp="!";

	struct sockaddr_in peer;
	sock = socket( AF_INET, SOCK_STREAM, 0 );
	if (sock<0) {
		std::cout<<"client.socket() error\n";
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
	
	fcntl(sock,F_SETFL, O_NONBLOCK);
	fcntl(STDIN_FILENO,F_SETFL, O_NONBLOCK);
}

chat_client::~chat_client() {
	if (sock) close(sock);
}


void chat_client::sendRequest() {
	int res = send(sock,req,1,0);
	if (res<=0) {
		std::cout<<"cl.send() error\n";
	}
	std::cout<<"request !"<<req<<"! sended\n";	

	id_msg++;
}

void chat_client::recieveAnswer() {
	int res = recv(sock, ans, 1, 0 );
	if ( res <= 0 )
		std::cout<<"cl.recv() error\n";
	else
		std::cout<<"answer !"<<ans<<"!  recieved\n ";
}


int main() {
	chat_client client1;
	client1.processing();
	/*for(int command=0;;) {
		std::cout<<"input command:";
		std::cin>>command;
		if (-1==command) break;
		std::cout<<std::endl;

		client1.assemblyRequest(command);
		client1.sendRequest();
		client1.recieveAnswer();
	}*/
	return 0;;
}
