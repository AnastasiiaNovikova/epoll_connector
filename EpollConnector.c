/*
 ============================================================================
 Name        : EpollConnector.c
 Authors     : Roman Vasilyev, Anastasia Novikova
 Version     : 1.0
 Copyright   : Created for Linux API @Mail.ru
 Description : Simple chat in C featuring epoll
 ============================================================================
 */

#include <stdio.h>
#include <stdlib.h>
#include "Client.h"
#include "Log.h"

#define PORT 2015
const char* GREETING = "Welcome to hell!\n";
const size_t GREEINTG_LEN = 17;

ClientRec_t* m_Clients;

int set_nonblock(int fd);
Client_t* search_for_client_with_fd(ClientRec_t* pClientList, int fd);
void add_element(ClientRec_t** pClientList, Client_t* pClient);
void remove_element_with_fd(ClientRec_t** pClientList, int fd);

int main(void) {
	printf("Server started successfully at port %d", PORT);
	m_Clients = NULL;
	int MasterSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

	if (MasterSocket == -1)
	{
		puts(strerror(errno));
		return 1;
	}

	int enable = 1;
	if (setsockopt(MasterSocket, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(int)) < 0) {
		perror("setsockopt(SO_REUSEADDR) failed");
		exit(1);
	}

	struct sockaddr_in SockAddr;
	SockAddr.sin_family = AF_INET;
	SockAddr.sin_port = htons(PORT);
	SockAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int Result = bind(MasterSocket, (struct sockaddr *)&SockAddr, sizeof(SockAddr));

	if (Result == -1)
	{
		puts(strerror(errno));
		return 1;
	}

	set_nonblock(MasterSocket);

	Result = listen(MasterSocket, SOMAXCONN);

	if (Result == -1)
	{
		puts(strerror(errno));
		return 1;
	}

	struct epoll_event Event;
	Event.data.fd = MasterSocket;
	Event.events = EPOLLIN;

	struct epoll_event * Events;
	Events = (struct epoll_event *) calloc(MAX_EVENTS, sizeof(struct epoll_event));

	int EPoll = epoll_create1(0);
	epoll_ctl(EPoll, EPOLL_CTL_ADD, MasterSocket, &Event);

	while (1)
	{
		size_t N = (size_t)epoll_wait(EPoll, Events, MAX_EVENTS, -1);
		for (size_t i = 0; i < N; i++)
		{
			if((Events[i].events & EPOLLERR)||(Events[i].events & EPOLLHUP))
			{
				if (Events[i].events & EPOLLHUP) {
					puts("connection terminated by client\n");
					log_message(LOG_FILENAME, "connection terminated by client\n");
				}
				shutdown(Events[i].data.fd, SHUT_RDWR);
				close(Events[i].data.fd);
				epoll_ctl(EPoll, EPOLL_CTL_DEL, Events[i].data.fd, &Event);
				remove_element_with_fd(&m_Clients, Events[i].data.fd);
			}
			else if (Events[i].data.fd == MasterSocket)
			{
				int SlaveSocket = accept(MasterSocket, 0, 0);
				set_nonblock(SlaveSocket);

				struct epoll_event Event;
				Event.data.fd = SlaveSocket;
				Event.events = EPOLLIN;

				epoll_ctl(EPoll, EPOLL_CTL_ADD, SlaveSocket, &Event);
				Client_t* client_ptr = create_client(SlaveSocket);
				add_element(&m_Clients, client_ptr);
				log_message(LOG_FILENAME, "accepted connection\n");
				puts("accepted connection");
				send(SlaveSocket, GREETING, GREEINTG_LEN, MSG_NOSIGNAL);
			}
			else
			{
				Client_t* cl_ptr = search_for_client_with_fd(m_Clients, Events[i].data.fd);
				static char Buffer[MESSAGE_PART_LEN];
				int RecvSize = recv(Events[i].data.fd, Buffer, MESSAGE_PART_LEN, MSG_NOSIGNAL);
				if (RecvSize <= 0) {
					epoll_ctl(EPoll, EPOLL_CTL_DEL, Events[i].data.fd, &Event);
					remove_element_with_fd(&m_Clients, Events[i].data.fd);
					puts("connection closed");
					log_message(LOG_FILENAME, "connection terminated\n");
				}
				else {
					schedule_message_cast(cl_ptr, m_Clients, Buffer, RecvSize);
				}
			}
		}
	}
	return EXIT_SUCCESS;
}

int set_nonblock(int fd)
{
	int flags;
#if defined(O_NONBLOCK)
	if (-1 == (flags = fcntl(fd, F_GETFL, 0)))
		flags = 0;
	return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
#else
	flags = 1;
	return ioctl(fd, FIOBIO, &flags);
#endif
}

Client_t* search_for_client_with_fd(ClientRec_t* pClientList, int fd) {
	ClientRec_t* current = pClientList;
	while (current != NULL) {
		if (current->pClient->m_fd == fd) {
			return current->pClient;
		}
		current = current->next;
	}
	return NULL;
}

void add_element(ClientRec_t** pClientList, Client_t* pClient) {
	ClientRec_t* newNode = create_client_rec(pClient);
    newNode->next = *pClientList;
    *pClientList = newNode;
}

void remove_element_with_fd(ClientRec_t** pClientList, int fd) {
	ClientRec_t* first = *pClientList;
	if (first->pClient->m_fd == fd) {
		// First element
		ClientRec_t* new_first = first->next;
		release_client_rec(&first);
		*pClientList = new_first;
	}
	else {
		ClientRec_t* current = first;
		int deletion_flag = 0;
		while (current->next != NULL) {
			// Middle element
			if (current->next->pClient->m_fd == fd) {
				ClientRec_t* to_delete = current->next;
				ClientRec_t* next_elem = current->next->next;
				release_client_rec(&to_delete);
				current->next = next_elem;
				deletion_flag = 1;
				break;
			}
			current = current->next;
		}
		if (deletion_flag == 0) {
			if (current->pClient->m_fd == fd) {
				// Last element, next of previous is set to NULL automatically
				release_client_rec(&current);
			}
			else {
				puts("Error! No client with specified FD");
				log_message(LOG_FILENAME, "Error! No client with specified FD");
			}
		}
	}
}
