// #include <stdio.h>
// #include <stdlib.h>
// #include <string.h>
// #include <unistd.h>
// #include <arpa/inet.h>

// int main(){

//   char *ip = "127.0.0.1";
//   int port = 8888;

//   int sock;
//   struct sockaddr_in addr;
//   socklen_t addr_size;
//   char buffer[1024];
//   int n;

//   sock = socket(AF_INET, SOCK_STREAM, 0);
//   if (sock < 0){
//     perror("[-]Socket error");
//     exit(1);
//   }
//   //printf("[+]TCP server socket created.\n");

//   memset(&addr, '\0', sizeof(addr));
//   addr.sin_family = AF_INET;
//   addr.sin_port = port;
//   addr.sin_addr.s_addr = inet_addr(ip);

//   int con = connect(sock, (struct sockaddr*)&addr, sizeof(addr));
//   if(con == -1) {
//     //printf("Error with connection");
//   }
//   //printf("Connected to the server.\n");

//   bzero(buffer, 1024);
//   strcpy(buffer, "*****Welcome to Game 1A2B*****");
//   printf("%s\n", buffer);
//   //printf("Client: %s\n", buffer);
//   int toServer = send(sock, buffer, strlen(buffer), 0);
//   if(toServer == -1){
//     printf("failed to send");
//   }
//   printf("%s\n", buffer);

//   bzero(buffer, 1024);
//   recv(sock, buffer, sizeof(buffer), 0);
//   //printf("Server: %s\n", buffer);

//   //close(sock);
//   //printf("Disconnected from the server.\n");

//   return 0;

// }




// #include <iostream>
// #include <string>
// #include <stdio.h>
// #include <sys/types.h>
// #include <sys/socket.h>
// #include <netinet/in.h>
// #include <arpa/inet.h>
// #include <stdlib.h>
// #include <unistd.h>
// #include <string.h>
// #include <netdb.h>
// #include <sys/uio.h>
// #include <sys/time.h>
// #include <sys/wait.h>
// #include <fcntl.h>
// #include <fstream>
// using namespace std;
// //Client side
// int main(int argc, char *argv[])
// {
//     //we need 2 things: ip address and port number, in that order
//     if(argc != 3)
//     {
//         cerr << "Usage: ip_address port" << endl; exit(0); 
//     } //grab the IP address and port number 
//     char *serverIp = argv[1]; int port = atoi(argv[2]); 
//     //create a message buffer 
//     char msg[1500]; 
//     //setup a socket and connection tools 
//     struct hostent* host = gethostbyname(serverIp); 
//     struct sockaddr_in sendSockAddr;   
//     bzero((char*)&sendSockAddr, sizeof(sendSockAddr)); 
//     sendSockAddr.sin_family = AF_INET; 
//     sendSockAddr.sin_addr.s_addr = 
//         inet_addr(inet_ntoa(*(struct in_addr*)*host->h_addr_list));
//     sendSockAddr.sin_port = htons(port);
//     int clientSd = socket(AF_INET, SOCK_STREAM, 0);

//     if (connect(sockfd, (struct sockaddr*)&servaddr,
//                             sizeof(servaddr)) < 0) {
//         printf("\n Error : Connect Failed \n");
//     }
 

//     //try to connect...
//     cout<<"try to connect"<<endl;
//     int status = connect(clientSd,
//                          (struct sockaddr *)&sendSockAddr, sizeof(sendSockAddr));
//     if(status < 0)
//     {
//         cout<<"Error connecting to socket!"<<endl;
//         return -1;
//     }
//     cout << "*****Welcome to Game 1A2B*****" << endl;
//     int bytesRead, bytesWritten = 0;
//     struct timeval start1, end1;
//     gettimeofday(&start1, NULL);
//     while(1)
//     {
//         cout << ">";
//         string data;
//         getline(cin, data);
//         memset(&msg, 0, sizeof(msg));//clear the buffer
//         strcpy(msg, data.c_str());
//         if(data == "exit")
//         {
//             send(clientSd, (char*)&msg, strlen(msg), 0);
//             break;
//         }
//         bytesWritten += send(clientSd, (char*)&msg, strlen(msg), 0);
//         cout << "Awaiting server response..." << endl;
//         memset(&msg, 0, sizeof(msg));//clear the buffer
//         bytesRead += recv(clientSd, (char*)&msg, sizeof(msg), 0);
//         if(!strcmp(msg, "exit"))
//         {
//             cout << "Server has quit the session" << endl;
//             break;
//         }
//         cout << "Server: " << msg << endl;
//     }
//     gettimeofday(&end1, NULL);
//     close(clientSd);
//     cout << "********Session********" << endl;
//     cout << "Bytes written: " << bytesWritten << 
//     " Bytes read: " << bytesRead << endl;
//     cout << "Elapsed time: " << (end1.tv_sec- start1.tv_sec) 
//       << " secs" << endl;
//     cout << "Connection closed" << endl;
//     return 0;    
// }





