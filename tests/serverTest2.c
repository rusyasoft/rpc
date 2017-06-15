 /**
 * @file	serverTest2.c
 * @author	Rustam Rakhimov <rusyasoft@gmail.com>
 *
 * @brief	Test case that checks simple integer based rpc calls
 * @year	2017
 */


//rustamchange// added headers
#include <stdio.h>
#include <string.h>
#include <stdint.h>
#include <stdarg.h>
#include <ctype.h>
//////////////////////////////


#include "../include/rpc.h"

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
uint16_t test_sum_callback_UINT8_args(RPC* rpc, char* name, int argc, void** args, void* context) {
	printf("---- inside summing_callback_UINT8 ----- name = %s, argc = %d\n", name, argc);
	int i =0;
	uint16_t sum = 0;
	for (i=0;i<argc;i++){
		sum += *(int8_t*)args[i];
	}
	return sum;
}

uint32_t test_sum_callback_UINT16_args(RPC* rpc, char* name, int argc, void** args, void* context) {
	printf("---- inside summing_callback_UINT16 ----- name = %s, argc = %d\n", name, argc);
	int i =0;
	uint32_t sum = 0;
	for (i=0;i<argc;i++){
		sum += *(int16_t*)args[i];
	}
	return sum;
}

uint64_t test_sum_callback_UINT32_args(RPC* rpc, char* name, int argc, void** args, void* context) {
	printf("---- inside summing_callback_UINT32 ----- name = %s, argc = %d\n", name, argc);
	int i =0;
	uint64_t sum = 0;
	for (i=0;i<argc;i++){
		sum += *(int32_t*)args[i];
	}
	return sum;
}

uint64_t test_sum_callback_UINT64_args(RPC* rpc, char* name, int argc, void** args, void* context) {
	printf("---- inside summing_callback_UINT64 ----- name = %s, argc = %d\n", name, argc);
	int i =0;
	uint64_t sum = 0;
	for (i=0;i<argc;i++){
		sum += *(int64_t*)args[i];
	}
	return sum;
}

float* test_sum_callback_FLOAT_args(RPC* rpc, char* name, int argc, void** args, void* context) {
	printf("---- inside summing_callback_float ----- name = %s, argc = %d\n", name, argc);
	int i =0;
	float sum = 0;
	for (i=0;i<argc;i++){
		sum += *(float*)args[i];
	}
    float* f_result = (float*) malloc(sizeof(float));
    *f_result = sum;
	return f_result;
}

double* test_sum_callback_DOUBLE_args(RPC* rpc, char* name, int argc, void** args, void* context) {
	printf("---- inside summing_callback_double ----- name = %s, argc = %d\n", name, argc);
	int i =0;
	double sum = 0;
	for (i=0;i<argc;i++){
		sum += *(double*)args[i];
	}
    double* d_result = (double*) malloc(sizeof(double));
    *d_result = sum;
	return d_result;
}

string * test_string_concat_callback_STRING_args(RPC* rpc, char* name, int argc, void** args, void *context){
    int i =0;
    printf("---- stringconcat function has been triggered ----\n");

    string * str_ptr = (string*)malloc(sizeof(string));
    str_ptr->size = ((string*)args[0])->size + ((string*)args[1])->size + 5;
    str_ptr->data = (uint8_t*)malloc(sizeof(uint8_t)*str_ptr->size);
    strcpy(str_ptr->data, ((string*)args[0])->data);
    strcat(str_ptr->data, " ");
    strcat(str_ptr->data, ((string*)args[1])->data);

    return str_ptr;
}

