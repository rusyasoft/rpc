

//rustamchange// added headers
#include <stdio.h>
#include <string.h>

#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
//////////////////////////////


#include "rpc.h"


//////////////////////////////////////////////

int main(int arc, char ** argv){
	RPC * myrpc = rpc_create(NULL, "127.0.0.1", 8888, "rustam", "dms");  // <-- gets connection to server if non NULL returned

	RPC_Procedure summing_caller;
	strncpy(summing_caller.name, "summing", RPC_MAX_NAME);
	summing_caller.return_type = RPC_TYPE_UINT32;
	summing_caller.argc = 2;
	summing_caller.types[0] = RPC_TYPE_UINT32;
	summing_caller.types[1] = RPC_TYPE_UINT32;
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

	int a=0,b=0;
	printf("RPC Test (addition and multiplication), Enter Numbers:\na = ");
	scanf("%d", &a);
	printf("b = ");
	scanf("%d", &b);

	int * retstr = (int*)rpc_invoke(myrpc, "summing",a,b);
	if (retstr != NULL)
		printf("RPC_MAX_NAME = %d, summing_caller.name = %s, return = %d\n", RPC_MAX_NAME, summing_caller.name, *retstr);

	retstr = (int*)rpc_invoke(myrpc, "multiplying",a,b);
	if (retstr != NULL)
		printf("RPC_MAX_NAME = %d, multiply_caller.name = %s, return = %d\n", RPC_MAX_NAME, multiply_caller.name, *retstr);
}
