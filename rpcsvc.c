//rustamchange// added headers
#include <stdio.h>

#include <stdint.h>
#include <stdarg.h>

#include <stdlib.h>
#include <errno.h>
#include <unistd.h>
#include <string.h>

#include <arpa/inet.h>    //close
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/time.h> //FD_SET, FD_ISSET, FD_ZERO macros
#include <netdb.h> 

#include "include/rpc.h"
#include "include/rpc_common.h"


#define va_arg_bysize(list, size)       (( ((char*)list) += size) - size)

/////////// modifiable definitions //////////////

// Server listening port number
#define PORT 8888

//data buffer set 1K
#define MAX_BUFFER_SIZE 1025 

// maximum number of allowed clients
#define MAX_NUM_OF_CLIENTS 30

/////////////////////////////////////////////////

RPC* rpcsvc_create(RPC* rpc,
    int(*connect)(RPC* rpc, uint32_t addr, uint16_t port, const char* username, const char* salted_password),
    int(*disconnect)(RPC* rpc),
    int(*send)(RPC* rpc, void* buf, int size),
    int(*receive)(RPC* rpc, void* buf, int size),
    int priv_size)
{
	if (rpc == NULL) {
		rpc = (RPC *)malloc(sizeof(RPC));
	}
    bzero(rpc, sizeof(RPC) + priv_size);
    rpc->connect = connect;
    rpc->disconnect = disconnect;
    rpc->send = send;
    rpc->receive = receive;

    return rpc;
}

RPC* rpcsvc_create_by_tcp(RPC* rpc,
	int tcp_socket_fd,
    int (*connect)(RPC* rpc, uint32_t addr, uint16_t port, const char* username, const char* salted_password),
	int (*disconnect)(RPC* rpc),
	int (*send)(RPC* rpc, void* buf, int size),
	int (*receive)(RPC* rpc, void* buf, int size),
	int priv_size)
{
	if (rpc == NULL) {
		rpc = (RPC *)malloc(sizeof(RPC));
	}
	
	bzero(rpc, sizeof(RPC) + priv_size);
    rpc->connect = connect;
    rpc->disconnect = disconnect;
    rpc->send = send;
    rpc->receive = receive;
	
	if (tcp_socket_fd != NULL){
		rpc->sockfd = tcp_socket_fd;
		return rpc;
	} else{
		return NULL;
	}
	
}

///////////////////////////////////////////////////////////////////////////
////// starting rpc server, before that rpc_init must be called ///////////
///////////////////////////////////////////////////////////////////////////

//Helper functions
//return 0, if the handling had no issue
int handle_new_connection(RPC* rpc, int master_socket, struct sockaddr_in* address, int * addrlen, int * new_socket, char *message, int *client_socket){
	int i;
	if ((*new_socket = accept(master_socket, (struct sockaddr *)address, (socklen_t*)addrlen))<0) {
		perror("accept");
		exit(EXIT_FAILURE);
    }

	// call rpc connect callback function
	if (rpc->connect != NULL) {
		rpc->connect(rpc, address->sin_addr.s_addr, ntohs(address->sin_port), "guest", "pass");
	}

	//add new socket to array of sockets
	for (i = 0; i < MAX_NUM_OF_CLIENTS; i++) {
		if( client_socket[i] == 0 ) {
			client_socket[i] = *new_socket;
			break;
		}
	}
	return 0;
}

