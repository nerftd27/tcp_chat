all:chat_server chat_client

chat_server: agg_client.o chat_server.o main_chat_server.o
	g++ agg_client.o chat_server.o main_chat_server.o -o chat_server

chat_client: agg_client.o chat_client.o main_chat_client.o
	g++ agg_client.o chat_client.o main_chat_client.o -o chat_client

agg_client.o: ./source/agg_client.cpp
	g++ -c -g -Wall ./source/agg_client.cpp

chat_server.o: ./source/chat_server.cpp
	g++ -c -g -Wall ./source/chat_server.cpp

main_chat_server.o: ./source/main_chat_server.cpp
	g++ -c -g -Wall ./source/main_chat_server.cpp

chat_client.o: ./source/chat_client.cpp
	g++ -c -g -Wall ./source/chat_client.cpp

main_chat_client.o: ./source/main_chat_client.cpp
	g++ -c -g -Wall ./source/main_chat_client.cpp

clean:
	rm -rf *.o chat_client chat_server

