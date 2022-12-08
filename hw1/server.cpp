#include <iostream>
#include <arpa/inet.h>
#include <errno.h>
#include <netinet/in.h>
#include <signal.h>
#include <stdio.h>
#include <stdlib.h>
#include <strings.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <string>
#include <cstring>
#include <pthread.h>
#include <sstream>
#include <sys/ipc.h>
#include <sys/shm.h>
#include <sys/wait.h>
#include <sys/time.h>
#include <stdint.h>
#include <cassert>
#include <fcntl.h>
#include <string>
#include <cstring>
#include <exception>
#define PORT 8888
#define MAXLINE 1024
#define SIZE 19200000

using namespace std;

struct user{
    char username[20][MAXLINE];
    char email[20][MAXLINE];
    char password[20][MAXLINE];
    int login[20];
    int num_of_user;
    int num_of_client;
    //int port[MAXLINE];
    //char* port_user[MAXLINE];
    //int connfd[20];
};

int max(int x, int y){
    if (x > y)
        return x;
    else
        return y;
}

int check_username_exit(struct user* user_array, char* username, int num_of_user){
    int index=-1;
    for(int i=0;i<num_of_user;i++){
        //cout<< "i" << i <<user_array->username[i]<<endl;
        if(strcmp(user_array->username[i],username)==0){
            index=i;
            break;
        }
    }
    return index;
}

int check_email_exit(struct user* user_array, char* email, int num_of_user){
    int index=-1;
    for(int i=0;i<num_of_user;i++){
        if(strcmp(user_array->email[i],email)==0){
            index=i;
            break;
        }
    }
    return index;
}

// int check_port_exit(struct user* user_array, int port, int num_of_user){
//     int index=-1;
//     for(int i=0;i<num_of_user;i++){
//         if(user_array->port[i]==port){
//             index=i;
//             break;
//         }
//     }
//     return index;
// }

