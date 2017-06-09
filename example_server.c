

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



int myconnect(RPC* rpc, uint32_t addr, uint16_t port, const char* username, const char* salted_password){
	printf("myconnect callbacked, ip is : %x , port : %d, username=%s, password=%s \n" ,  addr , port, username, salted_password);
	return 0;
}
int mydisconnect(RPC* rpc){
	printf("mydisconnect has been triggered\n");
	return 0;
}
int mysend(RPC* rpc, void* buf, int size){
	printf("mysend has been triggered(msg size = %d)", size);
	return 0;
}
int myreceive(RPC* rpc, void* buf, int size){
	printf("myreceive has been triggered(msg size = %d)", size);
	return 0;
}


// callback function for summing
uint32 summing_callback(RPC* rpc, char* name, int argc, void** args, void* context) {
	printf("---- inside summing_callback ----- name = %s, argc = %d\n", name, argc);
	int i =0;
	int sum = 0;
	for (i=0;i<argc;i++){
		printf("arg %d is %d\n", i+1, *(int*)args[i]);
		sum += *(int*)args[i];
	}

	return sum;
}

uint32 multiplying_callback(RPC* rpc, char* name, int argc, void** args, void* context) {
	printf("---- inside multiplying_callback ----- name = %s, argc = %d\n", name, argc);
	int i =0;
	int res = 1;
	for (i=0;i<argc;i++){
		printf("arg %d is %d\n", i+1, *(uint16*)args[i]);
		res *= *(uint16*)args[i];
	}

	return res;
}

uint32 sendstring_callback(RPC* rpc, char* name, int argc, void** args, void* context) {
        printf("---- inside sendstring_callback ----- name = %s, argc = %d\n", name, argc);
        int i =0;
        int sum = 0;
        for (i=0;i<argc;i++){
                printf("arg %d is size=%d\n", i+1, ((string*)args[i])->size);
                //sum += *(int*)args[i];
        }

        return sum;
}


int main(int arc, char ** argv){
	RPC *  myrpc = (RPC*)malloc(sizeof(RPC));
	int ret = rpc_init(myrpc, &myconnect, &mydisconnect, &mysend, &myreceive, 0);

	/// create procedure
	RPC_Procedure summing_caller;
	strncpy(summing_caller.name, "summing", RPC_MAX_NAME);
	summing_caller.return_type = RPC_TYPE_UINT32;
	summing_caller.argc = 2;
	summing_caller.types[0] = RPC_TYPE_UINT32;
	summing_caller.types[1] = RPC_TYPE_UINT32;
	//summing_caller.types[2] = RPC_TYPE_UINT8;
	summing_caller.func = &summing_callback;
	summing_caller.context = NULL;

	rpc_add(myrpc, &summing_caller);

	RPC_Procedure multiply_caller;
	strncpy(multiply_caller.name, "multiplying", RPC_MAX_NAME);
	multiply_caller.return_type = RPC_TYPE_UINT32;
	multiply_caller.argc = 2;
	multiply_caller.types[0] = RPC_TYPE_UINT16;
	multiply_caller.types[1] = RPC_TYPE_UINT16;	
	multiply_caller.func = &multiplying_callback;
	multiply_caller.context = NULL;

	rpc_add(myrpc, &multiply_caller);


	RPC_Procedure sendstring_caller;
        strncpy(sendstring_caller.name, "sendstring", RPC_MAX_NAME);
        sendstring_caller.return_type = RPC_TYPE_UINT32;
        sendstring_caller.argc = 2;
        sendstring_caller.types[0] = RPC_TYPE_STRING;
        sendstring_caller.types[1] = RPC_TYPE_STRING;
        sendstring_caller.func = &sendstring_callback;
        sendstring_caller.context = NULL;
        rpc_add(myrpc, &sendstring_caller);

	start_rpc_server(myrpc);
}
