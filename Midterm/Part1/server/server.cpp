#include <stdio.h>
#include <string.h> //strlen
#include <stdlib.h>
#include <string>
#include <string.h>
#include <iostream> 
#include <vector>
#include <errno.h>
#include <unistd.h> //close
#include <arpa/inet.h> //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <time.h>
#include <bits/stdc++.h>
#include <filesystem>
	
#define TRUE 1
#define FALSE 0
#define PORT 8888
#define MAXLINE 1024

namespace fs = std::filesystem;
using namespace std;


int max(int a, int b){
    if(a>=b){
        return a;
    }else{
        return b;
    }
}

int main(int argc , char *argv[])
{
	if(argc != 2){
        cerr << "Please give: port" << endl;
        exit(0);
    }
    int port = atoi(argv[1]);

    struct sockaddr_in cliaddr;
    socklen_t len;
	int opt = TRUE;
	int master_socket , addrlen , new_socket , client_socket[30] ,
		max_clients = 30 , activity, i , valread , sd;
	int max_sd;
	struct sockaddr_in address;
		
	char buffer[1025]; //data buffer of 1K
		
	//set of socket descriptors
	fd_set readfds;
	//char *message = "link start!\r\n";
	
	//initialise all client_socket[] to 0 so not checked
	for (i = 0; i < max_clients; i++){
		client_socket[i] = 0;
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
	address.sin_port = htons( port );
		
	//bind the socket to localhost port 8888
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	//printf("Listener on port %d \n", port);
    //cout<<"TCP server is running"<<endl;

    // create UDP socket
    int udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    // binding server addr structure to udp sockfd
    bind(udpfd, (struct sockaddr*)&address, sizeof(address));
    //cout<<"UDP server is running"<<endl;

	//try to specify maximum of 30 pending connections for the master socket
	if (listen(master_socket, 30) < 0){
		perror("listen");
		exit(EXIT_FAILURE);
	}
		
	//accept the incoming connection
	addrlen = sizeof(address);
	//puts("Waiting for connections ...");
		
	while(TRUE)
	{
		FD_ZERO(&readfds);
		FD_SET(master_socket, &readfds);
        FD_SET(udpfd, &readfds);

        //max
        max_sd = max(udpfd, master_socket);
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

        //udp
        if (FD_ISSET(udpfd, &readfds)){
            len = sizeof(cliaddr);
            memset(&buffer, 0, sizeof(buffer));
            recvfrom(udpfd, buffer, sizeof(buffer), 0, (struct sockaddr*)&cliaddr, &len);
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
			// string data;
			// data = "Usage: register <username> <email> <password>";
			// sendto(udpfd, data.c_str(), MAXLINE, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
            if(strcmp(arg[0],"get-file-list")==0){
                string path = fs::current_path(), s = "Files: ";
				//string s;
				for (const auto & entry : fs::directory_iterator(".")) {
					string temp  = entry.path();
					temp.erase(temp.begin()); temp.erase(temp.begin());
					s += temp;
					s += " ";
				}
				s += "\n";
				sendto(udpfd, s.c_str(), sizeof(buffer), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
            }else if(strcmp(arg[0],"get-file")==0){
                string arg;
                vector<string> args;
                args.clear();
                stringstream ss(buffer);
                while (getline(ss, arg, ' ')) {
                    args.push_back(arg);
                }
                int l = args.size();
                for (int i = 1; i < l; i++) {
                    if (args[i][args[i].length()-1] == '\n')
                        args[i].pop_back();
                    sendto(udpfd, args[i].c_str(), sizeof(args[i]), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
                    bzero(buffer, sizeof(buffer));
                    stringstream ts; ts << args[i];
                    ifstream fin(ts.str());
                    stringstream ssf;
                    ssf << fin.rdbuf();
                    string content = ssf.str();
                    sendto(udpfd, content.c_str(), sizeof(content), 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
                }
            }
        }
			
		//tcp_not_connect
		if (FD_ISSET(master_socket, &readfds)){
			if ((new_socket = accept(master_socket,(struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
				perror("accept");
				exit(EXIT_FAILURE);
			}
			//inform user of socket number - used in send and receive commands
			//printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
		
			//send new connection greeting message
			if(write(new_socket, "*****Welcome to Game 1A2B*****", 99)!=99){
				perror("send");
			}
				
			//puts("Welcome message sent successfully");
			//write(new_socket, "% ", 2);
			
			for (i = 0; i < max_clients; i++){
				if( client_socket[i] == 0 ){
					client_socket[i] = new_socket;
					//printf("Adding to list of sockets as %d\n" , i);
					break;
				}
			}
		}

		//tcp	
		for (i = 0; i < max_clients; i++){
			sd = client_socket[i];
			if (FD_ISSET( sd , &readfds)){
				//Check if it was for closing , and also read the
				//incoming message
				memset(buffer,0,sizeof(buffer));//�ڥ[�� 
				if ((valread = read( sd , buffer, 1024)) == 0)
				{
					//Somebody disconnected , get his details and print
					getpeername(sd , (struct sockaddr*)&address , \
						(socklen_t*)&addrlen);
					//printf("Host disconnected , ip %s , port %d \n" ,
						//inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
						
					//Close the socket and mark as 0 in list for reuse
					close( sd );
					client_socket[i] = 0;
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
                    if(strcmp(arg[0],"login")==0){                        
                        string data;
                        data = "Usage: login <username> <password>";
                        sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
						printf("socket fd is %d , ip is : %s , port : %d\n" , sd , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
					}
                    // ...
				}
			}
		}
	}
		
	return 0;
}