int main(int argc, char *argv[])
{
    srand(time(NULL));
    int listenfd, udpfd, connfd, nready, maxfdp1 = 0, listenfd1;
    char buffer[MAXLINE];
    pid_t childpid;
    fd_set rset;
    socklen_t len;
    struct sockaddr_in cliaddr, servaddr;
    int shmid;
    struct user* user_array;

    if(argc != 2){
        cerr << "Please give: port" << endl;
        exit(0);
    }
    int port = atoi(argv[1]);

    //shared memory
    shmid = shmget(IPC_PRIVATE, SIZE, IPC_CREAT|0600 ) ;
    void *shm = shmat( shmid, NULL, 0 ) ;
    user_array = (struct user*)shm;

    for(int i = 0; i < 30; i++){
        user_array->login[i] = 0;
    }
    user_array->num_of_client = 0;
    user_array->num_of_user = 0;

    //create listening TCP socket
    listenfd = socket(AF_INET, SOCK_STREAM, 0); //blocking
    bzero(&servaddr, sizeof(servaddr));
    int opt = 1; 
    if( setsockopt(listenfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ){  
        perror("setsockopt");  
        exit(EXIT_FAILURE);  
    } 
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
    servaddr.sin_port = htons(port);

    // binding server addr structure to listenfd
    bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    listen(listenfd, 100);
    cout<<"TCP server is running"<<endl;

    // create UDP socket
    udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    // binding server addr structure to udp sockfd
    bind(udpfd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    cout<<"UDP server is running"<<endl;

    pid_t pid;
    maxfdp1 = listenfd + 1;
    
    for (;;) {
        // set listenfd and udpfd in readset
        FD_SET(listenfd, &rset);
        // select the ready descriptor
        nready = select(maxfdp1, &rset, NULL, NULL, NULL); 
        connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &len); //accept tcp connection
        string data;
        data = "*****Welcome to Game 1A2B*****";
        memset(&buffer, 0, sizeof(buffer));
        recv(connfd, (char*)&buffer, sizeof(buffer), 0);
        sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
        cout<<"New connection"<<endl;
 
        pid = fork();
        int id = user_array->num_of_client;
        if(pid==0){
            //cout<<user_array->num_of_client<<endl;
            //cout<<user_array->num_of_user<<endl;
            //int connfd = connfd;
            int gamemode = 0;
            int guesstime = 0;
            char answer[MAXLINE];
            char name[MAXLINE] = {'\0'};
            //string name = "";
            user_array->num_of_client++;
            while(true){
                FD_SET(connfd, &rset);
                FD_SET(udpfd, &rset);
                int maxfdp = max(connfd, udpfd) + 1;
                nready = select(maxfdp, &rset, NULL, NULL, NULL);
                if (FD_ISSET( connfd , &rset)){  
                    len = sizeof(cliaddr);
                    memset(&buffer, 0, sizeof(buffer));
                    recv(connfd, (char*)&buffer, sizeof(buffer), 0);
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
                    
                    if(gamemode == 1){
                        string guess_string(arg[0]);
                        string answer_string(answer);
                        int guess;
                        int ans = stoi(answer_string);
                        try{
                            guess = stoi(guess_string);
                        }catch(exception &e){
                            data = "Your guess should be a 4-digit number.";
                            sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                            continue;
                        }                                    
                        guess = stoi(guess_string);
                        //cout<<guess<<endl;
                        int flag=0;
                        for(int i=0;i<4;i++){
                            if(guess_string[i]<'0' || guess_string[i]>'9'){
                                flag=1;
                                break;
                            }
                        }
                        if(flag==1){
                            data = "Your guess should be a 4-digit number.";
                            sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                            continue;
                        }
                        if(guess_string.size()!=4){
                            data = "Your guess should be a 4-digit number.";
                            sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                            continue;
                        }
                        int guess_array[4];
                        int answer_array[4];
                        int numA=0;
                        int numB=0;
                        int visited[10];
                        for(int i=0;i<10;i++){
                            visited[i]=0;
                        }
                        guess_array[0] = guess / 1000;
                        guess = guess % 1000;
                        guess_array[1] = guess / 100;
                        guess = guess % 100;
                        guess_array[2] = guess / 10;
                        guess = guess % 10;
                        guess_array[3] = guess;

                        answer_array[0] = ans / 1000;
                        ans = ans % 1000;
                        answer_array[1] = ans / 100;
                        ans = ans % 100;
                        answer_array[2] = ans / 10;
                        ans = ans % 10;
                        answer_array[3] = ans;

                        for(int i=0;i<4;i++){
                            if(guess_array[i]==answer_array[i]){
                                numA++;
                                visited[guess_array[i]]=1;
                                continue;
                            }
                        }
                        for(int i=0;i<4;i++){
                            if(visited[guess_array[i]]==1){
                                continue;
                            }
                            for(int j=0;j<4;j++){
                                if(guess_array[i]==answer_array[j] && i!=j){
                                    numB++;
                                    visited[guess_array[i]]=1;
                                    break;
                                }
                            }
                        }

                        // for(int j=0;j<4;j++){
                        //     if(guess_array[j]==answer_array[j]){
                        //         numA++;
                        //         continue;
                        //     }else{
                        //         for(int k=0;k<4;k++){
                        //             if(guess_array[j]==answer_array[k]){
                        //                 numB++;
                        //                 break;
                        //             }
                        //         }
                        //     }
                        // }
                        char _data[100]={'\0'};
                        if(strcmp(buffer,answer)==0){
                            strcat(_data, "You got the answer!");
                            gamemode = 0;
                            guesstime = 0;
                            sendto(connfd, _data, strlen(_data),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                            continue;
                        }
                        strcat(_data, to_string(numA).c_str());
                        strcat(_data, "A");
                        strcat(_data, to_string(numB).c_str());
                        //cout<<"here"<<endl;
                        if(guesstime!=4){
                            strcat(_data, "B");
                            guesstime++;
                            sendto(connfd, _data, strlen(_data),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                        }else{
                            strcat(_data, "B");
                            strcat(_data, "\nYou lose the game!");
                            gamemode = 0;
                            guesstime = 0;
                            sendto(connfd, _data, strlen(_data),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                        }
                            
                        
                    }else if(strcmp(arg[0],"login")==0){                        
                        if(length!=3){
                            string data;
                            data = "Usage: login <username> <password>";
                            sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                        }else{
                            string arg1(arg[1]);
                            string arg2(arg[2]);
                            int index = check_username_exit(user_array,arg[1],user_array->num_of_user);
                            //cout << "index" << index << endl;
                            if(index!=-1){
                                if(user_array->login[index]==1){
                                    string data;
                                    data = "Please logout first.";
                                    sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                                }else if(strcmp(user_array->password[index],arg[2])!=0){
                                    string data;
                                    data = "Password not correct.";
                                    sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                                }else{
                                    // int index_port = check_port_exit(user_array,ntohs(servaddr.sin_port),user_array->num_of_client);
                                    // cout<<"index_port"<<index_port<<endl;
                                    // user_array->port_user[index_port]=user_array->username[index];
                                    user_array->login[index]=1;
                                    //name = arg1;
                                    strcat(name, arg[1]);
                                    char data[100] = {'\0'};
                                    strcat(data, "Welcome, ");
                                    strcat(data, user_array->username[index]);
                                    strcat(data, ".");
                                    sendto(connfd, data, strlen(data),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                                }
                            }else{
                                string data;
                                data = "Username not found.";
                                sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                            }
                        }
                        
                    }else if(strcmp(arg[0],"logout")==0){
                        //int index_port = check_port_exit(user_array,ntohs(servaddr.sin_port),user_array->num_of_client);
                        int index = check_username_exit(user_array,name,user_array->num_of_user);
                        if(user_array->login[index]==0){
                            string data;
                            data = "Please login first.";                            
                            sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                        }else{
                            //Bye, <username>.
                            user_array->login[index]=0;
                            char data[100]={'\0'};
                            strcat(data, "Bye, ");
                            strcat(data, user_array->username[index]);
                            strcat(data, ".");
                            sendto(connfd, data, strlen(data),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                        }
                    }else if(strcmp(arg[0],"start-game")==0){
                        //string arg1(arg[1]);
                        //int index_port = check_port_exit(user_array,ntohs(servaddr.sin_port),user_array->num_of_client);
                        int index = check_username_exit(user_array,name,user_array->num_of_user);
                        //cout<<name<<endl;
                        if(user_array->login[index]==0){
                            data = "Please login first.";
                            sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                            continue;
                        }
                        if(length > 2){
                            string data;
                            data = "Usage: start-game <4-digit number>";
                            sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                            continue;
                        }
                        
                        if(length==1){
                            string data;
                            data = "Please typing a 4-digit number:";     
                            memset(answer, 0, MAXLINE);                               
                            string s = to_string(rand() % (9999 - 1000 + 1) + 1000);
                            copy(s.begin(), s.end(), answer);
                            answer[s.size()] = '\0';
                            gamemode = 1;
                            sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                            
                        }else if(length == 2){ 
                            string data;
                            data = "Please typing a 4-digit number:"; 
                            memset(answer, 0, MAXLINE);
                            strcat(answer,arg[1]);
                            string answer_string(answer);
                            int ans;
                            //cout<<answer_string<<endl;
                            try{
                                ans = stoi(answer_string);
                            }catch(exception &e){
                                data = "Your guess should be a 4-digit number.";
                                sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                                continue;
                            }
                            ans = stoi(answer_string);
                            //cout<<ans;
                            int flag=0;
                            for(int i=0;i<4;i++){
                                if(answer_string[i]<'0' || answer_string[i]>'9'){
                                    flag=1;
                                    break;
                                }
                            }
                            if(flag==1){
                                data = "Your guess should be a 4-digit number.";
                                sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                                continue;
                            }
                            if(answer_string.size()!=4){
                                data = "Your guess should be a 4-digit number.";
                                sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                                continue;
                            }
                            sendto(connfd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&servaddr,sizeof(connfd));
                            gamemode = 1;
                        }
                    }else if(strcmp(arg[0],"exit")==0){
                        // //cout<<"tcp get msg: exit"<<endl;
                        // int index = check_username_exit(user_array,name,user_array->num_of_user);
                        // //cout<<"tcp get msg: exit"<<endl;
                        // if(index!=-1){
                        //     user_array->login[index]=0;
                        // }
                        
                        // cout<<"tcp get msg: exit"<<endl;
                        // close(connfd);
                        // exit(-1);
                        int index = check_username_exit(user_array,name,user_array->num_of_user);
                        if(index != -1){
                            user_array->login[index] = 0;
                            index = -1;
                            //name = "";
                        }
                        cout<<"tcp get msg: exit"<<endl;
                        close(connfd);
                        exit(0);
                    }
                    continue;
                }  

                if (FD_ISSET(udpfd, &rset)) {
                    //cout<<udpfd<<endl;
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
                    if(gamemode==1){
                        string data;
                        data = "Your guess should be a 4-digit number.";
                        sendto(udpfd, data.c_str(), MAXLINE, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
                    }else if(strcmp(arg[0],"register")==0){
                        if(length!=4){
                            string data;
                            data = "Usage: register <username> <email> <password>";
                            sendto(udpfd, data.c_str(), MAXLINE, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
                        }else{
                            // string arg1(arg[1]);
                            // string arg2(arg[2]);
                            // string arg3(arg[3]);
                            if(check_username_exit(user_array,arg[1],user_array->num_of_user)!=-1){
                                string data;
                                data = "Username is already used.";
                                sendto(udpfd, data.c_str(), MAXLINE, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
                            }else if(check_email_exit(user_array,arg[2],user_array->num_of_user)!=-1){
                                string data;
                                data = "Email is already used.";
                                sendto(udpfd, data.c_str(), MAXLINE, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
                            }else{
                                char arg1[1024];
                                strcpy(arg1, arg[1]);
                                char arg2[1024];
                                strcpy(arg2, arg[2]);
                                char arg3[1024];
                                strcpy(arg3, arg[3]);
                                strcpy(user_array->username[user_array->num_of_user] , arg1);
                                strcpy(user_array->email[user_array->num_of_user] , arg2);
                                strcpy(user_array->password[user_array->num_of_user] , arg3);
                                user_array->login[user_array->num_of_user] = 0;
                                user_array->num_of_user++;
                                string data;
                                data = "Register successfully.";
                                sendto(udpfd, data.c_str(), MAXLINE, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
                            }
                            
                        }
                        
                    }else if(strcmp(arg[0],"game-rule")==0){
                        // 1. Each question is a 4-digit secret number.
                        // 2. After each guess, you will get a hint with the following
                        // information:
                        // 2.1 The number of “A”, which are digits in the guess that are
                        // in the correct position.
                        // 2.2 The number of “B”, which are digits in the guess that are
                        // in the answer but are in the wrong position.
                        // The hint will be formatted as “xAyB”.
                        // 3. 5 chances for each question.
                        string data;
                        if(length != 1){
                            data = "Usage: game-rule";
                        }
                        else{
                            data = 
"1. Each question is a 4-digit secret number.\n\
2. After each guess, you will get a hint with the following information:\n\
2.1 The number of \"A\", which are digits in the guess that are in the correct position.\n\
2.2 The number of \"B\", which are digits in the guess that are in the answer but are in the wrong position.\n\
The hint will be formatted as \"xAyB\".\n\
3. 5 chances for each question.";
                        }
                        sendto(udpfd, data.c_str(), MAXLINE, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));

                    }
                }  

            }
            close(connfd);
            exit(0);

        }
        close(connfd);

    }

}
        
