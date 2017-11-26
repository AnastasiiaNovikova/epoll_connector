/*
 * Client.h
 *
 *  Created on: Nov 25, 2017
 *      Author: lord
 */

#ifndef CLIENT_H_
#define CLIENT_H_

#pragma once

#include "Constants.h"

typedef struct Client {
	int m_fd;
	char* m_messageBuffer;
	size_t m_bufferBytesUsed;
} Client_t, *PClient_t;

typedef struct ClientRec {
	Client_t* pClient;
	struct ClientRec* next;
} ClientRec_t, *PClientRec_t;

Client_t* create_client(int slaveSocketFd);
void release_client(Client_t** pClient);
ClientRec_t* create_client_rec(Client_t* pClient);
void release_client_rec(ClientRec_t** pClientRec);
void schedule_message_cast(Client_t* pClient, ClientRec_t* pClientList, const char* pData, size_t len);
void cast_message(Client_t* pClient, ClientRec_t* pClientList);
void broadcast(Client_t* pSender, ClientRec_t* pClientList, const char* pData, size_t len);

#endif /* CLIENT_H_ */
