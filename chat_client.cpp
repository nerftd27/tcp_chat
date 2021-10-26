#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <iostream> 
#include <string>
#include <unistd.h>	//close(socket)

#include "json.hpp"
#define BUF_SIZE 20000

using json = nlohmann::json;

class chat_client {
	int unsigned id_msg;
	int sock;
	char req[BUF_SIZE];
	char ans[BUF_SIZE];
public:
	chat_client();
	~chat_client();
	void sendRequest();
	void recieveAnswer();
	void assemblyRequest(int command=0);
};

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
		std::cout<<"client.connect() error\n";
		std::exit(-2);
	}
	
}

chat_client::~chat_client() {
	if (sock) close(sock);
}


void chat_client::sendRequest() {
	int res = send(sock,req,BUF_SIZE,0);
	if (res<=0) {
		std::cout<<"cl.send() error\n";
	}
	std::cout<<"request !"<<req<<"! sended\n";	

	id_msg++;
}

void chat_client::recieveAnswer() {
	int res = recv(sock, ans, BUF_SIZE, 0 );
	if ( res <= 0 )
		std::cout<<"cl.recv() error\n";
	else
		std::cout<<"answer !"<<ans<<"!  recieved\n ";
}


int main() {
	chat_client client1;
	for(int command=0;;) {
		std::cout<<"input command:";
		std::cin>>command;
		if (-1==command) break;
		std::cout<<std::endl;

		client1.assemblyRequest(command);
		client1.sendRequest();
		client1.recieveAnswer();
	}
	return 0;;
}
