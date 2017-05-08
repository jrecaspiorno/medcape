/*
 * pipe.c
 *
 *  Created on: 10/5/2015
 *      Author: Jose Manuel Bote
 */

#include "pipeFile.h"

#include <fcntl.h>
#include <stdio.h>
#include "boolean.h"


/*
 * Create a pipe to communicate this C program to the Python program that
 * implements the Bluetooth connection.
 * Reading or writing pipe data is atomic if the size of data written is
 * less than PIPE_BUF. In Linux, pipe capacity is 65536 bytes and PIPE_BUF
 * 4096
 */
int createPipe(pipeFile* pipePython) {

	if (stat(pipePython->name, &(pipePython->stat)) != 0) {
		mkfifo(pipePython->name, S_IRWXU);  // Read-write permission, owner
		//fifoFd = open(fifoName, O_WRONLY | O_NONBLOCK);
	} else {
		remove(pipePython->name);
		if (stat(pipePython->name, &(pipePython->stat)) != 0) {
			mkfifo(pipePython->name, S_IRWXU);  // Read-write permission, owner
			//fifoFd = open(fifoName, O_WRONLY | O_NONBLOCK);
		} else {
			return -1;
		}
	}

	return 0;

}
