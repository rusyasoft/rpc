

//rustamchange// added headers
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
//////////////////////////////


#include "rpc.h"

//rustamchange// added and required definitions
// i dont know why this one didn't work by default
#define NULL (0)

//////////////////////////////////////////////

int main(int arc, char ** argv){

	RPC * myrpc = rpc_create(NULL, "117.16.136.173", 8888, "rustam", "dms");  // <-- gets connection to server if non NULL returned


	//if (typeof(ret) == int)
	//	printf("int ekanku!\n");
	//printf("%d \n", jav);
	
	RPC_Procedure summing_caller;
	strncpy(summing_caller.name, "summing", RPC_MAX_NAME);
	summing_caller.return_type = RPC_TYPE_UINT32;
	summing_caller.argc = 3;
	summing_caller.types[0] = RPC_TYPE_UINT32;
	summing_caller.types[1] = RPC_TYPE_UINT32;
	summing_caller.types[2] = RPC_TYPE_UINT8;
	summing_caller.func = NULL;
	summing_caller.context = NULL;
	rpc_add(myrpc, &summing_caller);
	
	
	RPC_Procedure multiply_caller;
	strncpy(multiply_caller.name, "multiplying", RPC_MAX_NAME);
	multiply_caller.return_type = RPC_TYPE_UINT32;
	multiply_caller.argc = 2;
	multiply_caller.types[0] = RPC_TYPE_UINT16;
	multiply_caller.types[1] = RPC_TYPE_UINT16;	
	multiply_caller.func = NULL;
	multiply_caller.context = NULL;	
	rpc_add(myrpc, &multiply_caller);

	int * retstr = (int*)rpc_invoke(myrpc, "summing",400,500, 16);
	if (retstr != NULL)
		printf("RPC_MAX_NAME = %d, summing_caller.name = %s, return = %d\n", RPC_MAX_NAME, summing_caller.name, *retstr);
	
	
	retstr = (int*)rpc_invoke(myrpc, "multiplying",20,30);
	if (retstr != NULL)
		printf("RPC_MAX_NAME = %d, multiply_caller.name = %s, return = %d\n", RPC_MAX_NAME, multiply_caller.name, *retstr);

	/* server side

	RPC *  myrpc = (RPC*)malloc(sizeof(RPC));
        int ret = rpc_init(myrpc, &myconnect, &mydisconnect, &mysend, &myreceive, 0);


	RPC_Procedure r_hello;
	r_hello.name = "hello";
	r_hello.return_type = RPC_TYPE_STRING;
	r_hello.argc = 1;
	r_hello.types[0] = RPC_TYPE_STRING;
	r_hello.func = actualfunction();
	*/

}
