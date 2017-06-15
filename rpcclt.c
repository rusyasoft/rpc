
#include "include/rpcclt.h"

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


#define MAX_NUM_OF_ARGUMENTS	10
void* rpcclt_invoke(RPC* rpc, const char* name, ...) {
	// TODO: Implement it
	// 1. parse args
	// 2. Make a message
	// 3. Send the message
	// 4. Wait for a while
	//    If time out, set errno and return NULL
	// 5. Parse return value
	// 6. Return it

	int i,j;
	uint8_t *bytearray;
	uint32_t totalsize = 0;


	// list out existing procuedure names in current rpc object
	int proc_index = -1; // index of procedure saved at procedure search time
	for (i=0;i<RPC_MAX_PROCEDURES;i++) {
		if (!strcmp(rpc->procedures[i].name, name)) {
			proc_index = i;
			break;
		}
	}

	if (proc_index != -1) {
		//bytearray = (uint8*) malloc(sizeof(uint8)*(totalsize + RPC_MAX_NAME +sizeof(int)));
		bytearray = (uint8_t*) malloc(RPC_MAX_BUFFER_SIZE);

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
			

			if (rpc->procedures[proc_index].types[i] < RPC_TYPE_STRING) {

				switch (rpc->procedures[proc_index].types[i]) {
					case RPC_TYPE_BOOL: cur_arg = va_arg(valist, int);	break;
					case RPC_TYPE_UINT8: cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_INT8: cur_arg = va_arg(valist, int);	break;
					case RPC_TYPE_UINT16: cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_INT16: cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_FLOAT: f_cur_arg = va_arg(valist, double); break;
					case RPC_TYPE_INT32: cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_UINT32: cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_UINT64: cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_INT64: cur_arg = va_arg(valist, int); break;
					case RPC_TYPE_DOUBLE: d_cur_arg = va_arg(valist, double); break;
				}

				totalsize += getTypeSize(rpc->procedures[proc_index].types[i]);

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

					totalsize += sizeof(uint16_t) + str_ptr->size;

					memcpy(p, &str_ptr->size, sizeof(uint16_t));
					p += sizeof(uint16_t);

					memcpy(p, str_ptr->data, str_ptr->size);
					p += str_ptr->size;
				}
			    // TODO: special cases with array type should be processed
			}
		}

		va_end(valist);
		////////////////////////////////////////////////////////


		// send the bytearray 
		int write_res = write(rpc->sockfd, bytearray, (totalsize + RPC_MAX_NAME + sizeof(int)));
		if (write_res < 0) {
			error("ERROR writing to socket");
		}
		else {
			int read_res = read(rpc->sockfd, bytearray, RPC_MAX_BUFFER_SIZE);
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
    				void * return_result = malloc(getTypeSize(rpc->procedures[proc_index].return_type) );
		            memcpy(return_result, bytearray, getTypeSize(rpc->procedures[proc_index].return_type));
	    			free(bytearray);
		    		return (int*)return_result;
                } else {
                    if (rpc->procedures[proc_index].return_type == RPC_TYPE_STRING) {
                        string * str_ptr = (string*) malloc(sizeof(string));
                        uint8_t * bufwalker = bytearray;
                        memcpy( &str_ptr->size, bufwalker, sizeof(uint16_t) );
                        bufwalker += sizeof(uint16_t);

                        str_ptr->data = (uint8_t*)malloc(sizeof(uint8_t)*str_ptr->size);
                        memcpy( str_ptr->data, bufwalker, sizeof(uint8_t)*str_ptr->size );
                        bufwalker += sizeof(uint8_t) * str_ptr->size;


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

RPC * rpcclt_create(RPC* rpc, char * server_ip_addr, uint16_t port, const char * username, const char *salted_password){
	if (rpc == NULL) {
		rpc = (RPC *)malloc(sizeof(RPC));
	}

	int portno, n;
	struct sockaddr_in serv_addr;
	struct hostent *server;
	char buffer[RPC_MAX_BUFFER_SIZE];

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

RPC* rpcclt_create_by_tcp(RPC *rpc, int tcp_socket_fd){
	if (rpc == NULL) {
		rpc = (RPC *)malloc(sizeof(RPC));
	}
	
	if (tcp_socket_fd != NULL){
		rpc->sockfd = tcp_socket_fd;
		return rpc;
	} else{
		return NULL;
	}
}