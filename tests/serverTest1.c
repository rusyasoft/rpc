/*
 * Author:	Rustam Rakhimov
 * Description:	Test case that checks simple integer based rpc calls
 * Year:	2017
 */

//rustamchange// added headers
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
//////////////////////////////


#include "../rpc.h"

//rustamchange// added and required definitions
// i dont know why this one didn't work by default
//#define NULL (0)

//////////////////////////////////////////////



int test_connect(RPC* rpc, uint32_t addr, uint16_t port, const char* username, const char* salted_password){
	printf("test_connect callbacked, ip is : %x , port : %d, username=%s, password=%s \n" ,  addr , port, username, salted_password);
	return 0;
}
int test_disconnect(RPC* rpc){
	printf("test_disconnect has been triggered\n");
	return 0;
}
int test_send(RPC* rpc, void* buf, int size){
	printf("test_send has been triggered(msg size = %d)", size);
	return 0;
}
int test_receive(RPC* rpc, void* buf, int size){
	printf("test_receive has been triggered(msg size = %d)", size);
	return 0;
}


// callback function for summing
uint32 test_sum_callback_2args(RPC* rpc, char* name, int argc, void** args, void* context) {
	printf("---- inside summing_callback ----- name = %s, argc = %d\n", name, argc);
	int i =0;
	int sum = 0;
	for (i=0;i<argc;i++){
		printf("arg %d is %d\n", i+1, *(int*)args[i]);
		sum += *(int*)args[i];
	}

	return sum;
}

uint32 test_multiply_callback_2args(RPC* rpc, char* name, int argc, void** args, void* context) {
	printf("---- inside multiplying_callback ----- name = %s, argc = %d\n", name, argc);
	int i =0;
	int res = 1;
	for (i=0;i<argc;i++){
		printf("arg %d is %d\n", i+1, *(uint16*)args[i]);
		res *= *(uint16*)args[i];
	}
	return res;
}


int main(int arc, char ** argv){
	RPC *  test_rpc = (RPC*)malloc(sizeof(RPC));
	int ret = rpc_init(test_rpc, &test_connect, &test_disconnect, &test_send, &test_receive, 0);

	/// create procedure
	RPC_Procedure test_summing_caller_2args;
	strncpy(test_summing_caller_2args.name, "summing_2args", RPC_MAX_NAME);
	test_summing_caller_2args.return_type = RPC_TYPE_UINT32;
	test_summing_caller_2args.argc = 2;
	test_summing_caller_2args.types[0] = RPC_TYPE_UINT32;
	test_summing_caller_2args.types[1] = RPC_TYPE_UINT32;
	test_summing_caller_2args.func = &test_sum_callback_2args;
	test_summing_caller_2args.context = NULL;

	rpc_add(test_rpc, &test_summing_caller_2args);

	RPC_Procedure test_multiply_caller_2args;
	strncpy(test_multiply_caller_2args.name, "multiplying_2args", RPC_MAX_NAME);
	test_multiply_caller_2args.return_type = RPC_TYPE_UINT32;
	test_multiply_caller_2args.argc = 2;
	test_multiply_caller_2args.types[0] = RPC_TYPE_UINT16;
	test_multiply_caller_2args.types[1] = RPC_TYPE_UINT16;	
	test_multiply_caller_2args.func = &test_multiply_callback_2args;
	test_multiply_caller_2args.context = NULL;

	rpc_add(test_rpc, &test_multiply_caller_2args);

	start_rpc_server(test_rpc);
}
