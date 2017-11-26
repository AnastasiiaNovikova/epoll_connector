/*
 * Client.c
 *
 *  Created on: Nov 25, 2017
 *      Author: lord
 */


#include "Client.h"

Client_t* create_client(int slaveSocketFd) {
	Client_t* pClient = (Client_t*) calloc(1, sizeof(Client_t));
	pClient->m_messageBuffer = (char*)calloc(MESSAGE_PART_LEN, sizeof(char));
	pClient->m_fd = slaveSocketFd;
	pClient->m_bufferBytesUsed = 0;
	return pClient;
}

void release_client(Client_t** pClient) {
	Client_t* ptr = *pClient;
	if (ptr->m_messageBuffer != NULL) {
		free(ptr->m_messageBuffer);
		ptr->m_messageBuffer = NULL;
	}
	ptr->m_fd = -1;
	ptr->m_bufferBytesUsed = 0;
	if (ptr != NULL) {
		free(ptr);
		*pClient = NULL;
	}
}

ClientRec_t* create_client_rec(Client_t* pClient) {
	ClientRec_t* newNode = (ClientRec_t*) calloc(1, sizeof(ClientRec_t));
	newNode->pClient = pClient;
    newNode->next = NULL;
    return newNode;
}

void release_client_rec(ClientRec_t** pClientRec) {
	ClientRec_t* ptr = *pClientRec;
	release_client(&(ptr->pClient));
	ptr->next = NULL;
	if (ptr != NULL) {
		free(ptr);
		*pClientRec = NULL;
	}
}

void schedule_message_cast(Client_t* pClient, ClientRec_t* pClientList, const char* pData, size_t len) {
	size_t bytesFree = MESSAGE_PART_LEN - pClient->m_bufferBytesUsed;
	if (bytesFree >= len) {
		memcpy(pClient->m_messageBuffer + pClient->m_bufferBytesUsed, pData, len);
		pClient->m_bufferBytesUsed += len;
		cast_message(pClient, pClientList);
	}
	else {
		size_t rest = len - bytesFree;
		memcpy(pClient->m_messageBuffer + pClient->m_bufferBytesUsed, pData, bytesFree);
		pClient->m_bufferBytesUsed = MESSAGE_PART_LEN;
		cast_message(pClient, pClientList);
		memcpy(pClient->m_messageBuffer + pClient->m_bufferBytesUsed, pData, rest);
		pClient->m_bufferBytesUsed = rest;
	}
}

void cast_message(Client_t* pClient, ClientRec_t* pClientList) {
	while (pClient->m_bufferBytesUsed > 0) {
		int delimiterPos = -1;
		for (int pos = 0; pos < pClient->m_bufferBytesUsed; pos++) {
			if (pClient->m_messageBuffer[pos] == '\n') {
				delimiterPos = pos;
				break;
			}
		}

		if (delimiterPos == -1) {
			if (pClient->m_bufferBytesUsed == MESSAGE_PART_LEN) {
				broadcast(pClient, pClientList, pClient->m_messageBuffer, MESSAGE_PART_LEN);
			}
			else {
				break;
			}
		}
		else {
			broadcast(pClient, pClientList, pClient->m_messageBuffer, (size_t)(delimiterPos + 1));
			memcpy(pClient->m_messageBuffer,
					pClient->m_messageBuffer + delimiterPos + 1,
					pClient->m_bufferBytesUsed - delimiterPos - 1);
			pClient->m_bufferBytesUsed -= delimiterPos + 1;
		}
	}
}

void broadcast(Client_t* pSender, ClientRec_t* pClientList, const char* pData, size_t len) {
	ClientRec_t* current = pClientList;
	while (current != NULL) {
		int fd = current->pClient->m_fd;
		if (fd != pSender->m_fd) {
			send(fd, pData, len, MSG_NOSIGNAL);
		}
		current = current->next;
	}

	log_message(LOG_FILENAME, pData);
}
