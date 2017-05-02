//rustamchange// added headers
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
//////////////////////////////

//rustamchange// added and required definitions
// i dont know why this one didn't work by default
#define NULL (0)

#define TRUE   1
#define FALSE  0



#define va_arg_bysize(list, size)       ((list += size) - size)
//////////////////////////////////////////////
////// modifiable definitions

// Server listening port number
#define PORT 8888

//data buffer set 1K
#define MAX_BUFFER_SIZE 1025 

// maximum number of allowed clients
#define MAX_NUM_OF_CLIENTS 30

//////////////////////////////////////////////

#include "rpc.h"


int rpc_init(RPC* rpc,
        int(*connect)(RPC* rpc, uint32_t addr, uint16_t port, const char* username, const char* salted_password),
        int(*disconnect)(RPC* rpc),
        int(*send)(RPC* rpc, void* buf, int size),
        int(*receive)(RPC* rpc, void* buf, int size),
        int priv_size){
	
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
	if ((*new_socket = accept(master_socket, (struct sockaddr *)address, (socklen_t*)addrlen))<0)
	{
		perror("accept");
		exit(EXIT_FAILURE);
        }

	//inform user of socket number - used in send and receive commands
	printf("New connection , socket fd is %d , ip is : %s , port : %d \n" , *new_socket , inet_ntoa(address->sin_addr) , ntohs(address->sin_port));
	
	// call rpc connect callback function
	if (rpc->connect != NULL){
		rpc->connect(rpc, address->sin_addr.s_addr, ntohs(address->sin_port), "guest", "pass");
	}

	/* no need to response with greeting message 
	//send new connection greeting message
	if( send(*new_socket, message, strlen(message), 0) != strlen(message) )
	{
		perror("send");
	}
	puts("Welcome message sent successfully");
	*/



	//add new socket to array of sockets
	for (i = 0; i < MAX_NUM_OF_CLIENTS; i++)
	{
		//if position is empty
		if( client_socket[i] == 0 )
		{
			client_socket[i] = *new_socket;
			printf("Adding to list of sockets as %d\n" , i);
			break;
		}
	}
	return 0;
}

