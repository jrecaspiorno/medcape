/* This mem2file.c program is a modified version of devmem2 by Jan-Derk Bakker
 * as referenced below. This program was modified by Derek Molloy for the book
 * Exploring BeagleBone. It is used in Chapter 13 to dump the DDR External Memory
 * pool to a file. See: www.exploringbeaglebone.com/chapter13/
 *
 * devmem2.c: Simple program to read/write from/to any location in memory.
 *
 *  Copyright (C) 2000, Jan-Derk Bakker (J.D.Bakker@its.tudelft.nl)
 *
 *
 * This software has been developed for the LART computing board
 * (http://www.lart.tudelft.nl/). The development has been sponsored by
 * the Mobile MultiMedia Communications (http://www.mmc.tudelft.nl/)
 * and Ubiquitous Communications (http://www.ubicom.tudelft.nl/)
 * projects.
 *
 * The author can be reached at:
 *
 *  Jan-Derk Bakker
 *  Information and Communication Theory Group
 *  Faculty of Information Technology and Systems
 *  Delft University of Technology
 *  P.O. Box 5031
 *  2600 GA Delft
 *  The Netherlands
 *
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the Free Software
 * Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
 */
#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <fcntl.h>
#include <ctype.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/mman.h>
#include <fcntl.h>
#include <limits.h>

#include "mem2file.h"
#include "pipeFile.h"

#define MAP_SIZE 0x0FFFFFFF
#define MAP_MASK (MAP_SIZE - 1)
#define MMAP_LOC   "/sys/class/uio/uio0/maps/map1/"

int fd;
void *map_base, *virt_addr;
unsigned long read_result, writeval;
unsigned int addr;
unsigned int dataSize;
unsigned int size_packet;
unsigned int number_total_samples;
unsigned int samples_per_chunk;
off_t target;
char filename[] = "/home/debian/workspace2016/pru_spi_ads/ztest.data";
FILE* fd_output;

// Pipe to Python program (Bluetooth module)
pipeFile pipePython = {
		.name = "pipe_medcape.data",
		//.stat,
		.fid = -1
};

int mem2file_initialize(){
    read_result, writeval;
    addr = readFileValue(MMAP_LOC "addr");
    dataSize = readFileValue(MMAP_LOC "size");
    number_total_samples = dataSize  / 18;
    samples_per_chunk = 100;
    target = addr;
	size_packet = 2*8+2;
	if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1){
		printf("Failed to open memory!");
		return -1;
    }

	//We establish the target like if we were in the last chunk(hence we cover all the data available to map)
	target = addr+((size_packet)*samples_per_chunk); //6*4=1chunk, we have 100 samples for each chunk

    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) {
       printf("Failed to map base address");
       return -1;
    }
	
		
	if ( (fd_output = fopen(filename, "w+b")) == NULL ) {
		perror("Error al abrir el fichero de datos");
	}
		
	
	// Configure pruIo driver, the nonin sensor and the pipe to Python
	printf("Configuring modules...\n");
	if (configureModules(&pipePython)) {
		exit(EXIT_FAILURE);
	}

	
   printf("mem2file succesfully initialized!! \n\n");

	return 0;
}

unsigned int readFileValue(char filename[]){
   FILE* fp;
   unsigned int value = 0;
   fp = fopen(filename, "rt");
   fscanf(fp, "%x", &value);
   fclose(fp);
   return value;
}

int mem2file_main(int number_chunk, int samples_taken) {

    target = addr;
	/*
	printf("\n Number_chunk: %d \n", number_chunk);
		
	int samples_left = number_total_samples - samples_taken;
	printf("\n \n Samples left: %d", samples_left);
	printf("\n  number_total_samples: %d", number_total_samples);
	printf("\n \n samples_taken %d \n \n", samples_taken);
	*/
	
	if(number_chunk==2){
		target = addr+((size_packet)*samples_per_chunk); //size_packet=1chunk (2 bytes per channel, and 3 status bytes (but we only take 2 to have a pair number), we have 100 samples for each chunk
	}


	
	if ( (fd_output = freopen(filename, "w+b", fd_output)) == NULL ) {
		perror("Error al abrir el fichero de datos");
	}
	

	target=(number_chunk==1)?addr:addr+((size_packet)*samples_per_chunk);
	virt_addr = map_base + (target & MAP_MASK);
	fwrite(virt_addr, size_packet, samples_per_chunk, fd_output);
	
	// Try to open the Python pipe
	if (pipePython.fid < 1) {
		pipePython.fid = open(pipePython.name, O_WRONLY | O_NONBLOCK);
		//printf("Pipe open\n");
	}

	// Send data if the Python pipe is open
	if (pipePython.fid > 0) {
		int writeSuccess = write(pipePython.fid, virt_addr, size_packet*samples_per_chunk);
		if (writeSuccess < 0) {
			close(pipePython.fid);
			pipePython.fid = -1;
		} else if (writeSuccess > PIPE_BUF) {
			printf("Warning!! Write in FIFO is not atomic\n");
		}
	}
	
    return 0;
}

int configureModules(pipeFile* pipePython) {
	// Pipe Python for Bluetooth
	printf("\tStarting pipe... ");
	if (createPipe(pipePython)) {
		printf("Failed\n");
	} else {
		printf("OK\n");
	}
	return 0;
}