int main(int arc, char ** argv) {
	RPC *  test_rpc = rpcsvc_create(NULL, &test_connect, &test_disconnect, &test_send, &test_receive, 0);

	/// procedure UINT8 2 args
	RPC_Procedure test_sum_UINT8_2args;
	strncpy(test_sum_UINT8_2args.name, "sum_UINT8_2args", RPC_MAX_NAME);
	test_sum_UINT8_2args.return_type = RPC_TYPE_UINT16;
	test_sum_UINT8_2args.argc = 2;
	test_sum_UINT8_2args.types[0] = RPC_TYPE_UINT8;
	test_sum_UINT8_2args.types[1] = RPC_TYPE_UINT8;
	test_sum_UINT8_2args.func = &test_sum_callback_UINT8_args;
	test_sum_UINT8_2args.context = NULL;
	rpc_add_procedure(test_rpc, &test_sum_UINT8_2args);

	/// procedure UINT16 2 args
	RPC_Procedure test_sum_UINT16_2args;
	strncpy(test_sum_UINT16_2args.name, "sum_UINT16_2args", RPC_MAX_NAME);
	test_sum_UINT16_2args.return_type = RPC_TYPE_UINT32;
	test_sum_UINT16_2args.argc = 2;
	test_sum_UINT16_2args.types[0] = RPC_TYPE_UINT16;
	test_sum_UINT16_2args.types[1] = RPC_TYPE_UINT16;
	test_sum_UINT16_2args.func = &test_sum_callback_UINT16_args;
	test_sum_UINT16_2args.context = NULL;
	rpc_add_procedure(test_rpc, &test_sum_UINT16_2args);

	/// procedure UINT32 2 args
	RPC_Procedure test_sum_UINT32_2args;
	strncpy(test_sum_UINT32_2args.name, "sum_UINT32_2args", RPC_MAX_NAME);
	test_sum_UINT32_2args.return_type = RPC_TYPE_UINT64;
	test_sum_UINT32_2args.argc = 2;
	test_sum_UINT32_2args.types[0] = RPC_TYPE_UINT32;
	test_sum_UINT32_2args.types[1] = RPC_TYPE_UINT32;
	test_sum_UINT32_2args.func = &test_sum_callback_UINT32_args;
	test_sum_UINT32_2args.context = NULL;
	rpc_add_procedure(test_rpc, &test_sum_UINT32_2args);

	/// procedure UINT64 2 args
	RPC_Procedure test_sum_UINT64_2args;
	strncpy(test_sum_UINT64_2args.name, "sum_UINT64_2args", RPC_MAX_NAME);
	test_sum_UINT64_2args.return_type = RPC_TYPE_UINT64;
	test_sum_UINT64_2args.argc = 2;
	test_sum_UINT64_2args.types[0] = RPC_TYPE_UINT64;
	test_sum_UINT64_2args.types[1] = RPC_TYPE_UINT64;
	test_sum_UINT64_2args.func = &test_sum_callback_UINT64_args;
	test_sum_UINT64_2args.context = NULL;
	rpc_add_procedure(test_rpc, &test_sum_UINT64_2args);

	/// procedure FLOAT 2 args
	RPC_Procedure test_sum_FLOAT_2args;
	strncpy(test_sum_FLOAT_2args.name, "sum_FLOAT_2args", RPC_MAX_NAME);
	test_sum_FLOAT_2args.return_type = RPC_TYPE_FLOAT;
	test_sum_FLOAT_2args.argc = 2;
	test_sum_FLOAT_2args.types[0] = RPC_TYPE_FLOAT;
	test_sum_FLOAT_2args.types[1] = RPC_TYPE_FLOAT;
	test_sum_FLOAT_2args.func = &test_sum_callback_FLOAT_args;
	test_sum_FLOAT_2args.context = NULL;
	rpc_add_procedure(test_rpc, &test_sum_FLOAT_2args);

	/// procedure DOUBLE 2 args
	RPC_Procedure test_sum_DOUBLE_2args;
	strncpy(test_sum_DOUBLE_2args.name, "sum_DOUBLE_2args", RPC_MAX_NAME);
	test_sum_DOUBLE_2args.return_type = RPC_TYPE_DOUBLE;
	test_sum_DOUBLE_2args.argc = 2;
	test_sum_DOUBLE_2args.types[0] = RPC_TYPE_DOUBLE;
	test_sum_DOUBLE_2args.types[1] = RPC_TYPE_DOUBLE;
	test_sum_DOUBLE_2args.func = &test_sum_callback_DOUBLE_args;
	test_sum_DOUBLE_2args.context = NULL;
	rpc_add_procedure(test_rpc, &test_sum_DOUBLE_2args);

    /// procedure STRING 2 arguments
    RPC_Procedure string_concat_caller;
    strncpy(string_concat_caller.name, "string_concat", RPC_MAX_NAME);
    string_concat_caller.return_type = RPC_TYPE_STRING;
    string_concat_caller.argc = 2;
    string_concat_caller.types[0] = RPC_TYPE_STRING;
    string_concat_caller.types[1] = RPC_TYPE_STRING;
    string_concat_caller.func = &test_string_concat_callback_STRING_args;
    string_concat_caller.context = NULL;
    rpc_add_procedure(test_rpc, &string_concat_caller);

	rpcsvc_run(test_rpc);
}
