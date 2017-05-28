

//rustamchange// added headers
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
//////////////////////////////


#include "../rpc.h"

//rustamchange// added and required definitions
// i dont know why this one didn't work by default
#define NULL (0)

//////////////////////////////////////////////

int main(int arc, char ** argv){
	RPC * test_rpc = rpc_create(NULL, "127.0.0.1", 8888, "rustam", "dms");  // <-- gets connection to server if non NULL returned

	RPC_Procedure test_summing_caller_2args;
	strncpy(test_summing_caller_2args.name, "summing_2args", RPC_MAX_NAME);
	test_summing_caller_2args.return_type = RPC_TYPE_UINT32;
	test_summing_caller_2args.argc = 2;
	test_summing_caller_2args.types[0] = RPC_TYPE_UINT32;
	test_summing_caller_2args.types[1] = RPC_TYPE_UINT32;
	test_summing_caller_2args.func = NULL;
	test_summing_caller_2args.context = NULL;
	rpc_add(test_rpc, &test_summing_caller_2args);


	RPC_Procedure test_multiply_caller_2args;
	strncpy(test_multiply_caller_2args.name, "multiplying_2args", RPC_MAX_NAME);
	test_multiply_caller_2args.return_type = RPC_TYPE_UINT32;
	test_multiply_caller_2args.argc = 2;
	test_multiply_caller_2args.types[0] = RPC_TYPE_UINT16;
	test_multiply_caller_2args.types[1] = RPC_TYPE_UINT16;	
	test_multiply_caller_2args.func = NULL;
	test_multiply_caller_2args.context = NULL;	
	rpc_add(test_rpc, &test_multiply_caller_2args);

	int a=10,b=20;

	int * retstr = (int*)rpc_invoke(test_rpc, "summing_2args",a,b);
	if (retstr != NULL)
		printf("RPC_MAX_NAME = %d, summing_caller.name = %s, return = %d\n", RPC_MAX_NAME, test_summing_caller_2args.name, *retstr);

	retstr = (int*)rpc_invoke(test_rpc, "multiplying_2args",a,b);
	if (retstr != NULL)
		printf("RPC_MAX_NAME = %d, test_multiply_caller_2args.name = %s, return = %d\n", RPC_MAX_NAME, test_multiply_caller_2args.name, *retstr);
}
