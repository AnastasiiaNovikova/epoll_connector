/*
 * Log.c
 *
 *  Created on: Nov 26, 2017
 *      Author: lord
 */

#include "Log.h"

void log_message(const char* log_filename, const char* message) {
	FILE* log_fd = fopen(log_filename, "a");
	if (log_fd != NULL) {
		fputs(message, log_fd);
		fclose(log_fd);
	}
}
