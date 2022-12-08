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
#include <algorithm>
#include <map>

	
#define TRUE 1
#define FALSE 0
#define PORT 8888
#define MAXLINE 1024
#define MAXCLIENT 20

using namespace std;





typedef struct public_room{
    string id;
    string manager;
    int status;
    int number_of_rounds;
    string guess_number;
    vector<string>player;
    int current_player;

}public_room;

typedef struct private_room{
    string id;
    string manager;
    string invitation_code;
    int status;
    int number_of_rounds;
    string guess_number;
    vector<string>player;
    int current_player;

}private_room;

typedef struct invitation{
    string inviter;
    string inviter_email;
    string room_id;
    string invitation_code;
}invitation;

typedef struct user{
    string username;
    string email;
    string password;
    int login; // is name login
    vector<struct invitation>invitation;
}user;

typedef struct client{
    int fd;
    struct user user;
    string room;
    int login; // is client login
    int in_room; // is client in room
}client;

// struct user{
//     char username[MAXCLIENT][MAXLINE];
//     char email[MAXCLIENT][MAXLINE];
//     char password[MAXCLIENT][MAXLINE];
//     int login[MAXCLIENT]; //is name login
//     int fd[MAXCLIENT];
//     char name[MAXCLIENT][MAXLINE];
//     int num_of_user;
//     int num_of_client;
//     char user_room[MAXCLIENT][MAXLINE];
//     int login_fd[MAXCLIENT]; //is fd login
    
// };


vector<user>users;
vector<client>clients;
vector<public_room>public_rooms;
vector<private_room>private_rooms;

// vector<string>public_room;
// vector<string>private_room;
// vector<string>private_room_invitation_code;
// vector<int>public_room_status; // 0 for open for players, 1 for start game
// vector<int>private_room_status; // 2 for open for players, 3 for start game

int check_username_exist(string username){ // give username in users
    int index = -1;
    for(int i=0;i<users.size();i++){
        if(users[i].username == username){
            index=i;
            break;
        }
    }
    return index;
}

int check_email_exist(string email){ // give email in users
    int index = -1;
    for(int i=0;i<users.size();i++){
        if(users[i].email == email){
            index=i;
            break;
        }
    }
    return index;
}

int check_fd_exist(int fd1){ // give fd in clients
    int index = -1;
    for(int i=0;i<clients.size();i++){
        if(clients[i].fd == fd1){
            index=i;
            break;
        }
    }
    return index;
}

int check_publicroom_id_exist(string game_id){ //give id in public room
    int index = -1;
    for(int i=0;i<public_rooms.size();i++){
        if(public_rooms[i].id == game_id){
            index=i;
            break;
        }
    }
    return index;
}

int check_privateroom_id_exist(string game_id){ // give id in private room
    int index = -1;
    for(int i=0;i<private_rooms.size();i++){
        if(private_rooms[i].id == game_id){
            index=i;
            break;
        }
    }
    return index;
}

int check_privateroom_invitation_code_exist(string invitation_code){ // give invitation code in private room
    int index = -1;
    for(int i=0;i<private_rooms.size();i++){
        if(private_rooms[i].invitation_code == invitation_code){
            index=i;
            break;
        }
    }
    return index;
}

int check_username_in_client(string username){ // give username in client
    int index = -1;
    for(int i=0;i<clients.size();i++){
        if(clients[i].user.username == username){
            index=i;
            break;
        }
    }
    return index;
}

int check_invitation_exist(struct user user1,string invitation_code){ // give invitation code in each user's invitation queue
    int index = -1;
    for(int i=0;i<user1.invitation.size();i++){
        //<<"####"<<user1.invitation[i].invitation_code<<endl;
        if(user1.invitation[i].invitation_code == invitation_code){
            index=i;
            break;
        }
    }
    //cout<<"onetime"<<endl;
    return index;
}

int find_player_index_in_public_room(int room_index, string player){ 
    int index = -1;
    for(int i=0;i<public_rooms[room_index].player.size();i++){
        if(public_rooms[room_index].player[i] == player){
            index=i;
            break;
        }
    }
    return index;
}

int find_player_index_in_private_room(int room_index, string player){ 
    int index = -1;
    for(int i=0;i<private_rooms[room_index].player.size();i++){
        if(private_rooms[room_index].player[i] == player){
            index=i;
            break;
        }
    }
    return index;
}

int check_invite_repeat(int user_index, string inviter, string room_id){ 
    int index = -1;
    for(int i=0;i<users[user_index].invitation.size();i++){
        if(users[user_index].invitation[i].inviter == inviter && users[user_index].invitation[i].room_id == room_id){
            index=i;
            break;
        }
    }
    return index;
}


int max(int a, int b){
    if(a>=b){
        return a;
    }else{
        return b;
    }
}

bool user_compare(user u1, user u2){
    return u1.username < u2.username;
}

bool invitation_compare(invitation v1, invitation v2){

    //return v1.room_id < v2.room_id;
    char *at = v1.room_id.data();
    char *bt = v2.room_id.data();
    int aa = atoi(at);
    int bb = atoi(bt);
    if(aa < bb){
        return 1; 
    }
    else{
        return 0;
    }

}



