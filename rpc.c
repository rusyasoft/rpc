//rustamchange// added headers
#include <stdio.h>

#include <stdint.h>




#include "include/rpc.h"



#define va_arg_bysize(list, size)       (( ((char*)list) += size) - size)

/////////// modifiable definitions //////////////

// Server listening port number
#define PORT 8888



// maximum number of allowed clients
#define MAX_NUM_OF_CLIENTS 30

/////////////////////////////////////////////////



int rpc_add_procedure(RPC* rpc, RPC_Procedure* procedure) {
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

/////////////////////////////////////////////



int rpc_destroy(RPC* rpc){

	if (rpc != NULL){
		close(rpc->sockfd);
	}
    return 0;
}
