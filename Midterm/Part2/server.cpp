#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream> 
#include <vector>
#include <errno.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h>
#include <time.h>
#include <sstream>
#include <algorithm>
#include <map>
#include <vector>

#define TRUE 1
#define FALSE 0
#define MAXLINE 1024

using namespace std;


int main(int argc , char *argv[])
{
    if(argc != 2){
        cerr << "Please give: port" << endl;
        exit(0);
    }
    int PORT = atoi(argv[1]);

    struct sockaddr_in cliaddr;
	struct sockaddr_in address;
    socklen_t len;
	int opt = TRUE;
	int master_socket, addrlen, new_socket, max_clients = 30, activity, i, valread, sd;
	int max_sd;
	int client_socket[30];
	char* ip_record[30];
	int port_record[30];
	int live[30];

	char buffer[MAXLINE]; //data buffer of 1K
		
	//set of socket descriptors
	fd_set readfds;
	
	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < max_clients; i++){
		client_socket[i] = 0;
		//ip_record[i] = '0';
		port_record[i] = 0;
		live[i] = 0;
	}

	//create a master socket
	if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0){
		perror("socket failed");
		exit(EXIT_FAILURE);
	}
	
	if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt,sizeof(opt)) < 0 ){
		perror("setsockopt");
		exit(EXIT_FAILURE);
	}
	
	//type of socket created
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons( PORT );
		
	//bind the socket to localhost port 8888
	bind(master_socket, (struct sockaddr *)&address, sizeof(address));
	//("Listener on port %d \n", PORT);
    cout<<"TCP server is running"<<endl;

	//try to specify maximum of 30 pending connections for the master socket
	if (listen(master_socket, 30) < 0){
		perror("listen");
		exit(EXIT_FAILURE);
	}
		
	//accept the incoming connection
	addrlen = sizeof(address);
	//puts("Waiting for connections ...");
	//map<int,int> mapping; //new_socket, user number

	int number = 1;
	while(TRUE)
	{
		FD_ZERO(&readfds);
		FD_SET(master_socket, &readfds);

        max_sd = master_socket;
		for ( i = 0 ; i < max_clients ; i++){
			sd = client_socket[i];
			if(sd > 0)
				FD_SET( sd , &readfds);
			if(sd > max_sd)
				max_sd = sd;
		}

		activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
		if ((activity < 0) && (errno!=EINTR)){
			printf("select error");
		}
			
		//tcp_not_connect
		if (FD_ISSET(master_socket, &readfds)){
			if ((new_socket = accept(master_socket,(struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
				perror("accept");
				exit(EXIT_FAILURE);
			}
			//inform user of socket number - used in send and receive commands
			string data = "Welcome, you are uesr";
			data += to_string(number);
			cout<<"New connection from "<<inet_ntoa(address.sin_addr)<<":"<<ntohs(address.sin_port)<<" user"<<number<<endl;
			//send new connection greeting message
			sendto(new_socket, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(address));
			//for (i = 0; i < max_clients; i++){
				//if( client_socket[i] == 0 ){
					client_socket[number] = new_socket;
					ip_record[number] = inet_ntoa(address.sin_addr);
					port_record[number] = ntohs(address.sin_port);
					live[number] = 1;
					//break;
				//}
			//}
			number++;
		}
			
		for (i = 1; i < number; i++)
		{
			sd = client_socket[i]; //specific client sd number
			if (FD_ISSET( sd , &readfds)){
				//Check if it was for closing , and also read the
				//incoming message
				memset(buffer,0,sizeof(buffer));
				if ((valread = read( sd , buffer, MAXLINE)) == 0)
				{
					//Somebody disconnected , get his details and print
					getpeername(sd , (struct sockaddr*)&address , \
						(socklen_t*)&addrlen);
					cout<<"user"<<i<<" "<<ip_record[i]<<":"<<port_record[i]<<" disconnected"<<endl;
						
					//Close the socket and mark as 0 in list for reuse
					live[i] = 0;
                    //cout<<i<<endl;
					close( sd );
					//clientFlag[i] = 0; //I ADD
				}
				else{
                    string T;
                    stringstream X(buffer); 
                    char** arg = new char*[100];
                    int length=0;
                    while (getline(X, T, ' ')) { 
                        char * writable = new char[T.size()+1];
                        copy(T.begin(), T.end(), writable);
                        writable[T.size()] = '\0';
                        arg[length] = writable;
                        length++;
                    } 
                    arg[length] = NULL;
				
                    if(strcmp(arg[0],"list-users")==0){                        
                        string data = "";
						for (int k = 1; k < number; k++){
							if(live[k]){
								data += "user";
								data += to_string(k);
								data += "\n";
							}
						}
                        sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
					}
					else if(strcmp(arg[0],"get-ip")==0){                        
                        string data;
                        data = "IP: ";
						data += ip_record[i];
						data += ":";
						data += to_string(port_record[i]);
                        sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
					}
				}
			}
		}
	}
		
	return 0;
}