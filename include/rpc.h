/**
 * @file	rpc.h
 * @author	Rustam Rakhimov <rusyasoft@gmail.com>
 *
 * @brief	
 *
 */
#ifndef __RPC_H__
#define __RPC_H__

#include <stdint.h>


#define RPC_MAGIC		0x40c04901	// RPC Procedure ver 1

#define RPC_MAX_PROCEDURES	64
#define RPC_MAX_NAME		32
#define RPC_MAX_ARGS		32
#define RPC_PROCEDURE_RESPONSE_TIMEOUT_S	100

#define RPC_MAX_BUFFER_SIZE 1025 

#define RPC_TYPE_VOID		0x00
#define RPC_TYPE_BOOL		0x01
#define RPC_TYPE_UINT8		0x02
#define RPC_TYPE_UINT16		0x03
#define RPC_TYPE_UINT32		0x04
#define RPC_TYPE_UINT64		0x05
#define RPC_TYPE_INT8		0x06
#define RPC_TYPE_INT16		0x07
#define RPC_TYPE_INT32		0x08
#define RPC_TYPE_INT64		0x09
#define RPC_TYPE_FLOAT		0x0a
#define RPC_TYPE_DOUBLE		0x0b
#define RPC_TYPE_STRING		0x0c
#define RPC_TYPE_ARRAY		0x10

#define RPC_ANYSOCK			0x50

#ifndef TRUE
	#define TRUE   1
#endif
#ifndef FALSE
	#define FALSE  0
#endif

#ifndef NULL
	#define NULL (0)
#endif


typedef struct _string {
	uint16_t	size;
	uint8_t		*data;
} string;

typedef struct _array {
	uint16_t	size;
	uint16_t	len;
	void		*data;
} array;

typedef struct _RPC RPC;

typedef struct _RPC_Procedure {
	char	name[RPC_MAX_NAME];
	int		return_type;
	int		argc;
	int		types[RPC_MAX_ARGS];
	void*(*func)(RPC* rpc, char* name, int argc, void** args, void* context);
	void*	context;
} RPC_Procedure;

struct _RPC {
	// Procedures
	char		name[RPC_MAX_NAME];			// Interface name
	uint32_t	ver;					// Interface version
	RPC_Procedure	procedures[RPC_MAX_PROCEDURES];
	
	// I/O
	int     (*connect)(RPC* rpc, uint32_t addr, uint16_t port, const char* username, const char* salted_password);
	int     (*disconnect)(RPC* rpc);
	int     (*send)(RPC* rpc, void* buf, int size);
	int     (*receive)(RPC* rpc, void* buf, int size);
	
	// Private data
	int     sockfd;
	uint8_t priv[0];
};

// adds RPC_Procedure into rpc
int rpc_add_procedure(RPC* rpc, RPC_Procedure* procedure);

#endif /* __RPC_H__ */

