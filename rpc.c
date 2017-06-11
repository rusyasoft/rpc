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

#include "rpc.h"



#define va_arg_bysize(list, size)       (( ((char*)list) += size) - size)

/////////// modifiable definitions //////////////

// Server listening port number
#define PORT 8888

//data buffer set 1K
#define MAX_BUFFER_SIZE 1025 

// maximum number of allowed clients
#define MAX_NUM_OF_CLIENTS 30

/////////////////////////////////////////////////


int rpc_init(RPC* rpc,
    int(*connect)(RPC* rpc, uint32_t addr, uint16_t port, const char* username, const char* salted_password),
    int(*disconnect)(RPC* rpc),
    int(*send)(RPC* rpc, void* buf, int size),
    int(*receive)(RPC* rpc, void* buf, int size),
    int priv_size)
{

    bzero(rpc, sizeof(RPC) + priv_size);
    rpc->connect = connect;
    rpc->disconnect = disconnect;
    rpc->send = send;
    rpc->receive = receive;

    return 0;
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
        printf("received numofbytes = %d\n", valread); 
		//// recovery example //// prototype code
		bufwalker = buffer;

        //dbg: bytes
        int dbg_i=0;
        for(dbg_i=0;dbg_i<valread;dbg_i++){ 
            printf("%X ", buffer[dbg_i]);
        }
        printf("\n");


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
                        memcpy( &str_ptr->size, bufwalker, sizeof(uint16) );
                        bufwalker += sizeof(uint16);
                        //dbg
                        printf("-->received string size = %d\n", str_ptr->size);

                        str_ptr->data = (uint8*)malloc(sizeof(uint8)*str_ptr->size);
                        memcpy( str_ptr->data, bufwalker, sizeof(uint8)*str_ptr->size );
                        bufwalker += sizeof(uint8) * str_ptr->size;

                        args[i] = str_ptr;
                    }
                }
			}
            //dbg: print freshly converted variables
            printf("args of %s => %f, %f\n", rpc->procedures[proc_index].name,  *(int*)args[0], *(int*)args[1]);
 
			proc_return = rpc->procedures[proc_index].func(rpc, rpc->procedures[proc_index].name, rpc->procedures[proc_index].argc, args, NULL);
            
            
            if (rpc->procedures[proc_index].return_type==RPC_TYPE_FLOAT || 
                rpc->procedures[proc_index].return_type==RPC_TYPE_DOUBLE){
                //dbg printf
                //printf("return from float or double func = %f %d\n",  *(float*)proc_return, proc_return );
                memcpy(buffer, proc_return, getTypeSize(rpc->procedures[proc_index].return_type) );
                buffer[getTypeSize(rpc->procedures[proc_index].return_type)+1] = '\0';
                send(sd, buffer, getTypeSize(rpc->procedures[proc_index].return_type)+1, 0);
            } else if ( rpc->procedures[proc_index].return_type == RPC_TYPE_STRING ) {
                printf("rpc: size %d, str: %s\n", ((string*)proc_return)->size, ((string*)proc_return)->data );
                string *str_ptr = (string*)proc_return;
                int totalsize = sizeof(uint16) + str_ptr->size;
                bufwalker = buffer;
                memcpy(bufwalker, &str_ptr->size, sizeof(uint16));
                bufwalker += sizeof(uint16);

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

//TODO: selection of the IP of server is not given // int start_rpc_server(char * ip, uint16_t port){
int start_rpc_server(RPC* rpc) {
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


int rpc_add(RPC* rpc, RPC_Procedure* procedure) {
	// TODO: Implement it
	// 1. find empty space
	// 2. insert the procedure
	// 3. return the procedure index
	// 4. if no space, return error number

	// TODO: just hardcoded for the first index
	int i=0;
	for (i = 0;  i < RPC_MAX_PROCEDURES; i++) {
		if (!strcmp(rpc->procedures[i].name, "")) {
			memcpy(&rpc->procedures[i], procedure, sizeof(*procedure));
			break;  // add and break out the loop
		}
	}
}


///// rustamchange /// helper functions /////
uint16 calculateVariablesSize(int numOfArguments, int * types){
	int totalSize = 0;
	int i=0;
	for (i = 0; i<numOfArguments ;i++) {
		switch (types[i]) {
			case RPC_TYPE_VOID:	totalSize += 0; break;
			case RPC_TYPE_BOOL:	totalSize += 1; break;
			case RPC_TYPE_UINT8:	totalSize += 1; break;
			case RPC_TYPE_UINT16:	totalSize += 2; break;
			case RPC_TYPE_UINT32:	totalSize += 4; break;
			case RPC_TYPE_UINT64:	totalSize += 8; break;
			case RPC_TYPE_INT8:	totalSize += 1; break;
			case RPC_TYPE_INT16:	totalSize += 2; break;
			case RPC_TYPE_INT32:	totalSize += 4; break;
			case RPC_TYPE_INT64:	totalSize += 8; break;
			case RPC_TYPE_FLOAT:	totalSize += 4; break;
			case RPC_TYPE_DOUBLE:	totalSize += 8; break;
			
			// TODO: special cases and should be processed differently
			//case RPC_TYPE_STRING:
			//case RPC_TYPE_ARRAY:
		}
	}
	return totalSize;
}

uint16 getProcedureArgsSize(RPC_Procedure*  procedure) {
    int totalSize = 0;
    int i=0;
    for (i = 0; i < procedure->argc; i++) {
        totalSize += getTypeSize(procedure->types[i]);
    }
    return totalSize;
}

uint32 getTypeSize(int inp_type) {
	switch (inp_type) {
		case RPC_TYPE_VOID:     return 0;
		case RPC_TYPE_BOOL:
		case RPC_TYPE_UINT8:
		case RPC_TYPE_INT8:     return 1;
		case RPC_TYPE_UINT16:
		case RPC_TYPE_INT16:    return 2;
		case RPC_TYPE_FLOAT:
		case RPC_TYPE_INT32:
		case RPC_TYPE_UINT32:   return 4;
		case RPC_TYPE_UINT64:
		case RPC_TYPE_INT64:
		case RPC_TYPE_DOUBLE:   return 8;

        // TODO: special cases and should be processed differently
        //case RPC_TYPE_STRING:
        //case RPC_TYPE_ARRAY:
	}
	return -1;
}


/////////////////////////////////////////////

#define MAX_NUM_OF_ARGUMENTS	10
void* rpc_invoke(RPC* rpc, const char* name, ...) {
	// TODO: Implement it
	// 1. parse args
	// 2. Make a message
	// 3. Send the message
	// 4. Wait for a while
	//    If time out, set errno and return NULL
	// 5. Parse return value
	// 6. Return it

	int i,j;
	uint8 *bytearray;
	uint32 totalsize = 0;


	// list out existing procuedure names in current rpc object
	int proc_index = -1; // index of procedure saved at procedure search time
	for (i=0;i<RPC_MAX_PROCEDURES;i++) {
		//printf("rpc->procedures[i].name = %s\n", rpc->procedures[i].name);
		if (!strcmp(rpc->procedures[i].name, name)) {
			proc_index = i;
			break;
		}
	}

	if (proc_index != -1) {
		//bytearray = (uint8*) malloc(sizeof(uint8)*(totalsize + RPC_MAX_NAME +sizeof(int)));
		bytearray = (uint8*) malloc(MAX_BUFFER_SIZE);

		//TODO: now argc taken as an offset for remaining arguments
		char *p = bytearray;

		//////////////// saving into bytearray ///////////////
		va_list valist;
		va_start(valist, rpc->procedures[proc_index].argc);

		// first save name of procedure
		memcpy(p, rpc->procedures[proc_index].name, RPC_MAX_NAME);
		p += RPC_MAX_NAME;

		// second integer value for number of arguments argc (even though one byte would be enough just following the structure :P )
		memcpy(p, &rpc->procedures[proc_index].argc, sizeof(int));
		p += sizeof(int);


		

		//// next the list of arguments are coming
		for (i=0; i < rpc->procedures[proc_index].argc; i++){
			int cur_arg = 0;
            float f_cur_arg = 0.0;
            double d_cur_arg = 0;
			
			printf("---- starting rpc_invoke.... types = %d --- \n", rpc->procedures[proc_index].types[i]);

			if (rpc->procedures[proc_index].types[i] < RPC_TYPE_STRING) {

				switch (rpc->procedures[proc_index].types[i]) {
					case RPC_TYPE_BOOL:	cur_arg = va_arg(valist, bool);	break;
					case RPC_TYPE_UINT8:	printf("--uint8\n"); cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_INT8:	printf("--int8\n"); cur_arg = va_arg(valist, int);	break;
					case RPC_TYPE_UINT16:	printf("--uint16\n"); cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_INT16:	printf("--int16\n");cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_FLOAT:	f_cur_arg = va_arg(valist, double); printf("invoke float arg = %f\n", f_cur_arg); break;
					case RPC_TYPE_INT32:	printf("--int32\n"); cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_UINT32:	printf("--uint32\n"); cur_arg = va_arg(valist, int); printf("++uint32\n"); break;
					case RPC_TYPE_UINT64:	cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_INT64:	cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_DOUBLE:	d_cur_arg = va_arg(valist, double); printf("invoke double arg = %f\n",d_cur_arg); break;

					// TODO: special cases and should be processed differently
					case RPC_TYPE_STRING:	cur_arg = va_arg(valist, int); printf("RPC_TYPE_STRING: cur_arg=0x%x\n", cur_arg); break;
					//case RPC_TYPE_ARRAY:
				}

				printf("before the calculateVariblesize \n");
				totalsize += getTypeSize(rpc->procedures[proc_index].types[i]);
				printf("totalsize = %d\n", totalsize);


				//tmp debug:
				printf("calculateVariableSize = %d, type = %d\n", calculateVariablesSize(1, &rpc->procedures[proc_index].types[i]), rpc->procedures[proc_index].types[i]);
                
                //distinguish float, double and normal integers
				if (rpc->procedures[proc_index].types[i]==RPC_TYPE_FLOAT){
                    memcpy(p, &f_cur_arg, calculateVariablesSize(1, &rpc->procedures[proc_index].types[i]));
                }else if (rpc->procedures[proc_index].types[i] == RPC_TYPE_DOUBLE){
                    memcpy(p, &d_cur_arg, calculateVariablesSize(1, &rpc->procedures[proc_index].types[i]));
                }else{
                    memcpy(p, &cur_arg, calculateVariablesSize(1, &rpc->procedures[proc_index].types[i]));
                }
				p +=  calculateVariablesSize(1, &rpc->procedures[proc_index].types[i]);
			} else {
				if (rpc->procedures[proc_index].types[i] == RPC_TYPE_STRING) {
					cur_arg = va_arg(valist, int);
					string *str_ptr = cur_arg;
					printf("RPC_TYPE_STRING: cur_arg=0x%x\n", cur_arg);
					printf("RPC_TYPE_STRING: str_ptr=0x%x\n", str_ptr);
					//printf("RPC_TYPE_STRING is triggered size = %d\n", str_ptr->size);
					printf("size of string argument = %d, data= %s \n", str_ptr->size, str_ptr->data);

					totalsize += sizeof(uint16) + str_ptr->size;

					memcpy(p, &str_ptr->size, sizeof(uint16));
					p += sizeof(uint16);

					memcpy(p, str_ptr->data, str_ptr->size);
					p += str_ptr->size;
				}
			}
		}

		va_end(valist);
		////////////////////////////////////////////////////////

        //dbg: bytes
        int dbg_i=0;
        for(dbg_i=0;dbg_i<totalsize+RPC_MAX_NAME+sizeof(int);dbg_i++){
            printf("%X ", bytearray[dbg_i]);
        }
        printf("\n");


		// send the bytearray 
		int write_res = write(rpc->sockfd, bytearray, (totalsize + RPC_MAX_NAME + sizeof(int)));
		if (write_res < 0) {
			error("ERROR writing to socket");
		}
		else {
			int read_res = read(rpc->sockfd, bytearray, MAX_BUFFER_SIZE);
			if (read_res < 0) {
				error("ERROR reading from socket");
			}
			else if(read_res == 0) { // timeout happened
				errno = ETIMEDOUT;
				printf("rpc_invoke: timeout at receiving\n");

				free(bytearray);
				return NULL;
			}
			else {
                if (rpc->procedures[proc_index].return_type < RPC_TYPE_STRING) {
    				void * return_result = malloc(rpc->procedures[proc_index].return_type);
		            memcpy(return_result, bytearray, sizeof(int));
	    			free(bytearray);
		    		return (int*)return_result;
                } else {
                    if (rpc->procedures[proc_index].return_type == RPC_TYPE_STRING) {
                        string * str_ptr = (string*) malloc(sizeof(string));
                        uint8 * bufwalker = bytearray;
                        memcpy( &str_ptr->size, bufwalker, sizeof(uint16) );
                        bufwalker += sizeof(uint16);

                        str_ptr->data = (uint8*)malloc(sizeof(uint8)*str_ptr->size);
                        memcpy( str_ptr->data, bufwalker, sizeof(uint8)*str_ptr->size );
                        bufwalker += sizeof(uint8) * str_ptr->size;

                        //dbg
                        printf("-->received string size = %d, str: %s\n", str_ptr->size, str_ptr->data);

                        return str_ptr;
                    }
                }
			}
		}
	} else {
		printf("rpc.c: The procedure is not found !!! str: %s \n", name);
	}

	return NULL;
}

RPC * rpc_create(RPC* rpc, char * server_ip_addr, uint16_t port, const char * username, const char *salted_password){
	if (rpc == NULL) {
		rpc = (RPC *)malloc(sizeof(RPC));
	}

	int portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[MAX_BUFFER_SIZE];

	portno = port;
	rpc->sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (rpc->sockfd < 0) 
        error("ERROR opening socket");


	server = gethostbyname(server_ip_addr);
    if (server == NULL) {
        printf("ERROR, no such host\n");
        exit(0);
    }

	bzero((char *) &serv_addr, sizeof(serv_addr));	

	serv_addr.sin_family = AF_INET;
	bcopy((char *)server->h_addr,
        (char *)&serv_addr.sin_addr.s_addr,
        server->h_length);
    serv_addr.sin_port = htons(portno);

	///// set socket receive timeout option //////////////////
	struct timeval time_value;
	time_value.tv_sec = RPC_PROCEDURE_RESPONSE_TIMEOUT_S;
	time_value.tv_usec = 0;
	setsockopt(rpc->sockfd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&time_value, sizeof(struct timeval) );
	//////////////////////////////////////////////////////////

	if (connect(rpc->sockfd,(struct sockaddr *) &serv_addr,sizeof(serv_addr)) < 0)
        error("ERROR at connecting");
	else
		printf("Connected Successfully to remote RPC Server !\n");

	return rpc;
}

int rpc_destroy(RPC* rpc){
	if (rpc != NULL){
		close(rpc->sockfd);
	}
    return 0;
}
