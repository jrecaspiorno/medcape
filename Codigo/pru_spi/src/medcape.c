//NOTA: Ver el comentario de encima de int n = prussdrv_pru_wait_event (PRU_EVTOUT_0);

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#define ADC_PRU_NUM    0   // using PRU0 for the ADC capture
#define CLK_PRU_NUM    1   // using PRU1 for the sample clock

#define PRU0_DATA_RAM0 AM33XX_DATARAM0_PHYS_BASE
#define PRU0_DATA_RAM1 AM33XX_DATARAM1_PHYS_BASE

#define SPI_DATA_LENG   7


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
/*
enum CONTROL {
	PAUSED = 0,
	RUNNING = 1,
	UPDATE = 3
};
*/

static uint32_t *pru1_data0;

int main() {
    /*pthread_t thid;
    uint32_t *p;
    int i;
    */
    if(getuid()!=0){
        printf("You must run this program as root. Exiting.\n");
        exit(EXIT_FAILURE);
    }
    // Initialize structure used by prussdrv_pruintc_intc
    // PRUSS_INTC_INITDATA is found in pruss_intc_mapping.h
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    // Allocate and initialize memory
    if( prussdrv_init() == -1 || 
        prussdrv_open(PRU_EVTOUT_0) == -1 ||
        prussdrv_pruintc_init(&pruss_intc_initdata) == -1 ){
        printf("PRU Init error.\n");
        exit(EXIT_FAILURE);
    }

    // set up the pointer to PRU memory
    if( prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void**)&pru1_data0) == -1 ){
        printf("PRU map error.\n");
        exit(EXIT_FAILURE);
    }
    
    pru1_data0[0]=0x01234567;
    pru1_data0[1]=0x89abcdef;
    pru1_data0[2]=0x0;
    pru1_data0[3]=0x0;

    printf("pru1_data0: %#010x - %#010x - %#010x - %#010x\n", pru1_data0[0], pru1_data0[1], pru1_data0[2], pru1_data0[3]);
    
    if( prussdrv_exec_program (0, "pru_ads.bin") == -1 ){
        printf("PRU exec error.\n");
        exit(EXIT_FAILURE);
    } 

    // Wait for event completion from PRU
    int n = prussdrv_pru_wait_event (PRU_EVTOUT_0);  // This assumes the PRU generates an interrupt
                                                        // connected to event out 0 immediately before halting
    printf("PRU program completed, event number %d.\n", n);

    printf("pru1_data0: %#010x - %#010x - %#010x - %#010x\n", pru1_data0[0], pru1_data0[1], pru1_data0[2], pru1_data0[3]);

    // Disable PRU and close memory mappings
    if( prussdrv_pru_disable(0) == -1 ){
        printf("PRU disable error.\n");
        exit(EXIT_FAILURE);
    } 
    
    prussdrv_exit ();
    return EXIT_SUCCESS;
}