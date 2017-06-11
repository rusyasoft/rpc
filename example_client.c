

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


	RPC_Procedure sendstring_caller;
    strncpy(sendstring_caller.name, "sendstring", RPC_MAX_NAME);
    sendstring_caller.return_type = RPC_TYPE_STRING;
    sendstring_caller.argc = 2;
    sendstring_caller.types[0] = RPC_TYPE_STRING;
    sendstring_caller.types[1] = RPC_TYPE_STRING;
    sendstring_caller.func = NULL;
    sendstring_caller.context = NULL;
    rpc_add(myrpc, &sendstring_caller);

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

	string *str_a, *str_b;
	str_a = (string*)malloc(sizeof(string));
	str_b = (string*)malloc(sizeof(string));

	str_a->size = 11;
	str_a->data = (uint8*)malloc(sizeof(uint8)*11);
	strncpy(str_a->data, "hellorpc", 9);

	str_b->size = 15;
	str_b->data = (uint8*)malloc(sizeof(uint8)*15);
	strncpy(str_b->data, "goodbyerpc", 11);

	printf("sendstring &str_a = 0x%x &str_b=0x%x\n",&str_a, &str_b );
	printf("values: str_a.size = %d, str_b.size = %d\n", str_a->size , str_b->size);
	printf("values1: str_a.data = %s, str_b.data = %s\n ", str_a->data, str_b->data);
	printf("addresses: &str_a.size = 0x%x &str_a.data = 0x%x\n", &str_a->size, &str_a->data);
	string * s_ret = (int*)rpc_invoke(myrpc, "sendstring", str_a, str_b);
	if (s_ret != NULL)
		printf("---> sendstring return = %s (size=%d)\n", s_ret->data, s_ret->size);
	else
		printf("---> error happend at calling sendstring\n");
}
