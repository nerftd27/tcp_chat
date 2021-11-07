#ifndef CHAT_CLIENT_H
#define CHAT_CLIENT_H

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#include <iostream>
#include <string>
#include <unistd.h>     //close(socket)
#include <stdio.h>      //STDIN_FILENO
#include "json.hpp"
#include "agg_client.h"
#include <fstream>
#include <cstdlib>
#include <csignal>

using json = nlohmann::json;

const char SERVER_IP[]="127.0.0.1";
const int SERVER_PORT=7500;


class chat_client {
        agg_client cl;
        int unsigned id_srv_msg;
        std::fstream buf_forks;                 //for transmiting data between forks (UUID_session)
        std::string buf_filename;


        char request[BUF_SIZE];
        char response[BUF_SIZE];
        std::string msg_body;

        commands getCommand();
        int assemblyRequest(int command=0);
        void sendRequest();
        std::string parseResponse();
public:
        chat_client();
        ~chat_client();
        void processing();
};

void printStart();


#endif	//CHAT_CLIENT_H