int handle_incoming_data(RPC * rpc, int clientID, int sd, int *readfds, struct sockaddr_in* address, int * addrlen, int *client_socket){
	//Check if it was for closing , and also read the incoming message
	int valread;
	char buffer[MAX_BUFFER_SIZE];  //data buffer of 1K
	char * bufwalker;

	if ((valread = read( sd , buffer, MAX_BUFFER_SIZE-1)) == 0) {
        //Somebody disconnected , get his details and print
		getpeername(sd , (struct sockaddr*)address , (socklen_t*)addrlen);
		// call rpc disconnect callback function
		if (rpc->disconnect != NULL) {
			rpc->disconnect(rpc);
		}
        //Close the socket and mark as 0 in list for reuse
        close( sd );
        client_socket[clientID] = 0;
    }

    //Echo back the message that came in
    else {
		//// recovery example //// prototype code
		bufwalker = buffer;

		char proc_name[RPC_MAX_NAME];
		int proc_argc;
		memcpy(proc_name, bufwalker, RPC_MAX_NAME);
		bufwalker += RPC_MAX_NAME;

		memcpy(&proc_argc, bufwalker, sizeof(int));
		bufwalker += sizeof(int);

		//TODO: search for the procedure from the RPC sturctures RPC_Procedure array
		int i=0;
		int proc_index = -1; // index of procedure saved at procedure search time
		for (i=0;i<RPC_MAX_PROCEDURES;i++) {
			if (!strcmp(rpc->procedures[i].name, proc_name)) {
				proc_index = i;
				break;
			}
		}


		//TODO: create and allocated required space for the procedure return value and Call procedure
		void *proc_return = NULL;
		if (proc_index != -1){
			// allocation space and preparation of input arguments
			void **args = (void**) malloc(sizeof(void*)*(rpc->procedures[proc_index].argc) );
			for (i=0; i < rpc->procedures[proc_index].argc; i++) {
                if (rpc->procedures[proc_index].types[i] < RPC_TYPE_STRING) {
				    args[i] = malloc(calculateVariablesSize(1, &rpc->procedures[proc_index].types[i])); //malloc(rpc->procedures[proc_index].types[i]);
				    //copying the procedure arguments from received buffer
				    memcpy(args[i], bufwalker, calculateVariablesSize(1, &rpc->procedures[proc_index].types[i]));
				    bufwalker += calculateVariablesSize(1, &rpc->procedures[proc_index].types[i]); //rpc->procedures[proc_index].types[i];
                } else {
                    if (rpc->procedures[proc_index].types[i] == RPC_TYPE_STRING) {
                        string *str_ptr = (string*)malloc(sizeof(string));
                        memcpy( &str_ptr->size, bufwalker, sizeof(uint16_t) );
                        bufwalker += sizeof(uint16_t);

                        str_ptr->data = (uint8_t*)malloc(sizeof(uint8_t)*str_ptr->size);
                        memcpy( str_ptr->data, bufwalker, sizeof(uint8_t)*str_ptr->size );
                        bufwalker += sizeof(uint8_t) * str_ptr->size;

                        args[i] = str_ptr;
                    }
                }
			}
 
			proc_return = rpc->procedures[proc_index].func(rpc, rpc->procedures[proc_index].name, rpc->procedures[proc_index].argc, args, NULL);
            
            
            if (rpc->procedures[proc_index].return_type==RPC_TYPE_FLOAT || 
                rpc->procedures[proc_index].return_type==RPC_TYPE_DOUBLE){
                memcpy(buffer, proc_return, getTypeSize(rpc->procedures[proc_index].return_type) );
                buffer[getTypeSize(rpc->procedures[proc_index].return_type)+1] = '\0';
                send(sd, buffer, getTypeSize(rpc->procedures[proc_index].return_type)+1, 0);
            } else if ( rpc->procedures[proc_index].return_type == RPC_TYPE_STRING ) {
                string *str_ptr = (string*)proc_return;
                int totalsize = sizeof(uint16_t) + str_ptr->size;
                bufwalker = buffer;
                memcpy(bufwalker, &str_ptr->size, sizeof(uint16_t));
                bufwalker += sizeof(uint16_t);

                memcpy(bufwalker, str_ptr->data, str_ptr->size);
                bufwalker += str_ptr->size;
                bufwalker[0] = '\0';
                send(sd, buffer, totalsize, 0);
                
            } else {
			    // send rpc response back to client
			    memcpy(buffer, &proc_return, rpc->procedures[proc_index].return_type);
		    	buffer[rpc->procedures[proc_index].return_type] = '\0';
		    	send(sd, buffer, getTypeSize(rpc->procedures[proc_index].return_type)+1, 0);
            }

		} else {
			printf("Couldn't find procedure named: %s\n", proc_name);
		}
    }
	return 0;
}

//TODO: selection of the IP of server is not given
int rpcsvc_run(RPC* rpc) {
    int opt = TRUE;
    int master_socket , addrlen , new_socket , client_socket[MAX_NUM_OF_CLIENTS]  , activity, i , valread , sd;
    int max_sd;
    struct sockaddr_in address;

    char buffer[MAX_BUFFER_SIZE];  //data buffer of 1K

    //set of socket descriptors
    fd_set readfds;

    //a message
    char *message = "RPC Daemon v1.0 \r\n";

    //initialise all client_socket[] to 0 so not checked
    for (i = 0; i < MAX_NUM_OF_CLIENTS; i++) {
        client_socket[i] = 0;
    }

    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) {
        perror("master socket creation failed");
        exit(EXIT_FAILURE);
    }

    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if ( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 ) {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }

    //set receive time out
    struct timeval timeout_value;
    timeout_value.tv_sec = RPC_PROCEDURE_RESPONSE_TIMEOUT_S;
    timeout_value.tv_usec = 0;
    if ( setsockopt(master_socket, SOL_SOCKET, SO_RCVTIMEO, (const char*)&timeout_value, sizeof(struct timeval)) ) {
        perror("setsockopt");
	    exit(EXIT_FAILURE);
    }

    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );

    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }

    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, MAX_NUM_OF_CLIENTS) < 0) {
        perror("listen");
        exit(EXIT_FAILURE);
    }

    //accept the incoming connection
    addrlen = sizeof(address);

    while(TRUE) {
        //clear the socket set
        FD_ZERO(&readfds);

        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;

        //add child sockets to set
        for ( i = 0 ; i < MAX_NUM_OF_CLIENTS ; i++) {
            //socket descriptor
            sd = client_socket[i];

            //if valid socket descriptor then add to read list
            if(sd > 0)
                FD_SET( sd , &readfds);

            //highest file descriptor number, need it for the select function
            if(sd > max_sd)
                max_sd = sd;
        }

        //wait for an activity on one of the sockets , timeout is NULL , so wait indefinitely
        activity = select( max_sd + 1 , &readfds , NULL , NULL , &timeout_value);

        if ((activity < 0) && (errno!=EINTR)) {
            printf("select error\n");
        }

        if (activity == 0) {
		    printf("timeout error\n");
	    }

        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds)) {
            handle_new_connection(rpc, master_socket, &address, &addrlen, &new_socket, message, (int*)&client_socket);
        }

        //else its some IO operation on some other socket :)
        for (i = 0; i < MAX_NUM_OF_CLIENTS; i++) {
            sd = client_socket[i];
            if (FD_ISSET( sd , &readfds)) {
				handle_incoming_data(rpc, i, sd, (int*) &readfds, &address, &addrlen, (int*) &client_socket);
            }
        }
    }
}