int handle_incoming_data(RPC * rpc, int clientID, int sd, int *readfds, struct sockaddr_in* address, int * addrlen, int *client_socket){
	printf("--> something happening with client %d \n", clientID);
    //Check if it was for closing , and also read the incoming message
	int valread;
	char buffer[MAX_BUFFER_SIZE];  //data buffer of 1K
	char * bufwalker;

    if ((valread = read( sd , buffer, MAX_BUFFER_SIZE-1)) == 0)
    {
        //Somebody disconnected , get his details and print
        getpeername(sd , (struct sockaddr*)address , (socklen_t*)addrlen);
        printf("Host disconnected , ip %s , port %d \n" , inet_ntoa(address->sin_addr) , ntohs(address->sin_port));

		// call rpc disconnect callback function
		if (rpc->disconnect != NULL){
			rpc->disconnect(rpc);
		}
                      
        //Close the socket and mark as 0 in list for reuse
        close( sd );
        client_socket[clientID] = 0;
    }
                  
    //Echo back the message that came in
    else
    {
		//// recovery example //// prototype code

		/*
		// first save name of procedure
		memcpy(p, rpc->procedures[proc_index].name, RPC_MAX_NAME);
		p += RPC_MAX_NAME;
		// second integer valuie for number of arguments argc (even though one byte would be enough just following the structure :P )
		memcpy(p, argc, sizeof(int));
		p += sizeof(int);
		*/

		bufwalker = buffer;

		char proc_name[RPC_MAX_NAME];
		int proc_argc;
		memcpy(proc_name, bufwalker, RPC_MAX_NAME);
		bufwalker += RPC_MAX_NAME;

		memcpy(&proc_argc, bufwalker, sizeof(int));
		bufwalker += sizeof(int);

		printf("procedure name = %s, number of arguments = %d\n", proc_name, proc_argc);

		/* /// test for recovering the received arguments and it worked well ///
		int a;
		memcpy(&a, bufwalker, sizeof(int));
		int b;
		memcpy(&b, bufwalker + sizeof(int), sizeof(int));
		printf("values came from client a = %d, b= %d\n", a,b);
		
		int c = a + b;
		// send primitive response
		memcpy(buffer, &c, sizeof(int));
		buffer[sizeof(int)] = '\0';
		send(sd, buffer, sizeof(int), 0);
		*/
		
		//TODO: search for the procedure from the RPC sturctures RPC_Procedure array
		int totalsize = 0, i=0;
		int proc_index = -1; // index of procedure saved at procedure search time
		for (i=0;i<RPC_MAX_PROCEDURES;i++){
			//printf("%d) %s,%s|\n", i+1, rpc->procedures[i].name, name );
			if (!strcmp(rpc->procedures[i].name, proc_name)){
				printf("argc = %d,\n", rpc->procedures[i].argc);
				printf("procedure found!-->\n");
				totalsize = calculateVariablesSize(rpc->procedures[i].argc, rpc->procedures[i].types);
				proc_index = i;
				printf("totalsize = %d\n", totalsize);
				break;
			}
		}
		
		//TODO: create and allocated required space for the procedure return value and Call procedure
		void *proc_return;
		if (proc_index != -1){
			// allocation space and preparation of input arguments
			void **args = (void**) malloc(sizeof(void*)*(rpc->procedures[proc_index].argc) );
			for (i=0; i < rpc->procedures[proc_index].argc; i++){
				args[i] = malloc(calculateVariablesSize(1, &rpc->procedures[proc_index].types[i])); //malloc(rpc->procedures[proc_index].types[i]);
				//copying the procedure arguments from received buffer
				memcpy(args[i], bufwalker, calculateVariablesSize(1, &rpc->procedures[proc_index].types[i]));
				printf("%d) %d    size = %d\n", i, *((short*)args[i])  ,calculateVariablesSize(1, &rpc->procedures[proc_index].types[i]) );
				bufwalker += calculateVariablesSize(1, &rpc->procedures[proc_index].types[i]); //rpc->procedures[proc_index].types[i];
				
			}
			proc_return = rpc->procedures[proc_index].func(rpc, rpc->procedures[proc_index].name, rpc->procedures[proc_index].argc, args, NULL);
			printf("proc_return = %d\n", proc_return);
			
			// send rpc response back to client
			memcpy(buffer, &proc_return, rpc->procedures[proc_index].return_type);
			buffer[rpc->procedures[proc_index].return_type] = '\0';
			send(sd, buffer, rpc->procedures[proc_index].return_type, 0);
		}else{
			printf("Couldn't find procedure named: %s\n", proc_name);
		}
		

		//rustamchange --> no reply performed so far
        //set the string terminating NULL byte on the end of the data read
        //buffer[valread] = '\0';
        //send(sd , buffer , strlen(buffer) , 0 );
    }
	return 0;
}

