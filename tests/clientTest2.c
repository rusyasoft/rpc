

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

	/// procedure UINT8 2 args
	RPC_Procedure test_sum_UINT8_2args;
	strncpy(test_sum_UINT8_2args.name, "sum_UINT8_2args", RPC_MAX_NAME);
	test_sum_UINT8_2args.return_type = RPC_TYPE_UINT16;
	test_sum_UINT8_2args.argc = 2;
	test_sum_UINT8_2args.types[0] = RPC_TYPE_UINT8;
	test_sum_UINT8_2args.types[1] = RPC_TYPE_UINT8;
	test_sum_UINT8_2args.func = NULL;
	test_sum_UINT8_2args.context = NULL;
	rpc_add(test_rpc, &test_sum_UINT8_2args);

	/// procedure UINT16 2 args
	RPC_Procedure test_sum_UINT16_2args;
	strncpy(test_sum_UINT16_2args.name, "sum_UINT16_2args", RPC_MAX_NAME);
	test_sum_UINT16_2args.return_type = RPC_TYPE_UINT32;
	test_sum_UINT16_2args.argc = 2;
	test_sum_UINT16_2args.types[0] = RPC_TYPE_UINT16;
	test_sum_UINT16_2args.types[1] = RPC_TYPE_UINT16;
	test_sum_UINT16_2args.func = NULL;
	test_sum_UINT16_2args.context = NULL;
	rpc_add(test_rpc, &test_sum_UINT16_2args);

	/// procedure UINT32 2 args
	RPC_Procedure test_sum_UINT32_2args;
	strncpy(test_sum_UINT32_2args.name, "sum_UINT32_2args", RPC_MAX_NAME);
	test_sum_UINT32_2args.return_type = RPC_TYPE_UINT64;
	test_sum_UINT32_2args.argc = 2;
	test_sum_UINT32_2args.types[0] = RPC_TYPE_UINT32;
	test_sum_UINT32_2args.types[1] = RPC_TYPE_UINT32;
	test_sum_UINT32_2args.func = NULL;
	test_sum_UINT32_2args.context = NULL;
	rpc_add(test_rpc, &test_sum_UINT32_2args);

	/// procedure UINT64 2 args
	RPC_Procedure test_sum_UINT64_2args;
	strncpy(test_sum_UINT64_2args.name, "sum_UINT64_2args", RPC_MAX_NAME);
	test_sum_UINT64_2args.return_type = RPC_TYPE_UINT64;
	test_sum_UINT64_2args.argc = 2;
	test_sum_UINT64_2args.types[0] = RPC_TYPE_UINT64;
	test_sum_UINT64_2args.types[1] = RPC_TYPE_UINT64;
	test_sum_UINT64_2args.func = NULL;
	test_sum_UINT64_2args.context = NULL;
	rpc_add(test_rpc, &test_sum_UINT64_2args);

	/// procedure FLOAT 2 args
	RPC_Procedure test_sum_FLOAT_2args;
	strncpy(test_sum_FLOAT_2args.name, "sum_FLOAT_2args", RPC_MAX_NAME);
	test_sum_FLOAT_2args.return_type = RPC_TYPE_FLOAT;
	test_sum_FLOAT_2args.argc = 2;
	test_sum_FLOAT_2args.types[0] = RPC_TYPE_FLOAT;
	test_sum_FLOAT_2args.types[1] = RPC_TYPE_FLOAT;
	test_sum_FLOAT_2args.func = NULL;
	test_sum_FLOAT_2args.context = NULL;
	rpc_add(test_rpc, &test_sum_FLOAT_2args);

	/// procedure DOUBLE 2 args
	RPC_Procedure test_sum_DOUBLE_2args;
	strncpy(test_sum_DOUBLE_2args.name, "sum_DOUBLE_2args", RPC_MAX_NAME);
	test_sum_DOUBLE_2args.return_type = RPC_TYPE_DOUBLE;
	test_sum_DOUBLE_2args.argc = 2;
	test_sum_DOUBLE_2args.types[0] = RPC_TYPE_DOUBLE;
	test_sum_DOUBLE_2args.types[1] = RPC_TYPE_DOUBLE;
	test_sum_DOUBLE_2args.func = NULL;
	test_sum_DOUBLE_2args.context = NULL;
	rpc_add(test_rpc, &test_sum_DOUBLE_2args);

    /// procedure STRING 2 args
    RPC_Procedure test_string_concat_STRING_2args;
    strncpy(test_string_concat_STRING_2args.name, "string_concat", RPC_MAX_NAME);
    test_string_concat_STRING_2args.return_type = RPC_TYPE_STRING;
    test_string_concat_STRING_2args.argc = 2;
    test_string_concat_STRING_2args.types[0] = RPC_TYPE_STRING;
    test_string_concat_STRING_2args.types[1] = RPC_TYPE_STRING;
    test_string_concat_STRING_2args.func = NULL;
    test_string_concat_STRING_2args.context = NULL;
    rpc_add(test_rpc, &test_string_concat_STRING_2args);



	uint8 uint8_a = 10, uint8_b = 20;
	int * retstr = (int*)rpc_invoke(test_rpc, "sum_UINT8_2args", uint8_a, uint8_b);
	if (retstr != NULL)
		printf("clientTest2: sum_UINT8_2args, return = %d\n",  (uint16)*retstr);

	uint16 uint16_a = 10, uint16_b = 20;
	retstr = (int*)rpc_invoke(test_rpc, "sum_UINT16_2args", uint16_a, uint16_b);
	if (retstr != NULL)
		printf("clientTest2: sum_UINT16_2args, return = %d\n", *retstr);
	
	uint32 uint32_a = 10, uint32_b = 20;
	retstr = (int*)rpc_invoke(test_rpc, "sum_UINT32_2args", uint32_a, uint32_b);
	if (retstr != NULL)
		printf("clientTest2: sum_UINT32_2args, return = %d\n", *retstr);

	uint64 uint64_a = 10, uint64_b = 20;
	retstr = (int*)rpc_invoke(test_rpc, "sum_UINT64_2args", uint64_a, uint64_b);
	if (retstr != NULL)
		printf("clientTest2: sum_UINT64_2args, return = %d\n", *retstr);

	float float_a = 11.1, float_b = 22.2;
    printf("clientTest2: float_a = %f, float_b = %f\n", float_a, float_b);
	float * float_retstr = (float*)rpc_invoke(test_rpc, "sum_FLOAT_2args", float_a, float_b);
	if (float_retstr != NULL)
		printf("clientTest2: sum_FLOAT_2args, return = %f\n", *float_retstr);

	double double_a = 10.1, double_b = 20.1;
    printf("clientTest2: float_a = %f, float_b = %f\n", double_a, double_b);
	double * double_retstr = (double*)rpc_invoke(test_rpc, "sum_DOUBLE_2args", double_a, double_b);
	if (double_retstr != NULL)
		printf("clietTest2: sum_DOUBLE_2args, return = %f\n", *double_retstr);


    string *str_a, *str_b;
    str_a = (string*)malloc(sizeof(string));
    str_b = (string*)malloc(sizeof(string));

    str_a->size = 11;
    str_a->data = (uint8*)malloc(sizeof(uint8)*11);
    strncpy(str_a->data, "hellorpc", 9);

    str_b->size = 15;
    str_b->data = (uint8*)malloc(sizeof(uint8)*15);
    strncpy(str_b->data, "goodbyerpc", 11);
    string * s_ret = (int*)rpc_invoke(test_rpc, "string_concat", str_a, str_b);
    if (s_ret != NULL)
        printf("clientTest2: string_concat return = %s (size=%d)\n", s_ret->data, s_ret->size);
    else
        printf("clientTest2: error happend at calling string_concat\n");

}
