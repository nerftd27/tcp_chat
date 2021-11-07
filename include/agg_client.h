#ifndef AGG_CLIENT_H
#define AGG_CLIENT_H

#include <iostream>
#include <vector>

const int BUF_SIZE=1024;
const int MAX_USERS=5;

enum commands {hello=0, login=1, message=2, ping=3, logout=4,
               hello_reply=5, login_reply=6, message_reply=7, ping_reply=8, logout_reply=9,
              };

struct chat_user {
        std::string login;
        std::string password;
        int authorized;
};

struct agg_client {             //agregate class, shared both chat_server and chat_client
        long UUID_session;
        chat_user user;
        int sock;
        unsigned int id_msg;
        agg_client (int s=0);
};

void inputUser(chat_user* us);
chat_user initUser(int k);
int authorizeUser(std::vector<chat_user>::iterator  it, chat_user* guest);
void clearAgg_client(std::vector<chat_user>::iterator  it, agg_client& cl);

#endif	//AGG_CLIENT_H