//TODO: selection of the IP of server is not given // int start_rpc_server(char * ip, uint16_t port){
int start_rpc_server(RPC* rpc){
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
    for (i = 0; i < MAX_NUM_OF_CLIENTS; i++) 
    {
        client_socket[i] = 0;
    }
      
    //create a master socket
    if( (master_socket = socket(AF_INET , SOCK_STREAM , 0)) == 0) 
    {
        perror("master socket creation failed");
        exit(EXIT_FAILURE);
    }
  
    //set master socket to allow multiple connections , this is just a good habit, it will work without this
    if( setsockopt(master_socket, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(opt)) < 0 )
    {
        perror("setsockopt");
        exit(EXIT_FAILURE);
    }
  
    //type of socket created
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY;
    address.sin_port = htons( PORT );
      
    //bind the socket to localhost port 8888
    if (bind(master_socket, (struct sockaddr *)&address, sizeof(address))<0) 
    {
        perror("bind failed");
        exit(EXIT_FAILURE);
    }
    printf("Listener on port %d \n", PORT);
     
    //try to specify maximum of 3 pending connections for the master socket
    if (listen(master_socket, MAX_NUM_OF_CLIENTS) < 0)
    {
        perror("listen");
        exit(EXIT_FAILURE);
    }

	//accept the incoming connection
    addrlen = sizeof(address);
    puts("Waiting for connections ...");
     
    while(TRUE) 
    {
        //clear the socket set
        FD_ZERO(&readfds);
  
        //add master socket to set
        FD_SET(master_socket, &readfds);
        max_sd = master_socket;
         
        //add child sockets to set
        for ( i = 0 ; i < MAX_NUM_OF_CLIENTS ; i++) 
        {
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
        activity = select( max_sd + 1 , &readfds , NULL , NULL , NULL);
    
        if ((activity < 0) && (errno!=EINTR)) 
        {
            printf("select error");
        }
          
        //If something happened on the master socket , then its an incoming connection
        if (FD_ISSET(master_socket, &readfds)) 
        {
			printf("master_socket something happening !\n ");
			handle_new_connection(rpc, master_socket, &address, &addrlen, &new_socket, message, &client_socket);
        }
          
        //else its some IO operation on some other socket :)
        for (i = 0; i < MAX_NUM_OF_CLIENTS; i++) 
        {
            sd = client_socket[i];
              
            if (FD_ISSET( sd , &readfds)) 
            {
				handle_incoming_data(rpc, i, sd, &readfds, &address, &addrlen, &client_socket);
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
	for (i = 0;  i < RPC_MAX_PROCEDURES; i++){
		if (!strcmp(rpc->procedures[i].name, "")){
			memcpy(&rpc->procedures[i], procedure, sizeof(*procedure));
			//printf("1 summing_caller.name = %s , %d\n", procedure->name, sizeof(*procedure));
			//printf("2 summing_caller.name = %s , %d\n", rpc->procedures[0].name , sizeof(rpc->procedures[0]));
			break;  // add and break out the loop
		}
	}
	
}


///// rustamchange /// helper functions /////
uint16 calculateVariablesSize(int numOfArguments, int * types){
	int totalSize = 0;
	int i=0;
	for (i = 0; i<numOfArguments ;i++){
		switch (types[i]){
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

uint32 getTypeSize(int inp_type){ 
        switch (inp_type){
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
	uint32 totalsize;


	// list out existing procuedure names in current rpc object
	int proc_index = -1; // index of procedure saved at procedure search time
	for (i=0;i<RPC_MAX_PROCEDURES;i++){
		printf("%d) %s,%s|\n", i+1, rpc->procedures[i].name, name );
		if (!strcmp(rpc->procedures[i].name, name)){
			printf("argc = %d,\n", rpc->procedures[i].argc);
			printf("procedure found!-->\n");
			totalsize = calculateVariablesSize(rpc->procedures[i].argc, rpc->procedures[i].types);
			proc_index = i;
			printf("totalsize = %d\n", totalsize);
			break;
		}
	}

	if (proc_index != -1){
		// procedure name 32 and argc=sizeof(int) must be added	
		bytearray = (uint8*)malloc(sizeof(uint8)*(totalsize + RPC_MAX_NAME +sizeof(int)));
		

		//TODO: now argc taken as a offset for remaining arguments
		char *p = bytearray;//(char *) &arg_c + sizeof(arg_c);
		//char *p = (char *) rpc + sizeof(*rpc);
		
		/*
		//TODO: value based variables structured thats why copy is possible, when the variables changes to pointers then its gonna fail
		memcpy(bytearray, p, totalsize);
		printf("*p = %u\n", *p);

		printf("total bytes: ");
		for(i=0; i<totalsize+200;i++){
			printf("%u ", p[i]);//bytearray[i]);
		}
		printf("\n");
		*/

		printf(" saving into bytearray 1, proc_index = %d, argc = %d \n", proc_index, rpc->procedures[proc_index].argc);
		
		//////////////// saving into bytearray ///////////////
		va_list valist;
		va_start(valist, rpc->procedures[proc_index].argc);
		
		printf(" saving into bytearray 2, sizeofvalist = %d, valist=%d \n", sizeof(valist), valist);
		
		// first save name of procedure
		memcpy(p, rpc->procedures[proc_index].name, RPC_MAX_NAME);
		p += RPC_MAX_NAME;

		printf(" saving into bytearray 3 argc = %d\n", rpc->procedures[proc_index].argc);
		int tmp2 = rpc->procedures[proc_index].argc;
		// second integer valuie for number of arguments argc (even though one byte would be enough just following the structure :P )
		//memcpy(p, &rpc->procedures[proc_index].argc, sizeof(int));
		memcpy(p, &tmp2, sizeof(int));
		printf(" saving into bytearray 4 \n");
		p += sizeof(int);

		

		printf(" Starting parsing arguments into bytearray \n");
		//// next the list of arguments are coming
		for (i=0; i < rpc->procedures[proc_index].argc; i++){
		
			//TODO: not sure why the 4 works, may be its because pointer type. Should be researched more 
			void * tmp = (void*)va_arg_bysize(valist, 4);//calculateVariablesSize(1, &rpc->procedures[proc_index].types[i])); //rpc->procedures[proc_index].types[i]);
			memcpy(p, tmp, calculateVariablesSize(1, &rpc->procedures[proc_index].types[i])); // rpc->procedures[proc_index].types[i]);
			p +=  calculateVariablesSize(1, &rpc->procedures[proc_index].types[i]); //rpc->procedures[proc_index].types[i];
			printf("%d (realsize = %d) (size is %d) => %d, addr=%x\n",i+1, calculateVariablesSize(1, &rpc->procedures[proc_index].types[i]), sizeof(tmp), *(uint32*)tmp, tmp);
		}
		va_end(valist);

		// checking by printing bytearray ////
		printf("total bytes: ");
		for(i=0; i<totalsize;i++){
				printf("%u ", bytearray[i]);
		}
		printf("\n");

		/* //// recovery example ////// it worked
		int a;
		memcpy(&a, bytearray + 32 + sizeof(int), sizeof(int));
		int b;
		memcpy(&b, bytearray + 32 + sizeof(int) + sizeof(int), sizeof(int));
		printf("a = %d, b= %d\n", a, b);
		*/
		////////////////////////////////////////////////////////

		// send the bytearray 
		int write_res = write(rpc->sockfd, bytearray, (totalsize + RPC_MAX_NAME + sizeof(int)));// totalsize);
		if (write_res < 0) {
			error("ERROR writing to socket");
		}
		else {
			printf("RPC Invoke send Waiting for response ...\n");
			int read_res = read(rpc->sockfd, bytearray, MAX_BUFFER_SIZE);
			if (read_res < 0) {
				error("ERROR reading from socket");
			}
			else {
				//int resultc;
				void * return_result = malloc(rpc->procedures[proc_index].return_type);
				memcpy(return_result, bytearray, sizeof(int));
				printf("response -> %s or %d\n", bytearray, *(int*)(return_result));
				return (int*)return_result;
			}
			
		
		}
	}else{
		printf("The procedure is not found !!!\n");
	}
	
	return NULL;
	
	/////////////////////

	
	
	
	

	return NULL;
}


//RPC * rpc_create(RPC* rpc, uint32_t addr, uint16_t port, const char * username, *salted_password){
RPC * rpc_create(RPC* rpc, char * server_ip_addr, uint16_t port, const char * username, const char *salted_password){

	if (rpc == NULL){
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
	//serv_addr.sin_addr.s_addr = addr;
	bcopy((char *)server->h_addr, 
         (char *)&serv_addr.sin_addr.s_addr,
         server->h_length);
	serv_addr.sin_port = htons(portno);
	

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
