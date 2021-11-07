#include "../include/agg_client.h"

agg_client::agg_client(int s) {
        UUID_session=0;
        user.login="dummy";
        user.password="dummy";
        user.authorized=0;
	sock=s;
        id_msg=1;
	
}


void inputUser(chat_user* us) {		//no check input
	std::cout<<"input username:\n";
	std::cin>>us->login;
	
	std::cout<<"input password:\n";
	std::cin>>us->password;		//fixme - implement private input
	std::cout<< std::endl;
}

chat_user initUser(int k) {	
	chat_user usr;
	usr.authorized=0;
        usr.login="user"+std::to_string(k);
        usr.password=std::to_string(k*111);
        std::cout<<"login:"<<usr.login<<" password:"<<usr.password<<std::endl;
	return usr;
}

int authorizeUser(std::vector<chat_user>::iterator  it, chat_user* guest) {
        for (int i=0;i<MAX_USERS;i++,it++) 
                if ((it->login==guest->login) and (it->password==guest->password) and (!it->authorized)) {
                        it->authorized=1;
                        return 1 ;
                }
        return 0;
}

void clearAgg_client(std::vector<chat_user>::iterator  it, agg_client& cl) {
	for (int i=0;i<MAX_USERS;i++,it++)
		if (it->login==cl.user.login)
			it->authorized=0;

	cl.UUID_session=0;
	cl.user.login="deleted";
	cl.user.password="deleted";
	cl.id_msg=1;

}




