#include "../include/chat_client.h"


int chat_client::assemblyRequest(int command) {
	std::string s;
	json j;
	switch (command) {			//todo - implement separate funcs according commands with array of pointers to this funcs
		case (commands::hello): {
			j["id"]=cl.id_msg;
			j["command"]=commands::hello;
			break;
		}
		case (commands::login): {
			j["id"]=cl.id_msg;
			j["command"]=commands::login;
			j["login"]=cl.user.login;
			j["password"]=cl.user.password;
			break;
		}
		case (commands::message): {
			buf_forks.seekg(0,std::ios::beg);
			buf_forks>>cl.UUID_session;	  

			j["id"]=cl.id_msg;
			j["command"]=commands::message;
			j["body"]=msg_body;
			j["session"]=cl.UUID_session;
			break;
		}
		case (commands::ping): {
			j["id"]=cl.id_msg;
			j["command"]=commands::ping;
			j["session"]=cl.UUID_session;
			break;
		}
		case (commands::logout): {
			j["id"]=cl.id_msg;
			j["command"]=commands::logout;
			j["session"]=cl.UUID_session;
			break;
		}
		case (commands::message_reply): { 	//responce to server to confirming recieving command::message. Service command, not for user
			j["id"]=cl.id_msg;
			j["command"]=commands::message_reply;
			j["status"]=1;
			j["client_id"]=id_srv_msg;	
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
				cl.UUID_session=j.value("session",0);	//in next iteartion it will be new fork, with zero value UUID_session, so we save it to filestream
				buf_forks<<cl.UUID_session;		//for transfering recieved uuid to main fork (at func assemblyRequest(), command==message).
				buf_forks.flush();			//dont forget flush filestream, otherwise assemblyRequest() cant gets uuid
					
				s+=" ok. UUID_session="+std::to_string(cl.UUID_session);
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
		case (commands::message): {		//server sended us some msg from other users
			s+=" "+j.value("sender_login","oops")+": "+j.value("body","oops");	//+UUID_session
			//todo - add responce to server about confirming recieving
			//assemblyRequest(commands::message_reply);
			//sendRequest();
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
			//	close(cl.sock);		//commented for fork can display logout_reply message. fork termninate in main process.
			//	exit(0);

			}
			
			break;
		}
	
	}
	return s;
}

void chat_client::processing() {
	int command=0;
	pid_t child_fork;
	
	//fork for real-time displaying messages from server.
	//in fork case  we obtain raw messages, parsing it, and execute needed actions
	switch (child_fork=fork()) {
		case -1:
			perror("fork error");
			break;
		case 0: {	//fork processing
			int bytes_read=1;
						
			while (1) {
				bytes_read=recv(cl.sock,response,BUF_SIZE,0);
				if (bytes_read<=0) break;
				std::cout<<parseResponse()<<std::endl;	
			}
			close(cl.sock);
					
			exit(0);
		}				
		default:{	//primary processing	
			while (1) {
				command=getCommand();		
				if (commands::message==command) {
					std::cout<<"input message:\n";
					std::getline(std::cin,msg_body);
					std::cout<<std::endl;			
				}
				assemblyRequest(command);
				sendRequest();	
				
				//quit processing
				if (commands::logout==command) {
					sleep(1);				//waiting for fork process logout_reply. but why?
					if (kill(child_fork, SIGKILL)) {	//fork, u ll come with me!
						perror("cant terminate child fork");
						exit(1);
					}
					std::cout<<"exit\n";								
					break;
					
				}					
				command=255;			//just reseting for next iteration
			}
		} break;					
	}	
}
chat_client::chat_client() {
	cl.id_msg=1;
	cl.user.login="user1";
	cl.user.password="111";
	cl.UUID_session=0;


	struct sockaddr_in peer;
	cl.sock = socket( AF_INET, SOCK_STREAM, 0 );
	if (cl.sock<0) {
		perror("client.socket() error");
		std::exit(-1);
	}
	

	peer.sin_family = AF_INET;
	peer.sin_port = htons(SERVER_PORT);
	peer.sin_addr.s_addr = inet_addr(SERVER_IP);

	int res = connect(cl.sock,(struct sockaddr*) &peer,sizeof(peer));
	if ( res ) {
		perror("client.connect() error\n");
		std::exit(-2);
	}
	
	inputUser(&cl.user); 
	

	//for transmiting data between forks
	buf_filename = "buf_forks_"+std::to_string(getpid())+".buf";	//make differents filenames for local execution multiply clients
	buf_forks.open(buf_filename,std::ios::out|std::ios::in|std::ios::trunc);	
	if (!buf_forks.is_open()) {
		perror("cant initialize buffer file");
		exit(1);
	}
}

chat_client::~chat_client() {	
	buf_forks.close();
	std::remove(buf_filename.c_str());
	if (cl.sock) close(cl.sock);
}

void printStart() {
	std::cout<<"simple tcp-chat by Nerftd client-version\n"<<
		"commands:\n"<<
		"0 - Hello\n"<<
		"1 - Login\n"<<
		"2 - Message\n"<<
		"3 - Ping\n"<<
		"4 - Logout\n"<<
		"Server - "<<SERVER_IP<<":"<<SERVER_PORT<<std::endl;
}

commands chat_client::getCommand()
{
    while (1)
    {
        char cmd;
        std::cin>>cmd;
        std::cin.ignore(32767,'\n');	//if input too much clear buf
 	switch (cmd) {
		case('0'):
			return commands::hello;
		case('1'):
			return commands::login;
		case('2'):
			return commands::message;
		case('3'):
			return commands::ping;
		case('4'):
			return commands::logout;
		default:{
			std::cout<<"invalid input,try again\n";
			break;
		}
	}
    }		   
}



void chat_client::sendRequest() {
	int res = send(cl.sock,request,sizeof(request),0);
	if (res<=0) {
		perror("client.send() error");
	}
	cl.id_msg++;
}