// TCP Client program
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

    int i;
    int isconnect = 0;
    int isfirsttime = 0;
    int is_login = 0;
    char username[100];

    int gamemode = 0;
    int guesstime = 0;
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
            //if(gamemode==0) cout<<"%";
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

            if(strcmp(arg[0],"register")==0){  //udp
                
                memset(buffer, 0, MAXLINE);
                sendto(sockfd_udp, S.c_str(), strlen(S.c_str()), 0, (const struct sockaddr*)&servaddr_udp, sizeof(servaddr_udp));
                recvfrom(sockfd_udp, buffer, sizeof(buffer), 0, (struct sockaddr*)&servaddr_udp, NULL);
                cout<<buffer<<endl;
                continue;
                
            }else if(strcmp(arg[0],"login")==0){ //tcp
                memset(buffer, 0, MAXLINE);
                sendto(sockfd, S.c_str(), strlen(S.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(sockfd));
                recv(sockfd, (char*)&buffer, sizeof(buffer), 0);

                string T1;
                stringstream X1(buffer); 
                char** arg1 = new char*[100];
                int length1=0;
                while (getline(X1, T1, ' ')) { 
                    char * writable1 = new char[T1.size()+1];
                    copy(T1.begin(), T1.end(), writable1);
                    writable1[T1.size()] = '\0';
                    arg1[length1] = writable1;
                    length1++;
                } 
                arg1[length1] = NULL;
                if(strcmp(arg1[0],"Welcome,")==0){
                    is_login = 1;
                    strcat(username,arg[1]);
                }

                cout<<buffer<<endl;
                continue;
            }else if(strcmp(arg[0],"game-rule")==0){ //udp
                memset(&buffer, 0, sizeof(buffer));
                sendto(sockfd_udp, S.c_str(), strlen(S.c_str()), 0, (const struct sockaddr*)&servaddr_udp, sizeof(servaddr_udp));
                recvfrom(sockfd_udp, buffer, sizeof(buffer), 0, (struct sockaddr*)&servaddr_udp, NULL);
                cout<<buffer<<endl;
                continue;
            }else if(strcmp(arg[0],"start-game")==0){
                memset(buffer, 0, MAXLINE);
                sendto(sockfd, S.c_str(), strlen(S.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(sockfd));
                recv(sockfd, (char*)&buffer, sizeof(buffer), 0);
                cout<<buffer<<endl; //Please typing a 4-digit number:
                if(strcmp(buffer,"Please login first.")==0 || strcmp(buffer,"Usage: start-game <4-digit number>")==0){
                    continue;
                }else{
                    gamemode=1;
                }

            }else if(strcmp(arg[0],"logout")==0){
                memset(buffer, 0, MAXLINE);
                sendto(sockfd, S.c_str(), strlen(S.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(sockfd));
                recv(sockfd, (char*)&buffer, sizeof(buffer), 0);
                cout<<buffer<<endl;
                continue;
            }else if(gamemode==1){
                memset(buffer, 0, MAXLINE);
                sendto(sockfd, S.c_str(), strlen(S.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(sockfd));
                
                recv(sockfd, (char*)&buffer, sizeof(buffer), 0);
                if(strcmp(buffer,"Your guess should be a 4-digit number.")==0){
                    cout<<buffer<<endl;
                    continue;
                }
                guesstime++;
                if(strcmp(buffer,"You got the answer!")==0){
                    gamemode = 0;
                    cout<<buffer<<endl;
                    continue;
                }
                string T1;
                stringstream X1(buffer); 
                char** arg1 = new char*[100];
                int length1=0;
                while (getline(X1, T1, ' ')) { 
                    char * writable1 = new char[T1.size()+1];
                    copy(T1.begin(), T1.end(), writable1);
                    writable1[T1.size()] = '\0';
                    arg1[length1] = writable1;
                    length1++;
                } 
                arg1[length1] = NULL;
                cout<<buffer<<endl;
                if(guesstime==5){
                    gamemode = 0;
                    guesstime = 0;
                    continue;
                }
                
            }else if(strcmp(arg[0],"exit")==0){
                sendto(sockfd, S.c_str(), strlen(S.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(sockfd));
                break;
            }
        }
        
    }
    
    
}