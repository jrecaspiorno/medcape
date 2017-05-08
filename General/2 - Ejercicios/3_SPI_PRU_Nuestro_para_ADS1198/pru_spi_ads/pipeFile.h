/*
 * pipe.h
 *
 *  Created on: 10/5/2015
 *      Author: Jose Manuel Bote
 */

#ifndef _PIPE_
#define _PIPE_

#include <sys/stat.h>

typedef struct {
	char* name;
	struct stat stat;
	int fid;
} pipeFile;


/*
 * Create a pipe to communicate this C program to the Python program that
 * implements the Bluetooth connection.
 * Reading or writing pipe data is atomic if the size of data written is
 * less than PIPE_BUF. In Linux, pipe capacity is 65536 bytes and PIPE_BUF
 * 4096
 */
int createPipe(pipeFile* pipePython);


#endif
