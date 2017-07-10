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

#include "ads1192.h"

#define ADC_PRU_NUM    0   // using PRU0 for the ADC capture
#define CLK_PRU_NUM    1   // using PRU1 for the sample clock

#define PRU0_DATA_RAM0 AM33XX_DATARAM0_PHYS_BASE
#define PRU0_DATA_RAM1 AM33XX_DATARAM1_PHYS_BASE

#define SPI_DATA_LENG   7

// Freq=1/( (2xCLK_LEVEL_DELAY) × (2×5×10^(−9)) )
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

static uint32_t *pru1_data0;




int config_ads(uint32_t *pru1_data0, int spi_period, uint8_t *id, uint8_t *config1, uint8_t *config2){

    int i, j, id_index;
    
    i=0;
    pru1_data0[i++]=spi_period;                         // Freq=1/( (2*spi_period) * (2*5*10^(−9)) )
    pru1_data0[i++]=0;                                  // Msgs
    
    //Stop Read
    pru1_data0[1]++;
    //Mensaje de 16b    -> Stop Read Continuous      //Delay con el siguiente msg=2^8*CLK_LEVEL_DELAY
    pru1_data0[i++]=8;     pru1_data0[i++]=SDATAC;   pru1_data0[i++]=6; 

    /// Read config and ID registres (2msgs)
    pru1_data0[1]+=2;
    //16b               -> (Read register|ID) | (nregs-1)                Sin delay con siguiente msg
    pru1_data0[i++]=16;    pru1_data0[i++]=((RREG|ID)<<8) | ((3-1)<<0);  pru1_data0[i++]=0;                                  
    //24b               -> Data: ID, CONFIG1, CONFIG2                    Delay=1024*CLK_LEVEL_DELAY
    pru1_data0[i++]=24;    pru1_data0[i++]=0;                            pru1_data0[i++]=6;

    //ID, CONFIG1, CONFIG2 index
    id_index=i-2;
    
    printf("Sample rate: %d\nnMsgs: %d\n", pru1_data0[0], pru1_data0[1]);
    for(j=2; j<i; j+=3)
        printf("%#010x - %#010x - %#010x\n", pru1_data0[j], pru1_data0[j+1], pru1_data0[j+2]);

    //Send an event to the PRU
    prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);
    //wait till it will be completed
    prussdrv_pru_wait_event(PRU_EVTOUT_1);
    prussdrv_pru_clear_event(PRU_EVTOUT_1,  PRU1_ARM_INTERRUPT  );

    printf("Reply:\n");
    for(j=2; j<i; j+=3)
        printf("%#010x - %#010x - %#010x\n", pru1_data0[j], pru1_data0[j+1], pru1_data0[j+2]);

    *config2=pru1_data0[id_index]&0xff;
    *config1=(pru1_data0[id_index]>>8)&0xff;
    *id=(pru1_data0[id_index]>>16)&0xff;
    printf("Id: %#04x - Config1: %#04x - Config2: %#04x\n", *id, *config1, *config2);
    
    return (*id==ID_VALUE)?0:-1;
}


void ads_set_rate_intref(uint32_t *pru1_data0, uint8_t *config1, uint8_t *config2, uint8_t sample_rate){

    pru1_data0[1]=1;

    *config1=(*config1&0xf8)|(sample_rate&0x7);
    *config2=*config2|0x20;
    //nBits
    pru1_data0[2]=4*8;
    //(Write register|CONFIG1) | (nregs-1) | DATA1 | DATA2
    pru1_data0[3]=  ((WREG|CONFIG1)<<24) | 
                    ((1-1)<<16)          | 
                    (*config1<<8)        |
                    (*config2);
    //Delay
    pru1_data0[4]=8;
    
    //Send an event to the PRU
    prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);

    //wait till it will be completed
    prussdrv_pru_wait_event(PRU_EVTOUT_1);
    prussdrv_pru_clear_event(PRU_EVTOUT_1,  PRU1_ARM_INTERRUPT  );
    
}

void end_config_ads(uint32_t *pru1_data0){
    pru1_data0[0]=FREQ_100kHz;
    pru1_data0[1]=0;
    //Send an event to the PRU
    prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);
}


    


