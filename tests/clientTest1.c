 /**
 * @file	clientTest1.c
 * @author	Rustam Rakhimov <rusyasoft@gmail.com>
 *
 * @brief	Test case that checks simple integer based rpc calls
 * @year	2017
 */


//rustamchange// added headers
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
//////////////////////////////

#include "../include/rpc.h"

//////////////////////////////////////////////

int main(int arc, char ** argv){
	RPC * test_rpc = rpcclt_create(NULL, "127.0.0.1", 8888, "rustam", "dms");  // <-- gets connection to server if non NULL returned

	
	printf("-------test0\n");
	
	RPC_Procedure test_summing_caller_2args;
	strncpy(test_summing_caller_2args.name, "summing_2args", RPC_MAX_NAME);
	test_summing_caller_2args.return_type = RPC_TYPE_UINT32;
	test_summing_caller_2args.argc = 2;
	test_summing_caller_2args.types[0] = RPC_TYPE_UINT32;
	test_summing_caller_2args.types[1] = RPC_TYPE_UINT32;
	test_summing_caller_2args.func = NULL;
	test_summing_caller_2args.context = NULL;
	rpc_add_procedure(test_rpc, &test_summing_caller_2args);


	RPC_Procedure test_multiply_caller_2args;
	strncpy(test_multiply_caller_2args.name, "multiplying_2args", RPC_MAX_NAME);
	test_multiply_caller_2args.return_type = RPC_TYPE_UINT32;
	test_multiply_caller_2args.argc = 2;
	test_multiply_caller_2args.types[0] = RPC_TYPE_UINT16;
	test_multiply_caller_2args.types[1] = RPC_TYPE_UINT16;	
	test_multiply_caller_2args.func = NULL;
	test_multiply_caller_2args.context = NULL;	
	rpc_add_procedure(test_rpc, &test_multiply_caller_2args);

	int a=10,b=20;
	
	printf("-------test1\n");
	
	int * retstr = (int*)rpcclt_invoke(test_rpc, "summing_2args",a,b);
	if (retstr != NULL)
		printf("RPC_MAX_NAME = %d, summing_caller.name = %s, return = %d\n", RPC_MAX_NAME, test_summing_caller_2args.name, *retstr);
		
	printf("-------test2\n");

	retstr = (int*)rpcclt_invoke(test_rpc, "multiplying_2args",a,b);
	if (retstr != NULL)
		printf("RPC_MAX_NAME = %d, test_multiply_caller_2args.name = %s, return = %d\n", RPC_MAX_NAME, test_multiply_caller_2args.name, *retstr);
}
