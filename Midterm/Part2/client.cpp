#include <iostream>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string>
#include <sstream>
#include <cassert>
 #include <fcntl.h>
#define PORT 8888
#define MAXLINE 1024
using namespace std;

int main(int argc, char *argv[])
{
    if(argc != 3){
        cerr << "Usage: ip_address port" << endl; exit(0); 
    } 
    char *serverIp = argv[1]; int port = atoi(argv[2]); 

    int isfirsttime = 0;
    int sockfd;
    char buffer[MAXLINE];
    string s = "Hello Server";
    char * message = new char[s.size()+1];
    copy(s.begin(), s.end(), message);
    message[s.size()] = '\0';
    struct sockaddr_in servaddr;
    int n;
    socklen_t* len;
    if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        printf("socket creation failed");
        exit(0);
    }
    memset(&servaddr, 0, sizeof(servaddr));

    int sockfd_udp;
    char buffer_udp[MAXLINE];
    struct sockaddr_in servaddr_udp;
    int n_udp;
    socklen_t* len_udp;

    if ((sockfd_udp = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        printf("socket creation failed");
        exit(0);
    }
    memset(&servaddr_udp, 0, sizeof(servaddr_udp));

    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);
    servaddr.sin_addr.s_addr = inet_addr(serverIp);
    if (connect(sockfd, (struct sockaddr*)&servaddr,
                            sizeof(servaddr)) < 0) {
        printf("\n Error : tcp Connect Failed \n");
    }
    int bytesWritten=0, bytesRead=0;
    servaddr_udp.sin_family = AF_INET;
    servaddr_udp.sin_port = htons(port);
    servaddr_udp.sin_addr.s_addr = inet_addr(serverIp);
    if(connect(sockfd_udp, (struct sockaddr *)&servaddr_udp, sizeof(servaddr_udp)) < 0){
        printf("\n Error : udp Connect Failed \n");
        exit(0);
    }

    while(true){
        if(isfirsttime==0){
            string data;
            data = "Hello server!";
            
            memset(&buffer, 0, sizeof(buffer));
            sendto(sockfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(sockfd));

            recv(sockfd, (char*)&buffer, sizeof(buffer), 0);

            cout<<buffer<<endl;
            isfirsttime = 1;
            continue;
        }else{
            cout<<"%";
            string S,T;
            getline(cin,S);
            stringstream X(S); 
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
            
            /*

            The type of arg is char**
            , this array aims to separate the command so that we could check the first word and see if it meets our requirement.
            You could implement in your own way.

            */
            
            if(strcmp(arg[0],"list-users")==0){  
                
                memset(buffer, 0, MAXLINE);
                //cout<<"haha"<<endl;
                sendto(sockfd, S.c_str(), strlen(S.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(sockfd));
                recv(sockfd, (char*)&buffer, sizeof(buffer), 0);
                cout<<buffer<<endl;

                continue;
                
            }else if(strcmp(arg[0],"exit")==0){ //tcp
                memset(buffer, 0, MAXLINE);
                //cout<<"haha"<<endl;
                sendto(sockfd, S.c_str(), strlen(S.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(sockfd));
                recv(sockfd, (char*)&buffer, sizeof(buffer), 0);
                cout<<"Bye, user"<<buffer<<endl;

                //...

                break;
            }else if(strcmp(arg[0],"get-ip")==0){ //tcp
                memset(buffer, 0, MAXLINE);
                //cout<<"haha"<<endl;
                sendto(sockfd, S.c_str(), strlen(S.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(sockfd));
                recv(sockfd, (char*)&buffer, sizeof(buffer), 0);
                cout<<buffer<<endl;
                //...

                continue;
            }
            //...
        }
        
    }
    
    
}