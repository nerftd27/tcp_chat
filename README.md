# tcp_chat
simple tcp chat with single server and multiply clients. 

Server processing based on non-blocking sokects (fcntl and select). Client implement 2 threads - main and child fork. 
In main process assembling and sending data to server, and child fork displays all incoming messages from server.
All realized without GUI, only linux terminal. 
Communication goes by json tokens, json parser implemented in json.hpp (by nlohmann)