int main(int argc, char *argv[]) {
    int opt, spi_period=FREQ_25kHz, tmp_srate=1000, sample_rate=FREQ_1kHz;
    int secs=20, test_signal=0, i;
    uint8_t id, config1, config2;
    uint16_t *pRAM;

    if(getuid()!=0){
        printf("You must run this program as root. Exiting.\n");
        exit(EXIT_FAILURE);
    }

    //Parse command-line options
    while((opt = getopt(argc, argv, "ts:r:hl:")) != -1) {
        switch(opt) {
            case 'h':
                printf("%s [-t] [-s SPI_PERIOD] [-r SAMPLE_RATE]\n", argv[0]);
                printf("-t:\t\t\ttest signal enabled\n");
                printf("-s SPI_PERIOD in 20ns:\t\t Freq = 1/(SPI_PERIOD*20ns)\n");
                printf("-r SAMPLE_RATE:\t\t125, 250, 500, 1000, 2000, 4000, 8000 [Hz]\n");
                exit(0);
            case 't':
                test_signal=1;
                break;
            case 's':
                spi_period=atoi(optarg);
                if(spi_period>FREQ_500kHz || spi_period<FREQ_12_5MHz){
                    printf("SPI freq. not supported\n");
                    exit(-1);
                }
                break;
            case 'r':
                tmp_srate=atoi(optarg);
                switch(tmp_srate) {
                    case 125:
                        sample_rate=SRATE_125; break;
                    case 250:
                        sample_rate=SRATE_250; break;
                    case 500:
                        sample_rate=SRATE_500; break;
                    case 1000:
                        sample_rate=SRATE_1K; break;
                    case 2000:
                        sample_rate=SRATE_2K; break;
                    case 4000:
                        sample_rate=SRATE_4K; break;
                    case 8000:
                        sample_rate=SRATE_8K; break;
                    default:
                    printf("Sample Rate not supported\n");
                    exit(-1);
                }
                break;
                
            case 'l':
                secs=atoi(optarg);
                break;
            default:
                printf("Error parsing: %c\n", (char)opt);
                exit(-1);
        }
    }
    
    // Initialize structure used by prussdrv_pruintc_intc
    // PRUSS_INTC_INITDATA is found in pruss_intc_mapping.h
    tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;

    // Allocate and initialize memory
    if( prussdrv_init() == -1 || 
        prussdrv_open(PRU_EVTOUT_0) == -1 ||
        prussdrv_open(PRU_EVTOUT_1) == -1 ||
        prussdrv_pruintc_init(&pruss_intc_initdata) == -1 ){
        printf("PRU Init error.\n");
        exit(EXIT_FAILURE);
    }

    // set up the pointer to PRU memory
    if( prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void**)&pru1_data0) == -1 ){
        printf("PRU map error.\n");
        exit(EXIT_FAILURE);
    }

    if( prussdrv_exec_program (0, "pru_ads.bin") == -1 ){
        printf("PRU exec error.\n");
        exit(EXIT_FAILURE);
    } 
    sleep(1);
    
    // Especificamos velocidad de transmisión y leemos regtistros de configuración
    if(config_ads(pru1_data0, spi_period, &id, &config1, &config2)){
        printf("Error inicializando ADS\n");
        exit(-1);
    }
        
    //Print POR registers
    
    //Set Sample Rate & Enable internal reference
    ads_set_rate_intref(pru1_data0, &config1, &config2, sample_rate);
   
 
    //Set Test signal
    //Print POR registers
    
    //End config and start auto sample
    end_config_ads(pru1_data0);
    
    printf("Finishing...\n");
    // Wait for event completion from PRU
    int n = prussdrv_pru_wait_event (PRU_EVTOUT_0);  // This assumes the PRU generates an interrupt
                                                     // connected to event out 0 immediately before halting
    prussdrv_pru_clear_event (PRU_EVTOUT_0,  PRU0_ARM_INTERRUPT  );
    printf("PRU program completed, event number %d.\n", n);

    pRAM=(uint16_t*)pru1_data0;
    printf("Data:\n");
    for(i=0; i<10*3; i+=3){
        printf("Status: 0x%04x Ch1: 0x%04x Ch2: 0x%04x\n", pRAM[i], pRAM[i+1], pRAM[i+2]);
    }

    // Disable PRU and close memory mappings
    if( prussdrv_pru_disable(0) == -1 ){
        printf("PRU disable error.\n");
        exit(EXIT_FAILURE);
    } 
    
    prussdrv_exit ();
    
    return EXIT_SUCCESS;
}
