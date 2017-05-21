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
#include <sys/stat.h>
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
#include <prussdrv.h>  
#include <pruss_intc_mapping.h>  
#include <fcntl.h>
#include <limits.h>

#include "pipeFile.h"
#include "bbb_gpio.h"
#include "bbb_spi.h"
#include "ads.h"
#include "mem2file.h"

#define MAP_SIZE 0x0FFFFFFF
#define MAP_MASK (MAP_SIZE - 1)
#define MMAP_LOC   "/sys/class/uio/uio0/maps/map1/"

#define ADC_PRU_NUM	   0   // using PRU0 for the ADC capture
#define CLK_PRU_NUM	   1   // using PRU1 for the sample clock
#define MMAP0_LOC   "/sys/class/uio/uio0/maps/map0/"
#define MMAP1_LOC   "/sys/class/uio/uio0/maps/map1/"


#define PRU0_DATA_RAM0 AM33XX_DATARAM0_PHYS_BASE
#define PRU0_DATA_RAM1 AM33XX_DATARAM1_PHYS_BASE

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
struct stat st;

volatile void *pru1_data0 = NULL;  
unsigned int physical_address;

enum FREQUENCY {    // measured and calibrated, but can be calculated
	FREQ_12_5MHz =  1,
	FREQ_6_25MHz =  5,
	FREQ_5MHz    =  7,
	FREQ_3_85MHz = 10,
	FREQ_1MHz   =  45,
	FREQ_500kHz =  95,
	FREQ_250kHz = 245,
	FREQ_100kHz = 495,
	FREQ_25kHz = 1995,
	FREQ_10kHz = 4995,
	FREQ_5kHz =  9995,
	FREQ_2kHz = 24995,
	FREQ_1kHz = 49995
};
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
	/*
	if((fd = open("/dev/mem", O_RDWR | O_SYNC)) == -1){
		printf("Failed to open memory!");
		return -1;
    }

	//We establish the target like if we were in the last chunk(hence we cover all the data available to map)
	target = addr;//+((size_packet)*samples_per_chunk); //6*4=1chunk, we have 100 samples for each chunk

    map_base = mmap(0, MAP_SIZE, PROT_READ | PROT_WRITE, MAP_SHARED, fd, target & ~MAP_MASK);
    if(map_base == (void *) -1) {
       printf("Failed to map base address");
       return -1;
    }
	*/
		
	if ( (fd_output = fopen(filename, "w+b")) == NULL ) {
		perror("Error al abrir el fichero de datos");
	}
		
	
	// Configure pruIo driver, the nonin sensor and the pipe to Python
	printf("Configuring modules...\n");
	if (configureModules(&pipePython)) {
		exit(EXIT_FAILURE);
	}
	
	//------------------------------------------------------------------
    // set up the pointer to PRU memory
    if( prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void**)&pru1_data0) == -1 ){
        printf("PRU map error.\n");
        exit(EXIT_FAILURE);
    }
   	//------------------------------------------------------------------
	
	//------------------------------------------------------------------
	 unsigned int spiData[7];
   	//May we store the initial spi speed in shared memory:
	//(ADS_SPI_HZ>>5)
    //printf("initial spi speed in memory \n");

	spiData[0] = SDATAC; 
    printf("Loading ads_sdatac command\n");
	spiData[1] = RDATAC; 
    printf("Loading ads_rdatac command\n");
	spiData[2] = SRATE_1K; 
	printf("Loading sample rate speed\n");
	#ifdef ADS1198
		spiData[3] = CONFIG3; 
	#endif
	#ifdef ADS1192
		spiData[3] = CONFIG2; 
	#endif
    printf("Loading Enable internal reference command \n");
	//May we check ids...?
   spiData[4] = physical_address;
   spiData[5] = dataSize;
   printf("Loading spi speed\n");
   //La velocidad del spi tiene que ser 500 veces la velocidad del sample rate,para que la lectura de los 18 bytes quepan 
   //en 1 ciclo de data_ready
   /*
   Ejemplos de combinaciones posibles:
   Data_ready: 500Hz
   SPI: 100kHz
   
   Data_ready: 8KHz
   SPI: 5MHz
   */
   //spiData[6] = FREQ_100kHz; 
   spiData[6] = FREQ_250kHz; 

   int numberSamples = spiData[5];
   printf("The DDR External Memory pool has location: 0x%x and size: 0x%x bytes = %d bytes\n", spiData[4], spiData[5], numberSamples);
   printf("-> this space has capacity to store %d 18-bytes samples (max)\n", numberSamples/N_DATA);

   // Write the spiData into PRU0 Data RAM0.
   prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, spiData, 28);  // spi data
   printf("spi_control_data stored in pru0 memory\n");
	//------------------------------------------------------------------

	


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
	int i = 0;
	/*
	for (i = 0; i < 10; i++) {  
	   // See if it's successfully writing the physical address of each word at  
	   // the (virtual, from our viewpoint) address  
	   printf("DDR[%d] is: %p / 0x%x\n", i, ((unsigned int *)shared_ddr) + i,   
		   ((unsigned int *) shared_ddr)[i]);  

	}  
	*/
	
	
	
	stat(filename, &st);
	int size_file = st.st_size;
	if (size_file >= (size_packet*samples_per_chunk)*50000) {
		if ( (fd_output = freopen(filename, "w+b", fd_output)) == NULL ) {
			perror("Error al abrir el fichero de datos");
		}
	}
	
	
	while (((uint8_t *)pru1_data0)[(size_packet*samples_per_chunk)] != 0xFF) {
		printf("Waiting for mem shared to refresh \n");
		fflush(stdout);
	   printf("Mem flag incorrect [%d] is: %x \n", (size_packet*samples_per_chunk), ((uint8_t *)pru1_data0)[(size_packet*samples_per_chunk)]);  
		fflush(stdout);
		printf("\n");
	}
   printf("Mem flag correct [%d] is: %x \n", (size_packet*samples_per_chunk), ((uint8_t *)pru1_data0)[(size_packet*samples_per_chunk)]);  

	fwrite(pru1_data0, size_packet, samples_per_chunk, fd_output);
	((uint8_t *)pru1_data0)[(size_packet*samples_per_chunk)] = 0x00;
	

	// Try to open the Python pipe
	if (pipePython.fid < 1) {
		pipePython.fid = open(pipePython.name, O_WRONLY | O_NONBLOCK);
		//printf("Pipe open\n");
	}

	// Send data if the Python pipe is open
	if (pipePython.fid > 0) {
		int writeSuccess = write(pipePython.fid, pru1_data0, size_packet*samples_per_chunk);
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
