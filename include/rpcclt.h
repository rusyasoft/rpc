/**
 * @file	rpcclt.h
 * @author	Rustam Rakhimov <rusyasoft@gmail.com>
 *
 * @brief	
 *
 */
#ifndef __RPCCLT_H__
#define __RPCCLT_H__

#include "rpc.h"


// invokes RPC_Procedure registered into rpc instance by its name
void* rpcclt_invoke(RPC* rpc, const char* name, ...);

/* 
 * creates rpc client by initializing its network connection to server
 * server connection information (ip, port, username, password) must be given
 *
 */
RPC* rpcclt_create(RPC* rpc, char * server_ip_addr, uint16_t port, const char* username, const char* salted_password);

/*
 * creation of rpc by given tcp socket
 * - if RPC_ANYSOCK used as a socket argument then, the new socket will be created
 */
RPC* rpcclt_create_by_tcp(RPC* rpc, int tcp_socket_fd);



#endif