/*
 * Constants.h
 *
 *  Created on: Nov 25, 2017
 *      Author: lord
 */

#ifndef CONSTANTS_H_
#define CONSTANTS_H_

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <fcntl.h>
#include <poll.h>

#include <sys/epoll.h>

#include <errno.h>
#include <string.h>
#include <memory.h>

#define MESSAGE_PART_LEN 1024
#define MAX_EVENTS 128
#define LOG_FILENAME "EpollConnector.log"


#endif /* CONSTANTS_H_ */
