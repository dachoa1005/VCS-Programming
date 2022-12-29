all: client server
server: server.c
	gcc server.c -o server -lssl -lcrypto
client: client.c
	gcc client.c -o client -lssl -lcrypto