int main(int argc , char *argv[])
{
	// if(argc != 2){
    //     cerr << "Please give: port" << endl;
    //     exit(0);
    // }
    // int port = atoi(argv[1]);
    
    struct sockaddr_in cliaddr;
    socklen_t len;
	int opt = TRUE;
	int master_socket , addrlen , new_socket , client_socket[20] ,
		max_clients = 20 , activity, i , valread , sd;
	int max_sd;
	struct sockaddr_in address;
    //struct user user_array;
		
	char buffer[1025]; //data buffer of 1K

    int gamemode = 0;
    int guesstime = 0;
    char answer[MAXLINE];
    
    //char name[MAXLINE] = {'\0'};
    //string name = "";
    
		
	//set of socket descriptors
	fd_set readfds;
	//char *message = "link start!\r\n";
	
	//initialise all client_socket[] to 0 so not checked
    
	for (i = 0; i < max_clients; i++){
		client_socket[i] = 0;
	}
    // for(int i = 0; i < 20; i++){
    //     //cout<<"haha"<<endl;
    //     user_array.login[i] = 0;
    //     strcpy(user_array.user_room[i],"\0");
        
    // }
    // user_array.num_of_client = 0;
    // user_array.num_of_user = 0;
    

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
	if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0){
		perror("bind failed");
		exit(EXIT_FAILURE);
	}
	//printf("Listener on port %d \n", PORT);
    //cout<<"TCP server is running"<<endl;

    // create UDP socket
    int udpfd = socket(AF_INET, SOCK_DGRAM, 0);
    // binding server addr structure to udp sockfd
    bind(udpfd, (struct sockaddr*)&address, sizeof(address));
    //cout<<"UDP server is running"<<endl;

	//try to specify maximum of 30 pending connections for the master socket
	if (listen(master_socket, 20) < 0){
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
			//printf("select error");
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
            for(int i=0;i<100;i++){
                if(arg[length-1][i]=='\n'){
                    arg[length-1][i] = '\0';
                    break;
                }
            }
            arg[length] = NULL;

            // int index_fd = check_fd_exist(sd); //inviter
            // int index = check_username_exist(clients[index_fd].user.username); //inviter
            // int index_public = check_publicroom_id_exist(clients[index_fd].room);
            // int index_private = check_privateroom_id_exist(clients[index_fd].room);

            // int global_index_fd = check_fd_exist(sd);
            // int global_index_public = check_publicroom_id_exist(clients[global_index_fd].room);
            // int global_index_private = check_privateroom_id_exist(clients[global_index_fd].room);
            
            // if(global_index_public != -1){
            //     if(public_rooms[global_index_public].status == 1){
            //         string data;
            //         data = "Please enter 4 digit number with leading zero";
            //         int data_length = data.length();
            //         sendto(udpfd, data.c_str(), data_length, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
            //     }
            // }else if(global_index_private != -1){
            //     if(private_rooms[global_index_private].status == 1){
            //         string data;
            //         data = "Please enter 4 digit number with leading zero";
            //         int data_length = data.length();
            //         sendto(udpfd, data.c_str(), data_length, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
            //     }
            // }else 

            if(strcmp(arg[0],"register")==0){
                if(length!=4){
                    string data;
                    data = "Usage: register <username> <email> <password>\n";
                    int data_length = data.length();
                    sendto(udpfd, data.c_str(), data_length, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
                    
                }else{
                    if(check_username_exist(arg[1])!=-1){
                        string data;
                        data = "Username or Email is already used\n";
                        int data_length = data.length();
                        sendto(udpfd, data.c_str(), data_length, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
                    }else if(check_email_exist(arg[2])!=-1){
                        string data;
                        data = "Username or Email is already used\n";
                        int data_length = data.length();
                        sendto(udpfd, data.c_str(), data_length, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));
                    }else{
                        user registering_user;
                        string arg1 = arg[1];
                        string arg2 = arg[2];
                        string arg3 = arg[3];
                        registering_user.username = arg1;
                        registering_user.email = arg2;
                        registering_user.password = arg3;
                        registering_user.login = 0;
                        users.push_back(registering_user);
                        //cout<<arg1<<" "<<arg2<<" "<<arg3<<endl;
                        string data;
                        data = "Register Successfully\n";
                        int data_length = data.length();
                        sendto(udpfd, data.c_str(), data_length, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));

                    }
                    
                }
                
            }else if(strcmp(arg[0],"game-rule")==0){
                string data;
                int data_length;
                if(length != 1){
                    data = "Usage: game-rule\n";
                    
                }
                else{
                    data = 
                    "1. Each question is a 4-digit secret number.\n\
                    2. After each guess, you will get a hint with the following information:\n\
                    2.1 The number of \"A\", which are digits in the guess that are in the correct position.\n\
                    2.2 The number of \"B\", which are digits in the guess that are in the answer but are in the wrong position.\n\
                    The hint will be formatted as \"xAyB\".\n\
                    3. 5 chances for each question.\n";
                }
                data_length = data.length();
                sendto(udpfd, data.c_str(), data_length, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));

            }else if(strcmp(arg[0],"list")==0 && strcmp(arg[1],"rooms")==0){
                //cout<<"haha"<<endl;
                map<string,pair<string,int>>sorted_room; // id, public, status
                for(int i=0;i<public_rooms.size();i++){
                    sorted_room[public_rooms[i].id] = make_pair("Public", public_rooms[i].status);
                }
                for(int i=0;i<private_rooms.size();i++){
                    sorted_room[private_rooms[i].id] = make_pair("Private", private_rooms[i].status);
                }
                //sort(sorted_room.begin(), sorted_room.end());

                string data;
                int data_length;
                if(public_rooms.size() == 0 && private_rooms.size()==0){
                    data = "List Game Rooms\nNo Rooms\n";
                }else{
                    data = "List Game Rooms\n";
                    int j=1;
                    for(auto it = sorted_room.begin(); it != sorted_room.end(); it++){
                        if((*it).second.second == 0){
                            data = data + to_string(j) + ". (" + (*it).second.first + ") Game Room " + (*it).first + " is open for players\n";
                        }else if((*it).second.second == 1){
                            data = data + to_string(j) + ". (" + (*it).second.first + ") Game Room " + (*it).first + " has started playing\n";
                        }
                        j++;
                    }
                }
                data_length = data.length();
                sendto(udpfd, data.c_str(), data_length, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));

            }else if(strcmp(arg[0],"list")==0 && strcmp(arg[1],"users")==0){
                vector<user>sorted_user;
                for(int i=0;i<users.size();i++){
                    sorted_user.push_back(users[i]);
                }
                
                sort(sorted_user.begin(), sorted_user.end(), user_compare);

                string data;
                int data_length;
                if(users.size()==0){
                    data = "List Users\nNo Users\n";
                }else{
                    data = "List Users\n";
                    for(int i=0;i<sorted_user.size();i++){
                        if(sorted_user[i].login == 0){
                            data = data + to_string(i+1) + ". " + sorted_user[i].username + "<" + sorted_user[i].email + "> Offline\n";
                        }else if(sorted_user[i].login == 1){
                            data = data + to_string(i+1) + ". " + sorted_user[i].username + "<" + sorted_user[i].email + "> Online\n";
                        }
                    }
                }
                data_length = data.length();
                sendto(udpfd, data.c_str(), data_length, 0, (struct sockaddr*)&cliaddr, sizeof(cliaddr));

            }
        }
		
		//tcp_not_connect
		if (FD_ISSET(master_socket, &readfds)){
            
			if ((new_socket = accept(master_socket,(struct sockaddr *)&address, (socklen_t*)&addrlen))<0){
				//perror("accept");
				exit(EXIT_FAILURE);
			}
            //cout<<"haha"<<endl;
			//inform user of socket number - used in send and receive commands
			//printf("New connection , socket fd is %d , ip is : %s , port : %d\n" , new_socket , inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
            
            // string data;
            // data = "*****Welcome to Game 1A2B*****";
            // memset(&buffer, 0, sizeof(buffer));
            //recv(new_socket, (char*)&buffer, sizeof(buffer), 0);
            //sendto(new_socket, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(new_socket));
            //cout<<"New connection"<<endl;
            // user_array.fd[user_array.num_of_client] = new_socket;
            // user_array.num_of_client++;
            client new_client;
            new_client.fd = new_socket;
            new_client.login = 0;
            new_client.in_room = 0;
            clients.push_back(new_client);

			for (i = 0; i < max_clients; i++){
				if( client_socket[i] == 0 ){
					client_socket[i] = new_socket;
					//printf("Adding to list of sockets as %d\n" , i);
					break;
				}
			}
            //cout<<"haha"<<endl;
		}

		//tcp	


        
		for (i = 0; i < max_clients; i++){
			sd = client_socket[i];
            
			if (FD_ISSET( sd , &readfds)){
                
				//Check if it was for closing , and also read the
				//incoming message
				memset(buffer,0,sizeof(buffer));
				if ((valread = read( sd , buffer, 1024)) == 0)
				{
					//Somebody disconnected , get his details and print
					getpeername(sd , (struct sockaddr*)&address , \
						(socklen_t*)&addrlen);
					//printf("Host disconnected , ip %s , port %d \n" ,
						//inet_ntoa(address.sin_addr) , ntohs(address.sin_port));
                    
                    int index_fd = check_fd_exist(sd); 
                    int index = check_username_exist(clients[index_fd].user.username); 
                    int index_public = check_publicroom_id_exist(clients[index_fd].room);
                    int index_private = check_privateroom_id_exist(clients[index_fd].room);
                    if(clients[index_fd].in_room == 1){
                        clients[index_fd].in_room = 0;
                        clients[index_fd].room = "";
                        
                        if(index_public!=-1){
                            int index_player_in_public = find_player_index_in_public_room(index_public, clients[index_fd].user.username);
                            for(int i=0;i<public_rooms[index_public].player.size();i++){
                                //cout<<"##"<<public_rooms[index_public].player[i]<<endl;
                            }
                            //cout<<index_player_in_public<<clients[index_fd].user.username<<endl;
                            public_rooms[index_public].player.erase(public_rooms[index_public].player.begin()+index_player_in_public);
                            if(public_rooms[index_public].manager == clients[index_fd].user.username){
                                public_rooms.erase(public_rooms.begin()+index_public);
                            }
                        }else if(index_private!=-1){
                            int index_player_in_private = find_player_index_in_private_room(index_private, clients[index_fd].user.username);
                            private_rooms[index_private].player.erase(private_rooms[index_private].player.begin()+index_player_in_private);
                            if(private_rooms[index_private].manager == clients[index_fd].user.username){
                                private_rooms.erase(private_rooms.begin()+index_private);
                            }
                        }
                        

                    }
                    if(clients[index_fd].login == 1){
                        clients[index_fd].login = 0;
                        struct user logout_user;
                        clients[index_fd].user = logout_user;
                    }

                    users[index].login = 0;

                    
						
					//Close the socket and mark as 0 in list for reuse
					close( sd );
					client_socket[i] = 0;
					//clientFlag[i] = 0; //I ADD


				}
				else{
                    len = sizeof(cliaddr);
                    //memset(&buffer, 0, sizeof(buffer));   
                    //recv(sd, (char*)&buffer, sizeof(buffer), 0);
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
                    for(int i=0;i<100;i++){
                        if(arg[length-1][i]=='\n'){
                            arg[length-1][i] = '\0';
                            break;
                        }
                    }
                    arg[length] = NULL;


                    if(strcmp(arg[0],"login")==0){    
                                         
                        if(length!=3){
                            string data;
                            data = "Usage: login <username> <password>";
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }else{
                            string arg1 = arg[1];
                            string arg2 = arg[2];
                            int index = check_username_exist(arg1);
                            int index_fd = check_fd_exist(sd);
                            //int index = check_username_exit(user_array,user_array.name[user_array.fd[index_fd]],user_array.num_of_user);
                            if(index!=-1){
                                if(users[index].login==1){
                                    string data;
                                    data = "Someone already logged in as " + arg1 + "\n";
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(clients[index_fd].login==1){
                                    string u_name = clients[index_fd].user.username;
                                    string data;
                                    data = "You already logged in as " + u_name + "\n";
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(users[index].password != arg2){
                                    //cout<<users[index].password<<" "<<arg2<<endl;
                                    //cout<<strlen(users[index].password.c_str())<<" #"<<strlen(arg2.c_str())<<endl;
                                    string data;
                                    data = "Wrong password\n";
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else{
                                    users[index].login = 1;
                                    
                                    clients[index_fd].login = 1;
                                    clients[index_fd].user = users[index];
                                    string data;
                                    data = "Welcome, " + users[index].username + "\n";
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }
                            }else{
                                string data;
                                data = "Username does not exist\n";
                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                            }
                        }
                        
                    }else if(strcmp(arg[0],"logout")==0){
                        int index_fd = check_fd_exist(sd);
                        int index = check_username_exist(clients[index_fd].user.username);
                        if(clients[index_fd].login == 0){
                            string data;
                            data = "You are not logged in\n";                            
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }else if(clients[index_fd].in_room == 1){
                            string data;
                            data = "You are already in game room " + clients[index_fd].room + ", please leave game room\n";
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }else{
                            //Bye, <username>.
                            users[index].login = 0;
                            struct user logout_user;
                            clients[index_fd].user = logout_user;
                            clients[index_fd].login = 0;
                            string data = "Goodbye, " + users[index].username + "\n";
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }
                    }else if(strcmp(arg[0],"create")==0 && strcmp(arg[1],"public")==0 && strcmp(arg[2],"room")==0){
                        int index_fd = check_fd_exist(sd);
                        int index = check_username_exist(clients[index_fd].user.username);
                        string arg3 = arg[3];
                        if(length!=4){
                            string data;
                            data = "Usage: create public room <game room id>\n";
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }else{
                            if(clients[index_fd].login == 0){
                                string data;
                                data = "You are not logged in\n";                            
                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                            }else{
                                if(clients[index_fd].in_room == 1){
                                    string data;
                                    data = "You are already in game room " + clients[index_fd].room + ", please leave game room\n";
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(check_publicroom_id_exist(arg3)!=-1 || check_privateroom_id_exist(arg3)!=-1){
                                    string data;
                                    data = "Game room ID is used, choose another one\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else{
                                    string game_id = arg3;
                                    string data;
                                    data = "You create public game room "+ game_id + "\n"; 
                                    public_room room;
                                    room.id = arg3;
                                    room.status = 0;
                                    room.manager = clients[index_fd].user.username;
                                    room.player.push_back(clients[index_fd].user.username);
                                    
                                    room.current_player = 0;
                                    public_rooms.push_back(room);
                                    clients[index_fd].room = game_id;
                                    clients[index_fd].in_room = 1;
                                    
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }
                            }
                        }

                    }else if(strcmp(arg[0],"create")==0 && strcmp(arg[1],"private")==0 && strcmp(arg[2],"room")==0){
                        int index_fd = check_fd_exist(sd);
                        int index = check_username_exist(clients[index_fd].user.username);
                        string arg3 = arg[3];
                        string arg4 = arg[4];
                        if(length!=5){
                            string data;
                            data = "Usage: create private room <game_room_id> <invitation code>\n";
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }else{
                            if(clients[index_fd].login == 0){
                                string data;
                                data = "You are not logged in\n";                            
                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                            }else{
                                if(clients[index_fd].in_room == 1){
                                    string data;
                                    data = "You are already in game room " + clients[index_fd].room + ", please leave game room\n";
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(check_publicroom_id_exist(arg3)!=-1 || check_privateroom_id_exist(arg3)!=-1){
                                    string data;
                                    data = "Game room ID is used, choose another one\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else{
                                    string game_id = arg3;
                                    string data;
                                    data = "You create private game room "+ game_id + "\n"; 
                                    private_room room;
                                    room.id = arg3;
                                    room.invitation_code = arg4;
                                    room.manager = clients[index_fd].user.username;
                                    room.status = 0;
                                    room.player.push_back(clients[index_fd].user.username);
                                    room.current_player = 0;
                                    private_rooms.push_back(room);
                                    clients[index_fd].room = game_id;
                                    clients[index_fd].in_room = 1;
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }
                            }
                        }

                    }else if(strcmp(arg[0],"join")==0 && strcmp(arg[1],"room")==0){
                        int index_fd = check_fd_exist(sd);
                        int index = check_username_exist(clients[index_fd].user.username);
                        string arg2 = arg[2];
                        int index_public = check_publicroom_id_exist(arg2);
                        int index_private = check_privateroom_id_exist(arg2);
                        
                        if(length!=3){
                            string data;
                            data = "Usage: join room <game room id>\n";
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }else{
                            if(clients[index_fd].login == 0){
                                string data;
                                data = "You are not logged in\n";                            
                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                            }else{
                                if(clients[index_fd].in_room == 1){
                                    string data;
                                    data = "You are already in game room " + clients[index_fd].room + ", please leave game room\n";
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(index_public == -1 && index_private == -1){
                                    string data;
                                    data = "Game room " +  arg2 + " is not exist\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(index_private != -1){
                                    string data;
                                    data = "Game room is private, please join game by invitation code\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(public_rooms[index_public].status == 1){
                                    string data;
                                    data = "Game has started, you can't join now\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else{
                                    string game_id = arg2;
                                    string data;
                                    data = "You join game room "+ game_id + "\n"; 
                                    clients[index_fd].room = game_id;
                                    clients[index_fd].in_room = 1;
                                    public_rooms[index_public].player.push_back(clients[index_fd].user.username);
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                    for(int i=0;i<clients.size();i++){
                                        if(clients[i].fd != sd && clients[i].room == game_id){
                                            data = "Welcome, " + clients[index_fd].user.username + " to game!\n";
                                            sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                        }
                                    }
                                }
                            }
                        }

                    }else if(strcmp(arg[0],"invite")==0){
                        string arg1 = arg[1];
                        int index_fd = check_fd_exist(sd); //inviter
                        int index = check_username_exist(clients[index_fd].user.username); //inviter
                        int index_email = check_email_exist(arg1); //invitee
                        int index_public = check_publicroom_id_exist(clients[index_fd].room);
                        int index_private = check_privateroom_id_exist(clients[index_fd].room);
                        int index_invitee_fd = check_username_in_client(users[index_email].username);
                        
                        if(length!=2){
                            string data;
                            data = "Usage: invite <invitee email> \n";
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }else{
                            if(clients[index_fd].login == 0){
                                string data;
                                data = "You are not logged in\n";                            
                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                            }else{
                                if(clients[index_fd].in_room == 0){
                                    string data;
                                    data = "You did not join any game room\n";
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(index_private == -1){
                                    string data;
                                    data = "You are not private game room manager\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(private_rooms[index_private].manager != clients[index_fd].user.username){
                                    string data;
                                    data = "You are not private game room manager\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(users[index_email].login == 0){
                                    string data;
                                    data = "Invitee not logged in\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else{

                                    string game_id = clients[index_fd].room;
                                    string data;
                                    data = "You send invitation to " + users[index_email].username + "<" + arg1 + ">\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                    data = "You receive invitation from " + users[index].username + "<" + users[index].email + ">\n"; 

                                    if(check_invite_repeat(index_email, users[index].username, clients[index_fd].room)==-1){

                                        struct invitation inviter;
                                        inviter.inviter = users[index].username;
                                        inviter.inviter_email = users[index].email;
                                        inviter.room_id = clients[index_fd].room;
                                        inviter.invitation_code = private_rooms[index_private].invitation_code;
                                        users[index_email].invitation.push_back(inviter);
                                        //cout<<private_rooms[index_private].invitation_code<<endl;
                                    }
                                    
                                    sendto(clients[index_invitee_fd].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[index_invitee_fd].fd));
                                }
                            }
                        }

                    }else if(strcmp(arg[0],"list")==0 && strcmp(arg[1],"invitations")==0){
                        int index_fd = check_fd_exist(sd); //inviter
                        int index = check_username_exist(clients[index_fd].user.username); //inviter
                        
                        if(users[index].invitation.size() == 0){
                            string data;
                            data = "List invitations\nNo Invitations\n";                            
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }else{
                            string data;
                            data = "List invitations\n";
                            vector<struct invitation>invitations;
                            for(int i=0;i<users[index].invitation.size();i++){
                                //data = data + to_string(i+1) + ". " + users[index].invitation[i].inviter + "<" + users[index].invitation[i].inviter_email + "> invite you to join game room " + users[index].invitation[i].room_id + ", invitation code is " + users[index].invitation[i].invitation_code + "\n";
                                invitations.push_back(users[index].invitation[i]);
                            }
                            sort(invitations.begin(),invitations.end(),invitation_compare);
                            for(int i=0;i<invitations.size();i++){
                                data = data + to_string(i+1) + ". " + invitations[i].inviter + "<" + invitations[i].inviter_email + "> invite you to join game room " + invitations[i].room_id + ", invitation code is " + invitations[i].invitation_code + "\n";
                            }
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }


                    }else if(strcmp(arg[0],"accept")==0){
                        string arg2 = arg[2];
                        string arg1 = arg[1];
                        int index_fd = check_fd_exist(sd); //inviter
                        int index = check_username_exist(clients[index_fd].user.username); //inviter
                        //int index_email = check_email_exist(users[index].email);
                        int index_public = check_publicroom_id_exist(clients[index_fd].room);
                        int index_private = check_privateroom_id_exist(clients[index_fd].room);
                        int index_invitation_code = check_invitation_exist(users[index], arg2);
                        int index_invitation_code_exist = check_privateroom_invitation_code_exist(arg2);
                        
                        
                        if(length!=3){
                            string data;
                            data = "Usage: accept <inviter email> <invitation code>\n";
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }else{
                            if(clients[index_fd].login == 0){
                                string data;
                                data = "You are not logged in\n";                            
                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                            }else{
                                if(clients[index_fd].in_room == 1){
                                    string data;
                                    data = "You are already in game room " + clients[index_fd].room + ", please leave game room\n";
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(index_invitation_code_exist == -1){
                                    string data;
                                    data = "Your invitation code is incorrect\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(index_invitation_code == -1){
                                    string data;
                                    data = "Invitation not exist\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else if(private_rooms[index_invitation_code_exist].status == 1){
                                    string data;
                                    data = "Game has started, you can't join now\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else{
                                    // if(clients[index_fd].room != private_rooms[index_invitation_code_exist].id){
                                    //     string data;
                                    //     data = "Invitation not exist\n"; 
                                    //     sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                    // }else{
                                    string game_id = clients[index_fd].room;
                                    string data;
                                    data = "You join game room " + private_rooms[index_invitation_code_exist].id + "\n"; 
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                    clients[index_fd].room = private_rooms[index_invitation_code_exist].id;
                                    clients[index_fd].in_room = 1;
                                    private_rooms[index_invitation_code_exist].player.push_back(clients[index_fd].user.username);
                                    //users[index].invitation.erase(users[index].invitation.begin() + index_invitation_code);
                                    data = "Welcome, " + users[index].username + " to game!\n"; 
                                    for(int i=0;i<clients.size();i++){
                                        if((clients[i].fd != sd) && (clients[i].room == private_rooms[index_invitation_code_exist].id)){
                                            sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                        }
                                    }
                                        
                                    // }
                                    
                                }
                            }
                        }

                    }else if(strcmp(arg[0],"leave")==0 && strcmp(arg[1],"room")==0){
                        string arg1 = arg[1];
                        int index_fd = check_fd_exist(sd); 
                        int index = check_username_exist(clients[index_fd].user.username); 
                        int index_public = check_publicroom_id_exist(clients[index_fd].room);
                        int index_private = check_privateroom_id_exist(clients[index_fd].room);
                        
                        
                        if(clients[index_fd].login == 0){
                            string data;
                            data = "You are not logged in\n";                            
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }else{
                            if(clients[index_fd].in_room == 0){
                                string data;
                                data = "You did not join any game room\n";
                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                            }else{
                                if(index_public!=-1){
                                    if(public_rooms[index_public].manager == users[index].username){
                                        string data;
                                        data = "You leave game room " + public_rooms[index_public].id + "\n";
                                        sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        clients[index_fd].room = "";
                                        clients[index_fd].in_room = 0;
                                        
                                        for(int i=0;i<users.size();i++){
                                            vector<int>index_need_to_be_deleted;
                                            for(int j=0;j<users[i].invitation.size();j++){
                                                if(users[i].invitation[j].inviter == users[index].username && users[i].invitation[j].room_id == public_rooms[index_public].id){
                                                    index_need_to_be_deleted.push_back(j);
                                                    //users[i].invitation.erase(users[i].invitation.begin()+j);
                                                }
                                            }
                                            for(int j=index_need_to_be_deleted.size()-1;j>=0;j--){
                                                users[i].invitation.erase(users[i].invitation.begin()+j);
                                            }
                                        }
                                        
                                        
                                        data = "Game room manager leave game room " + public_rooms[index_public].id + ", you are forced to leave too\n";
                                        for(int i=0;i<clients.size();i++){
                                            if(clients[i].fd != sd && clients[i].room == public_rooms[index_public].id){
                                                sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                                clients[i].room = "";
                                                clients[i].in_room = 0;
                                            }
                                        }
                                        public_rooms.erase(public_rooms.begin()+index_public);
                                    }else if(public_rooms[index_public].manager != users[index].username && public_rooms[index_public].status == 1){
                                        int index_player = find_player_index_in_public_room(index_public,users[index].username);
                                        public_rooms[index_public].status = 0;
                                        public_rooms[index_public].player.erase(public_rooms[index_public].player.begin()+index_player);
                                        public_rooms[index_public].current_player = 0;
                                        string data;
                                        data = "You leave game room " + public_rooms[index_public].id + ", game ends\n";
                                        sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        clients[index_fd].room = "";
                                        clients[index_fd].in_room = 0;

                                        for(int i=0;i<users.size();i++){
                                            vector<int>index_need_to_be_deleted;
                                            for(int j=0;j<users[i].invitation.size();j++){
                                                if(users[i].invitation[j].inviter == users[index].username && users[i].invitation[j].room_id == public_rooms[index_public].id){
                                                    index_need_to_be_deleted.push_back(j);
                                                    //users[i].invitation.erase(users[i].invitation.begin()+j);
                                                }
                                            }
                                            for(int j=index_need_to_be_deleted.size()-1;j>=0;j--){
                                                users[i].invitation.erase(users[i].invitation.begin()+j);
                                            }
                                        }
                                        data = users[index].username + " leave game room " + public_rooms[index_public].id + ", game ends\n";
                                        for(int i=0;i<clients.size();i++){
                                            if(clients[i].fd != sd && clients[i].room == public_rooms[index_public].id){
                                                sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                            }
                                        }
                                    }else if(public_rooms[index_public].manager != users[index].username && public_rooms[index_public].status == 0){
                                        public_rooms[index_public].status = 0;
                                        int index_player = find_player_index_in_public_room(index_public,users[index].username);
                                        public_rooms[index_public].player.erase(public_rooms[index_public].player.begin()+index_player);
                                        string data;
                                        data = "You leave game room " + public_rooms[index_public].id + "\n";
                                        sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        clients[index_fd].room = "";
                                        clients[index_fd].in_room = 0;
                                        
                                        for(int i=0;i<users.size();i++){
                                            vector<int>index_need_to_be_deleted;
                                            for(int j=0;j<users[i].invitation.size();j++){
                                                if(users[i].invitation[j].inviter == users[index].username && users[i].invitation[j].room_id == public_rooms[index_public].id){
                                                    index_need_to_be_deleted.push_back(j);
                                                    //users[i].invitation.erase(users[i].invitation.begin()+j);
                                                }
                                            }
                                            for(int j=index_need_to_be_deleted.size()-1;j>=0;j--){
                                                users[i].invitation.erase(users[i].invitation.begin()+j);
                                            }
                                        }
                                        
                                        data = users[index].username + " leave game room " + public_rooms[index_public].id + "\n";
                                        for(int i=0;i<clients.size();i++){
                                            if(clients[i].fd != sd && clients[i].room == public_rooms[index_public].id){
                                                sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                            }
                                        }
                                    }
                                }else if(index_private!=-1){
                                    if(private_rooms[index_private].manager == users[index].username){
                                        string data;
                                        data = "You leave game room " + private_rooms[index_private].id + "\n";
                                        sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        clients[index_fd].room = "";
                                        clients[index_fd].in_room = 0;
                                        for(int i=0;i<users.size();i++){
                                            vector<int>index_need_to_be_deleted;
                                            for(int j=0;j<users[i].invitation.size();j++){
                                                if(users[i].invitation[j].inviter == users[index].username && users[i].invitation[j].room_id == private_rooms[index_private].id){
                                                    index_need_to_be_deleted.push_back(j);
                                                    //users[i].invitation.erase(users[i].invitation.begin()+j);
                                                }
                                            }
                                            for(int j=index_need_to_be_deleted.size()-1;j>=0;j--){
                                                users[i].invitation.erase(users[i].invitation.begin()+j);
                                            }
                                        }
                                        data = "Game room manager leave game room " + private_rooms[index_private].id + ", you are forced to leave too\n";
                                        for(int i=0;i<clients.size();i++){
                                            if(clients[i].fd != sd && clients[i].room == private_rooms[index_private].id){
                                                sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                                clients[i].room = "";
                                                clients[i].in_room = 0;
                                            }
                                        }
                                        private_rooms.erase(private_rooms.begin()+index_private);
                                    }else if(private_rooms[index_private].manager != users[index].username && private_rooms[index_private].status == 1){
                                        private_rooms[index_private].status = 0;
                                        private_rooms[index_private].player.clear();
                                        private_rooms[index_private].current_player = 0;

                                        string data;
                                        data = "You leave game room " + private_rooms[index_private].id + ", game ends\n";
                                        sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        clients[index_fd].room = "";
                                        clients[index_fd].in_room = 0;
                                        int index_player = find_player_index_in_private_room(index_private,users[index].username);
                                        private_rooms[index_private].player.erase(private_rooms[index_private].player.begin()+index_player);
                                        for(int i=0;i<users.size();i++){
                                            vector<int>index_need_to_be_deleted;
                                            for(int j=0;j<users[i].invitation.size();j++){
                                                if(users[i].invitation[j].inviter == users[index].username && users[i].invitation[j].room_id == private_rooms[index_private].id){
                                                    index_need_to_be_deleted.push_back(j);
                                                    //users[i].invitation.erase(users[i].invitation.begin()+j);
                                                }
                                            }
                                            for(int j=index_need_to_be_deleted.size()-1;j>=0;j--){
                                                users[i].invitation.erase(users[i].invitation.begin()+j);
                                            }
                                        }

                                        data = users[index].username + " leave game room " + private_rooms[index_private].id + ", game ends\n";
                                        for(int i=0;i<clients.size();i++){
                                            if(clients[i].fd != sd && clients[i].room == private_rooms[index_private].id){
                                                sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                            }
                                        }
                                    }else if(private_rooms[index_private].manager != users[index].username && private_rooms[index_private].status == 0){
                                        private_rooms[index_private].status = 0;
                                        string data;
                                        data = "You leave game room " + private_rooms[index_private].id + "\n";
                                        sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        //cout<<"haha"<<endl;
                                        clients[index_fd].room = "";
                                        clients[index_fd].in_room = 0;
                                        int index_player = find_player_index_in_private_room(index_private,users[index].username);
                                        //cout<<index_player<<endl;
                                        private_rooms[index_private].player.erase(private_rooms[index_private].player.begin()+index_player);
                                        //cout<<"haha"<<endl;
                                        
                                        for(int i=0;i<users.size();i++){
                                            vector<int>index_need_to_be_deleted;
                                            for(int j=0;j<users[i].invitation.size();j++){
                                                if(users[i].invitation[j].inviter == users[index].username && users[i].invitation[j].room_id == private_rooms[index_private].id){
                                                    index_need_to_be_deleted.push_back(j);
                                                    //users[i].invitation.erase(users[i].invitation.begin()+j);
                                                }
                                            }
                                            for(int j=index_need_to_be_deleted.size()-1;j>=0;j--){
                                                users[i].invitation.erase(users[i].invitation.begin()+j);
                                            }
                                        }
                                        //cout<<"haha"<<endl;
                                        

                                        data = users[index].username + " leave game room " + private_rooms[index_private].id + "\n";
                                        for(int i=0;i<clients.size();i++){
                                            if(clients[i].fd != sd && clients[i].room == private_rooms[index_private].id){
                                                sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                            }
                                        }
                                    }
                                }
                            }
                        }
                        

                    }else if(strcmp(arg[0],"start")==0 && strcmp(arg[1],"game")==0){
                        string arg2 = arg[2];
                        
                        int index_fd = check_fd_exist(sd); //inviter
                        int index = check_username_exist(clients[index_fd].user.username); //inviter
                        int index_public = check_publicroom_id_exist(clients[index_fd].room);
                        int index_private = check_privateroom_id_exist(clients[index_fd].room);
                        
                        
                        if( length!=3 && length!=4 ){
                            string data;
                            data = "Usage: start game <number of rounds> <guess number>\n";
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }else{
                            if(clients[index_fd].login == 0){
                                string data;
                                data = "You are not logged in\n";                            
                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                            }else{
                                if(clients[index_fd].in_room == 0){
                                    string data;
                                    data = "You did not join any game room\n";
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else{
                                    if(index_public != -1){
                                        if(public_rooms[index_public].manager != users[index].username){
                                            string data;
                                            data = "You are not game room manager, you can't start game\n";
                                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        }else if(public_rooms[index_public].status == 1){
                                            string data;
                                            data = "Game has started, you can't start again\n";
                                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        }else if(length == 4){
                                            string arg3 = arg[3];
                                            string data;
                                            int guess_number;
                                            try{
                                                guess_number = stoi(arg3);
                                            }catch(exception &e){
                                                data = "Please enter 4 digit number with leading zero\n";
                                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                                continue;
                                            }
                                            //guess_number = stoi(arg3);
                                            int flag=0;
                                            for(int i=0;i<4;i++){
                                                if(arg3[i]<'0' || arg3[i]>'9'){
                                                    flag=1;
                                                    break;
                                                }
                                            }
                                            if(flag==1){
                                                data = "Please enter 4 digit number with leading zero\n";
                                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                                continue;
                                            }
                                            if(arg3.size()!=4){
                                                data = "Please enter 4 digit number with leading zero\n";
                                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                                continue;
                                            }
                                            public_rooms[index_public].guess_number = arg3;
                                            public_rooms[index_public].number_of_rounds = stoi(arg2); //沒有處理錯誤狀況

                                            vector<string>players;
                                            for(int i=0;i<clients.size();i++){
                                                if(clients[i].room == public_rooms[index_public].id){
                                                    players.push_back(clients[i].user.username);
                                                }
                                            }
                                            //sort(players.begin(),players.end());

                                            public_rooms[index_public].player = players;
                                            public_rooms[index_public].current_player = 0;
                                            public_rooms[index_public].status = 1;

                                            data = "Game start! Current player is " + public_rooms[index_public].player[public_rooms[index_public].current_player] + "\n";
                                            for(int i=0;i<clients.size();i++){
                                                if(public_rooms[index_public].id == clients[i].room){
                                                    sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                                }
                                            }
                                            
                                        }else if(length == 3){
                                            public_rooms[index_public].guess_number = to_string(rand() % (9999 - 1000 + 1) + 1000); //偷吃步
                                            public_rooms[index_public].number_of_rounds = stoi(arg2); //沒有處理錯誤狀況
                                            string data;
                                            vector<string>players;
                                            for(int i=0;i<clients.size();i++){
                                                if(clients[i].room == public_rooms[index_public].id){
                                                    players.push_back(clients[i].user.username);
                                                }
                                            }
                                            //sort(players.begin(),players.end());

                                            public_rooms[index_public].player = players;
                                            public_rooms[index_public].current_player = 0;
                                            public_rooms[index_public].status = 1;

                                            data = "Game start! Current player is " + public_rooms[index_public].player[public_rooms[index_public].current_player] + "\n";
                                            
                                            for(int i=0;i<clients.size();i++){
                                                if(public_rooms[index_public].id == clients[i].room){
                                                    sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                                }
                                            }

                                        }
                                    }else if(index_private != -1){
                                        if(private_rooms[index_private].manager != users[index].username){
                                            string data;
                                            data = "You are not game room manager, you can't start game\n";
                                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        }else if(private_rooms[index_private].status == 1){
                                            string data;
                                            data = "Game has started, you can't start again\n";
                                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        }else if(length == 4){
                                            string data;
                                            string arg3 = arg[3];
                                            int guess_number;
                                            try{
                                                guess_number = stoi(arg3);
                                            }catch(exception &e){
                                                data = "Please enter 4 digit number with leading zero\n";
                                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                                continue;
                                            }
                                            //guess_number = stoi(arg3);
                                            int flag=0;
                                            for(int i=0;i<4;i++){
                                                if(arg3[i]<'0' || arg3[i]>'9'){
                                                    flag=1;
                                                    break;
                                                }
                                            }
                                            if(flag==1){
                                                data = "Please enter 4 digit number with leading zero\n";
                                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                                continue;
                                            }
                                            if(arg3.size()!=4){
                                                data = "Please enter 4 digit number with leading zero\n";
                                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                                continue;
                                            }
                                            private_rooms[index_private].guess_number = arg3;
                                            private_rooms[index_private].number_of_rounds = stoi(arg2); //沒有處理錯誤狀況
                                            private_rooms[index_private].status = 1;

                                            vector<string>players;
                                            for(int i=0;i<clients.size();i++){
                                                if(clients[i].room == private_rooms[index_private].id){
                                                    players.push_back(clients[i].user.username);
                                                }
                                            }
                                            sort(players.begin(),players.end());

                                            private_rooms[index_private].player = players;
                                            private_rooms[index_private].current_player = 0;

                                            data = "Game start! Current player is " + private_rooms[index_private].player[private_rooms[index_private].current_player] + "\n";
                                            for(int i=0;i<clients.size();i++){
                                                if(private_rooms[index_private].id == clients[i].room){
                                                    sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                                }
                                            }
                                        }else if(length == 3){
                                            private_rooms[index_private].guess_number = to_string(rand() % (9999 - 1000 + 1) + 1000); //偷吃步
                                            private_rooms[index_private].number_of_rounds = stoi(arg2); //沒有處理錯誤狀況

                                            vector<string>players;
                                            string data;
                                            for(int i=0;i<clients.size();i++){
                                                if(clients[i].room == private_rooms[index_private].id){
                                                    players.push_back(clients[i].user.username);
                                                }
                                            }
                                            sort(players.begin(),players.end());

                                            private_rooms[index_private].player = players;
                                            private_rooms[index_private].current_player = 0;
                                            private_rooms[index_public].status = 1;

                                            data = "Game start! Current player is " + private_rooms[index_private].player[private_rooms[index_private].current_player] + "\n";
                                            for(int i=0;i<clients.size();i++){
                                                if(private_rooms[index_private].id == clients[i].room){
                                                    sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                                }
                                            }

                                        }
                                    }
                                }
                            }
                        }
                    }else if(strcmp(arg[0],"guess")==0){
                        string arg1 = arg[1];
                        
                        int index_fd = check_fd_exist(sd); 
                        int index = check_username_exist(clients[index_fd].user.username); 
                        int index_public = check_publicroom_id_exist(clients[index_fd].room);
                        int index_private = check_privateroom_id_exist(clients[index_fd].room);
                        
                        
                        if( length!=2 ){
                            string data;
                            data = "Usage: guess <guess number>\n";
                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        }else{
                            if(clients[index_fd].login == 0){
                                string data;
                                data = "You are not logged in\n";                            
                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                            }else{
                                if(clients[index_fd].in_room == 0){
                                    string data;
                                    data = "You did not join any game room\n";
                                    sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                }else{
                                    if(index_public != -1){
                                        if(public_rooms[index_public].status == 0 && public_rooms[index_public].manager == users[index].username){
                                            string data;
                                            data = "You are game room manager, please start game first\n";
                                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        }else if(public_rooms[index_public].status == 0 && public_rooms[index_public].manager != users[index].username){
                                            string data;
                                            data = "Game has not started yet\n";
                                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        }else if(public_rooms[index_public].player[public_rooms[index_public].current_player] != users[index].username){
                                            string data;
                                            data = "Please wait..., current player is " + public_rooms[index_public].player[public_rooms[index_public].current_player] + "\n";
                                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        }else{
                                            string data;
                                            int guess_number;
                                            try{
                                                guess_number = stoi(arg1);
                                            }catch(exception &e){
                                                data = "Please enter 4 digit number with leading zero\n";
                                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                                continue;
                                            }
                                            //guess_number = stoi(arg3);
                                            int flag=0;
                                            for(int i=0;i<4;i++){
                                                if(arg1[i]<'0' || arg1[i]>'9'){
                                                    flag=1;
                                                    break;
                                                }
                                            }
                                            if(flag==1){
                                                data = "Please enter 4 digit number with leading zero\n";
                                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                                continue;
                                            }
                                            if(arg1.size()!=4){
                                                data = "Please enter 4 digit number with leading zero\n";
                                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                                continue;
                                            }

                                            string guess = arg1;
                                            string ans = public_rooms[index_public].guess_number;
                                            int numA=0;
                                            int numB=0;
                                            int visited[4];
                                            for(int i=0;i<4;i++){
                                                visited[i] = 0;
                                            }
                                            for(int i=0;i<4;i++){
                                                if(guess[i]==ans[i]){
                                                    numA++;
                                                    visited[i] = 1;
                                                    
                                                }
                                            }
                                            for(int i=0;i<4;i++){
                                                for(int j=0;j<4;j++){
                                                    if(visited[i] == 1 || visited[j] == 1){
                                                        continue;
                                                    }
                                                    if(guess[i]==ans[j] && i!=j){
                                                        numB++;
                                                        break;
                                                    }
                                                }
                                            }
                                            
                                            if(numA==4){
                                                data = clients[index_fd].user.username + " guess '" + arg1 + "' and got Bingo!!! " + clients[index_fd].user.username + " wins the game, game ends\n";
                                                for(int i=0;i<clients.size();i++){
                                                    if(clients[i].room == public_rooms[index_public].id){
                                                        sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                                    }
                                                }
                                                public_rooms[index_public].status = 0;
                                                public_rooms[index_public].number_of_rounds = 0;
                                                public_rooms[index_public].guess_number = "";
                                                //public_rooms[index_public].player.clear();
                                                public_rooms[index_public].current_player = 0;
                                                continue;
                                            }
                                            if(public_rooms[index_public].current_player == public_rooms[index_public].player.size()-1){ // if he is the last player
                                                public_rooms[index_public].number_of_rounds = public_rooms[index_public].number_of_rounds - 1;
                                            }
                                            
                                            if(public_rooms[index_public].number_of_rounds == 0){
                                                data = clients[index_fd].user.username + " guess '" + arg1 + "' and got '" + to_string(numA) + "A" + to_string(numB) + "B'\nGame ends, no one wins\n";
                                                for(int i=0;i<clients.size();i++){
                                                    if(clients[i].room == public_rooms[index_public].id){
                                                        sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                                    }
                                                }
                                                public_rooms[index_public].status = 0;
                                                public_rooms[index_public].guess_number = "";
                                                //public_rooms[index_public].player.clear();
                                                public_rooms[index_public].current_player = 0;
                                                
                                                continue;
                                            }else if(public_rooms[index_public].number_of_rounds != 0){
                                                data = clients[index_fd].user.username + " guess '" + arg1 + "' and got '" + to_string(numA) + "A" + to_string(numB) + "B'\n";
                                                for(int i=0;i<clients.size();i++){
                                                    if(clients[i].room == public_rooms[index_public].id){
                                                        sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                                    }
                                                }
                                                public_rooms[index_public].current_player = (public_rooms[index_public].current_player + 1) % public_rooms[index_public].player.size();
                                                
                                                continue;
                                            }
                                        }
                                    }else if(index_private != -1){
                                        if(private_rooms[index_private].status == 0 && private_rooms[index_private].manager == users[index].username){
                                            string data;
                                            data = "You are game room manager, please start game first\n";
                                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        }else if(private_rooms[index_private].status == 0 && private_rooms[index_private].manager != users[index].username){
                                            string data;
                                            data = "Game has not started yet\n";
                                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        }else if(private_rooms[index_private].player[private_rooms[index_private].current_player] != users[index].username){
                                            string data;
                                            data = "Please wait..., current player is " + private_rooms[index_private].player[private_rooms[index_private].current_player] + "\n";
                                            sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                        }else{
                                            string data;
                                            int guess_number;
                                            try{
                                                guess_number = stoi(arg1);
                                            }catch(exception &e){
                                                data = "Please enter 4 digit number with leading zero\n";
                                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                                continue;
                                            }
                                            //guess_number = stoi(arg3);
                                            int flag=0;
                                            for(int i=0;i<4;i++){
                                                if(arg1[i]<'0' || arg1[i]>'9'){
                                                    flag=1;
                                                    break;
                                                }
                                            }
                                            if(flag==1){
                                                data = "Please enter 4 digit number with leading zero\n";
                                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                                continue;
                                            }
                                            if(arg1.size()!=4){
                                                data = "Please enter 4 digit number with leading zero\n";
                                                sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                                                continue;
                                            }

                                            string guess = arg1;
                                            string ans = private_rooms[index_private].guess_number;
                                            int numA=0;
                                            int numB=0;
                                            int visited[4];
                                            for(int i=0;i<4;i++){
                                                visited[i] = 0;
                                            }
                                            for(int i=0;i<4;i++){
                                                if(guess[i]==ans[i]){
                                                    numA++;
                                                    visited[i] = 1;
                                                    
                                                }
                                            }
                                            for(int i=0;i<4;i++){
                                                for(int j=0;j<4;j++){
                                                    if(visited[i] == 1 || visited[j] == 1){
                                                        continue;
                                                    }
                                                    if(guess[i]==ans[j] && i!=j){
                                                        numB++;
                                                        break;
                                                    }
                                                }
                                            }


                                            if(numA==4){
                                                data = clients[index_fd].user.username + " guess '" + arg1 + "' and got Bingo!!! " + clients[index_fd].user.username + " wins the game, game ends\n";
                                                for(int i=0;i<clients.size();i++){
                                                    if(clients[i].room == private_rooms[index_private].id){
                                                        sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                                    }
                                                }
                                                private_rooms[index_private].status = 0;
                                                private_rooms[index_private].number_of_rounds = 0;
                                                private_rooms[index_private].guess_number = "";
                                                private_rooms[index_private].player.clear();
                                                private_rooms[index_private].current_player = 0;
                                                continue;
                                            }
                                            if(private_rooms[index_private].current_player == private_rooms[index_private].player.size()-1){ // if he is the last player
                                                private_rooms[index_private].number_of_rounds = private_rooms[index_private].number_of_rounds - 1;
                                            }
                                            
                                            if(private_rooms[index_private].number_of_rounds == 0){
                                                data = clients[index_fd].user.username + " guess '" + arg1 + "' and got '" + to_string(numA) + "A" + to_string(numB) + "B'\nGame ends, no one wins\n";
                                                for(int i=0;i<clients.size();i++){
                                                    if(clients[i].room == private_rooms[index_private].id){
                                                        sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                                    }
                                                }
                                                private_rooms[index_private].status = 0;
                                                private_rooms[index_private].guess_number = "";
                                                private_rooms[index_private].player.clear();
                                                private_rooms[index_private].current_player = 0;
                                                continue;
                                            }else if(private_rooms[index_private].number_of_rounds != 0){
                                                data = clients[index_fd].user.username + " guess '" + arg1 + "' and got '" + to_string(numA) + "A" + to_string(numB) + "B'\n";
                                                for(int i=0;i<clients.size();i++){
                                                    if(clients[i].room == private_rooms[index_private].id){
                                                        sendto(clients[i].fd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(clients[i].fd));
                                                    }
                                                }
                                                private_rooms[index_private].current_player = (private_rooms[index_private].current_player + 1) % private_rooms[index_private].player.size();
                                                
                                                continue;
                                            }
                                        }
                                    }
                                }
                            }
                        }

                    }else if(strcmp(arg[0],"exit")==0){
                        continue;
                    }


// string guess_string(arg[0]);
                        // string answer_string(answer);
                        // int guess;
                        // int ans = stoi(answer_string);
                        // try{
                        //     guess = stoi(guess_string);
                        // }catch(exception &e){
                        //     string data;
                        //     data = "Your guess should be a 4-digit number.";
                        //     sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        //     continue;
                        // }                                    
                        // guess = stoi(guess_string);
                        // //cout<<guess<<endl;
                        // int flag=0;
                        // for(int i=0;i<4;i++){
                        //     if(guess_string[i]<'0' || guess_string[i]>'9'){
                        //         flag=1;
                        //         break;
                        //     }
                        // }
                        // if(flag==1){
                        //     string data;
                        //     data = "Your guess should be a 4-digit number.";
                        //     sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        //     continue;
                        // }
                        // if(guess_string.size()!=4){
                        //     string data;
                        //     data = "Your guess should be a 4-digit number.";
                        //     sendto(sd, data.c_str(), strlen(data.c_str()),0, (const struct sockaddr*)&address,sizeof(sd));
                        //     continue;
                        // }
                        // int guess_array[4];
                        // int answer_array[4];
                        // int numA=0;
                        // int numB=0;
                        // int visited[10];
                        // for(int i=0;i<10;i++){
                        //     visited[i]=0;
                        // }
                        // guess_array[0] = guess / 1000;
                        // guess = guess % 1000;
                        // guess_array[1] = guess / 100;
                        // guess = guess % 100;
                        // guess_array[2] = guess / 10;
                        // guess = guess % 10;
                        // guess_array[3] = guess;

                        // answer_array[0] = ans / 1000;
                        // ans = ans % 1000;
                        // answer_array[1] = ans / 100;
                        // ans = ans % 100;
                        // answer_array[2] = ans / 10;
                        // ans = ans % 10;
                        // answer_array[3] = ans;

                        // for(int i=0;i<4;i++){
                        //     if(guess_array[i]==answer_array[i]){
                        //         numA++;
                        //         visited[guess_array[i]]=1;
                        //         continue;
                        //     }
                        // }
                        // for(int i=0;i<4;i++){
                        //     if(visited[guess_array[i]]==1){
                        //         continue;
                        //     }
                        //     for(int j=0;j<4;j++){
                        //         if(guess_array[i]==answer_array[j] && i!=j){
                        //             numB++;
                        //             visited[guess_array[i]]=1;
                        //             break;
                        //         }
                        //     }
                        // }
                        // char _data[100]={'\0'};
                        // if(strcmp(buffer,answer)==0){
                        //     strcat(_data, "You got the answer!");
                        //     gamemode = 0;
                        //     guesstime = 0;
                        //     sendto(sd, _data, strlen(_data),0, (const struct sockaddr*)&address,sizeof(sd));
                        //     continue;
                        // }
                        // strcat(_data, to_string(numA).c_str());
                        // strcat(_data, "A");
                        // strcat(_data, to_string(numB).c_str());
                        // //cout<<"here"<<endl;
                        // if(guesstime!=4){
                        //     strcat(_data, "B");
                        //     guesstime++;
                        //     sendto(sd, _data, strlen(_data),0, (const struct sockaddr*)&address,sizeof(sd));
                        // }else{
                        //     strcat(_data, "B");
                        //     strcat(_data, "\nYou lose the game!");
                        //     gamemode = 0;
                        //     guesstime = 0;
                        //     sendto(sd, _data, strlen(_data),0, (const struct sockaddr*)&address,sizeof(sd));
                        // }



                    }
                    // else if(strcmp(arg[0],"exit")==0){
                    //     int index_fd = check_fd_exit(user_array, sd, user_array.num_of_user);
                    //     int index = check_username_exit(user_array,user_array.name[user_array.fd[index_fd]],user_array.num_of_user);
                    //     if(index != -1){
                    //         user_array.login[index] = 0;
                    //         index = -1;
                    //         //name = "";
                    //     }
                    //     cout<<"tcp get msg: exit"<<endl;
                    //     close(sd);
                    //     exit(0);
                    // }
                    
                    continue;
                    // ...
                    
				//}
			}
		}
	}
		
	return 0;
}

