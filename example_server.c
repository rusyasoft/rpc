

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
		printf("arg %d is %d\n", i, *(int*)args[i]);
		sum += *(int*)args[i];
	}
	
	return sum;
}

uint32 multiplying_callback(RPC* rpc, char* name, int argc, void** args, void* context) {
	printf("---- inside multiplying_callback ----- name = %s, argc = %d\n", name, argc);
	int i =0;
	int res = 1;
	for (i=0;i<argc;i++){
		printf("arg %d is %d\n", i, *(uint16*)args[i]);
		res *= *(uint16*)args[i];
	}
	
	return res;
}


int main(int arc, char ** argv){
	
	RPC *  myrpc = (RPC*)malloc(sizeof(RPC));
	
	int ret = rpc_init(myrpc, &myconnect, &mydisconnect, &mysend, &myreceive, 0);


	/// create procedure
	RPC_Procedure summing_caller;
	strncpy(summing_caller.name, "summing", RPC_MAX_NAME);
	summing_caller.return_type = RPC_TYPE_UINT32;
	summing_caller.argc = 3;
	summing_caller.types[0] = RPC_TYPE_UINT32;
	summing_caller.types[1] = RPC_TYPE_UINT32;
	summing_caller.types[2] = RPC_TYPE_UINT8;
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

	start_rpc_server(myrpc);
	
	/// version 2
	/// char * hello(char * name) {char s[200]; sprintf(s, "hello %s\n", name); return s;}

	//RPC * myrpc = rpc_create(NULL, "127.0.0.1", 12345, "rustam", "dms");  // <-- gets connection to server if non NULL returned


	//if (typeof(ret) == int)
	//	printf("int ekanku!\n");
	//printf("%d \n", jav);
	
	

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
