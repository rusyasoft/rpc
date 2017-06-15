/**
 * @file	rpcsvc.h
 * @author	Rustam Rakhimov <rusyasoft@gmail.com>
 *
 * @brief	
 *
 */
#ifndef __RPCSVC_H__
#define __RPCSVC_H__

#include "rpc.h"

/*
 * rpc server initialization by giving socket connection information (such as IPAddr, port, username and password)
 * 
 */
RPC* rpcsvc_create(RPC* rpc,
    int (*connect)(RPC* rpc, uint32_t addr, uint16_t port, const char* username, const char* salted_password),
	int (*disconnect)(RPC* rpc),
	int (*send)(RPC* rpc, void* buf, int size),
	int (*receive)(RPC* rpc, void* buf, int size),
	int priv_size
);

// new version
RPC* rpcsvc_create_by_tcp(RPC* rpc,
	int tcp_socket_fd,
    int (*connect)(RPC* rpc, uint32_t addr, uint16_t port, const char* username, const char* salted_password),
	int (*disconnect)(RPC* rpc),
	int (*send)(RPC* rpc, void* buf, int size),
	int (*receive)(RPC* rpc, void* buf, int size),
	int priv_size
);


//deprecated name is changed: int start_rpc_server(RPC* rpc);
// runs the the rpc server in forever cycle
int rpcsvc_run(RPC* rpc);

#endif