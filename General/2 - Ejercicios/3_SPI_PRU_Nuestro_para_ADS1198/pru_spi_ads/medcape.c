#include <stdio.h>
#include <pthread.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>

#include "bbb_gpio.h"
#include "bbb_spi.h"
#include "ads.h"

#define COMPILER_BARRIER() asm volatile("" ::: "memory")

//-----------Exploring BBB part-------------------------------------
#include <stdio.h>
#include <stdlib.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define ADC_PRU_NUM	   0   // using PRU0 for the ADC capture
#define CLK_PRU_NUM	   1   // using PRU1 for the sample clock
#define MMAP0_LOC   "/sys/class/uio/uio0/maps/map0/"
#define MMAP1_LOC   "/sys/class/uio/uio0/maps/map1/"

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

enum CONTROL {
	PAUSED = 0,
	RUNNING = 1,
	UPDATE = 3
};

// Short function to load a single unsigned int from a sysfs entry
unsigned int readFileValue_sysfs(char filename[]){
   FILE* fp;
   unsigned int value = 0;
   fp = fopen(filename, "rt");
   fscanf(fp, "%x", &value);
   fclose(fp);
   return value;
}
//----------------------------------------------------------------------

volatile uint32_t ints=0;

static void *load_mem2file_thread(void *arg) {
    printf("load_mem2file_thread Thread\n");

	while(1) {
		//mem2file();
	}
	
}


unsigned long int get_gpio_ints() {
    int line_num;
    unsigned long ints=-1;
    char line[256];
    char tmp_char[256];
    FILE *pf_int;

    if ( (pf_int=fopen("/proc/interrupts", "r"))==NULL ) {
        perror("Error abriendo fichero de interrupciones");
        return -1;
    }

    while (fgets(line, sizeof(line), pf_int)) {
        /* note that fgets don't strip the terminating \n, checking its
         presen*ce would allow to handle lines longer that sizeof(line) */
        printf("%s", line);
        if (line_num==177) {
            sscanf(line, "%d: %lu, %s", &line_num, &ints, tmp_char);
            printf("Ints: %lu\n", ints);
        }

    }
    /* may check feof here to make a difference between eof and io failure -- network
     timeout fo*r instance */

    fclose(pf_int);

    return ints;
}


int main() {
    int i;
    pthread_t thid;

	if(getuid()!=0){
      printf("You must run this program as root. Exiting.\n");
      exit(EXIT_FAILURE);
   }
   
/* //Descomentar cuando se tenga hecho el dts que también los configure
    printf("Init GPIOs\n");
    if ( ads_init_gpios() == -1 ) {
        printf("can't init GPIOs\n");
        return -1;
    }
	
    printf("Reset cycle\n");
    if ( ads_reset() == -1 ) {
        printf("can't reset ADS\n");
        return -1;
    }
	
    printf("START: start continuous reading\n");
    if ( ads_start() == -1 ) {
        printf("can't start conversion\n");
        return -1;
    }
	
*/

   // Initialize structure used by prussdrv_pruintc_intc
   // PRUSS_INTC_INITDATA is found in pruss_intc_mapping.h
   tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

   // Read in the location and address of the shared memory. This value changes
   // each time a new block of memory is allocated.
   unsigned int timerData[2];
   timerData[0] = FREQ_1kHz;
   timerData[1] = RUNNING;
   printf("The PRU clock state is set as period: %d (0x%x) and state: %d\n", timerData[0], timerData[0], timerData[1]);
   unsigned int PRU_data_addr = readFileValue_sysfs(MMAP0_LOC "addr");
   printf("-> the PRUClock memory is mapped at the base address: %x\n", (PRU_data_addr + 0x2000));
   printf("-> the PRUClock on/off state is mapped at address: %x\n", (PRU_data_addr + 0x10000));

   // data for PRU0 based on the MCPXXXX datasheet
   unsigned int spiData[6];
   	//May we store the initial spi speed in shared memory:
	//(ADS_SPI_HZ>>5)
    //printf("initial spi speed in memory \n");

	spiData[0] = SDATAC; 
    printf("Loading ads_sdatac command\n");
	spiData[1] = RDATAC; 
    printf("Loading ads_rdatac command\n");
	spiData[2] = SRATE_1K; 
	printf("Loading final spi speed\n");
	#ifdef ADS1198
		spiData[3] = CONFIG3; 
	#endif
	#ifdef ADS1192
		spiData[3] = CONFIG2; 
	#endif
    printf("Loading Enable internal reference command \n");
	//May we check ids...?
   spiData[4] = readFileValue_sysfs(MMAP1_LOC "addr");
   spiData[5] = readFileValue_sysfs(MMAP1_LOC "size");
   
   
   printf("Sending the SPI Control Data: 0x%x\n", spiData[0]);
   printf("The DDR External Memory pool has location: 0x%x and size: 0x%x bytes\n", spiData[4], spiData[5]);
   int numberSamples = spiData[5];
   printf("-> this space has capacity to store %d 8-bit samples (max)\n", numberSamples);

   // Allocate and initialize memory
   prussdrv_init ();
   prussdrv_open (PRU_EVTOUT_0);

   // Write the spiData into PRU0 Data RAM0.
   prussdrv_pru_write_memory(PRUSS0_PRU0_DATARAM, 0, spiData, 24);  // spi data
   printf("spi_data stored in pru0 memory\n");
   prussdrv_pru_write_memory(PRUSS0_PRU1_DATARAM, 0, timerData, 8); // sample clock
   printf("clock_data stored in pru1 memory\n");

   
   // Map the PRU's interrupts
   prussdrv_pruintc_init(&pruss_intc_initdata);

    //NOTE no muevas esto de aquí, siempre se genera una interrupción inicial
    //y podemos estar jodiendo la comunicación SPI!!!
    system("cat /proc/interrupts | grep 177 | echo Initial interrupts: $(awk '{print $2}')");
   
	//Pru0 must loop polling the value of data_ready (until data_ready=0 i.e. falling, i.e. waiting for edge to get data)
	//We don't need to count the interruptions in pru(because we shouldn't have any)	
	// Load and execute the PRU program on the PRU
	printf("Main sleep... executing pru programs... (%d)\n", timerData[0]);

	//prussdrv_exec_program (ADC_PRU_NUM, "./PRUADC.bin");
	prussdrv_exec_program (CLK_PRU_NUM, "./PRUClock.bin");

	/*
    //Create thread
    if (pthread_create(&thid, NULL, &load_mem2file_thread, NULL)) {
        perror("Failed to create the toggle thread");
        return -1;
    }
  
  
   // Wait for event completion from PRU, returns the PRU_EVTOUT_0 number
   int n = prussdrv_pru_wait_event (PRU_EVTOUT_0);
   printf("EBBADC PRU0 program completed, event number %d.\n", n);
   
	*/
	// Disable PRU and close memory mappings 
   prussdrv_pru_disable(ADC_PRU_NUM);
   prussdrv_pru_disable(CLK_PRU_NUM);
   
   prussdrv_exit ();
   
	//execute sdatac command(this is already done in pru0)

    printf("Ints: %u\n", ints);
    system("cat /proc/interrupts | grep 177 | echo final interrupts: $(awk '{print $2}')");
	
   return 0;
}
