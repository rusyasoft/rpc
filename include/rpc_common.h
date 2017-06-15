/**
 * @file	rpc_common.h
 * @author	Rustam Rakhimov <rusyasoft@gmail.com>
 *
 * @brief	
 *
 */



#ifndef __RPC_COMMON_H__
#define __RPC_COMMON_H__

// helper function
uint16_t calculateVariablesSize(int numOfArguments, int * types);

uint16_t getProcedureArgsSize(RPC_Procedure *procedure);

uint32_t getTypeSize(int inp_type);

#endif