#ifndef CHAT_SERVER_H
#define CHAT_SERVER_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#include <cstdlib>
#include <unistd.h> //close socket

#include "json.hpp"
#include <fcntl.h>
#include <algorithm>
#include "agg_client.h"

using json = nlohmann::json;

class chat_server {
        int listener;
        std::vector<agg_client> accepted_sockets;

        char request[BUF_SIZE];
        char response[BUF_SIZE];

        int bytes_read;
        int broadcast;                          //triger for broadcast sending
        int id_cl_msg;
        std::vector<chat_user> dbUsers;         //storage here references logins and passwords 
        std::string msg_body;                   //for broadcast sending
        std::string sender_login;               //for broadcast sending

        int getListenSocket();
        commands parseRequest(std::vector<agg_client>::iterator it);
        int sendResponse(std::vector<agg_client>::iterator it);
public:
        chat_server();
        ~chat_server();
        void Processing();
};

void printStartSrv();
#endif	//CHAT_SERVER_H
