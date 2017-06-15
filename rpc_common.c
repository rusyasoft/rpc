#include <stdint.h>

#include "include/rpc.h"
#include "include/rpc_common.h"

uint16_t calculateVariablesSize(int numOfArguments, int * types){
	int totalSize = 0;
	int i=0;
	for (i = 0; i<numOfArguments ;i++) {
		switch (types[i]) {
			case RPC_TYPE_VOID:	totalSize += 0; break;
			case RPC_TYPE_BOOL:	totalSize += 1; break;
			case RPC_TYPE_UINT8:	totalSize += 1; break;
			case RPC_TYPE_UINT16:	totalSize += 2; break;
			case RPC_TYPE_UINT32:	totalSize += 4; break;
			case RPC_TYPE_UINT64:	totalSize += 8; break;
			case RPC_TYPE_INT8:	totalSize += 1; break;
			case RPC_TYPE_INT16:	totalSize += 2; break;
			case RPC_TYPE_INT32:	totalSize += 4; break;
			case RPC_TYPE_INT64:	totalSize += 8; break;
			case RPC_TYPE_FLOAT:	totalSize += 4; break;
			case RPC_TYPE_DOUBLE:	totalSize += 8; break;
			
			// TODO: special cases and should be processed differently
			//case RPC_TYPE_STRING:
			//case RPC_TYPE_ARRAY:
		}
	}
	return totalSize;
}

uint16_t getProcedureArgsSize(RPC_Procedure*  procedure) {
    int totalSize = 0;
    int i=0;
    for (i = 0; i < procedure->argc; i++) {
        totalSize += getTypeSize(procedure->types[i]);
    }
    return totalSize;
}

uint32_t getTypeSize(int inp_type) {
	switch (inp_type) {
		case RPC_TYPE_VOID:     return 0;
		case RPC_TYPE_BOOL:
		case RPC_TYPE_UINT8:
		case RPC_TYPE_INT8:     return 1;
		case RPC_TYPE_UINT16:
		case RPC_TYPE_INT16:    return 2;
		case RPC_TYPE_FLOAT:
		case RPC_TYPE_INT32:
		case RPC_TYPE_UINT32:   return 4;
		case RPC_TYPE_UINT64:
		case RPC_TYPE_INT64:
		case RPC_TYPE_DOUBLE:   return 8;

        // TODO: special cases and should be processed differently
        //case RPC_TYPE_STRING:
        //case RPC_TYPE_ARRAY:
	}
	return -1;
}