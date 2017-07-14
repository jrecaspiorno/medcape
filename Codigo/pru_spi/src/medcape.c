//NOTA: Ver el comentario de encima de int n = prussdrv_pru_wait_event (PRU_EVTOUT_0);

#include <stdio.h>
#include <stdint.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <time.h>
#include <string.h>
#include <time.h>
#include <prussdrv.h>
#include <pruss_intc_mapping.h>

#include "ads1192.h"

#define ADC_PRU_NUM    0   // using PRU0 for the ADC capture
#define CLK_PRU_NUM    1   // using PRU1 for the sample clock

#define PRU0_DATA_RAM0 AM33XX_DATARAM0_PHYS_BASE
#define PRU0_DATA_RAM1 AM33XX_DATARAM1_PHYS_BASE

#define SPI_DATA_LENG   7

#define BYTES_PER_SAMPLE        8               //Bytes per sample (nSumber+STATUS+CH1+CH2) 16bits each
#define SAMPLES_PER_BANK        (1<<10)         //Complete camples per data bank
#define N_BANKS                 4               //Data banks to be sampled


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
    uint8_t nRegs;
    
    pru1_data0[1]=1;

    *config1=(*config1&0xf8)|(sample_rate&0x7);
    *config2=*config2|0x20;
    
    nRegs=2;
    //nBits
    pru1_data0[2]=(nRegs+2)*8;
    //(Write register|CONFIG1) | (nregs-1) | DATA1 | DATA2
    pru1_data0[3]=  ((WREG|CONFIG1)<<24) | 
                    ((nRegs-1)<<16)      | 
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

void ads_test_signal(uint32_t *pru1_data0, uint8_t *config2){
    uint8_t nRegs;
    
    pru1_data0[1]=2;

    *config2=*config2|0x03;
    nRegs=1;
    //nBits
    pru1_data0[2]=(nRegs+2)*8;
    //(Write register|CONFIG2) | (nregs-1) | DATA1
    pru1_data0[3]=  ((WREG|CONFIG2)<<16) | 
                    ((nRegs-1)<<8)       | 
                    (*config2);
    //Delay
    pru1_data0[4]=8;

    nRegs=2;
    //nBits
    pru1_data0[5]=(nRegs+2)*8;
    //(Write register|CH1SET) | (nregs-1) | DATA1
    pru1_data0[6]=  ((WREG|CH1SET)<<24)   | 
                    ((nRegs-1)<<16)       | 
                    (0x05<<8)             | 
                    (0x05);
    //Delay
    pru1_data0[7]=8;
       
    
    //Send an event to the PRU
    prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);

    //wait till it will be completed
    prussdrv_pru_wait_event(PRU_EVTOUT_1);
    prussdrv_pru_clear_event(PRU_EVTOUT_1,  PRU1_ARM_INTERRUPT  );
}

void end_config_ads(uint32_t *pru1_data0, int spi_period){
    pru1_data0[0]=spi_period;
    pru1_data0[1]=0;
    //Send an event to the PRU
    prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);
}

int main(int argc, char *argv[]) {
    uint32_t *pru1_data0, *pru1_data1;
    int opt, tmp_srate, sample_rate, spi_freq;
    int test_signal=1, i, j;
    uint8_t id, config1, config2;
    uint16_t *pRAM;
    time_t current_time;
    struct tm *time_info;
    char time_string[30];  // space for "YYYY-MM-DD_HH-MM-SS\0"
    FILE *pf_data;

    //Especificamos Sample rate por defecto
    //sample_rate=SRATE_1K;
    sample_rate=SRATE_8K;

    
    if(getuid()!=0){
        printf("You must run this program as root. Exiting.\n");
        exit(EXIT_FAILURE);
    }

    //Parse command-line options
    while((opt = getopt(argc, argv, "tr:h:")) != -1) {
        switch(opt) {
            case 'h':
                printf("%s [-t] [-r SAMPLE_RATE]\n", argv[0]);
                printf("-t:\t\t\ttest signal enabled\n");
                printf("-r SAMPLE_RATE:\t\t125, 250, 500, 1000, 2000, 4000, 8000 [Hz]\n");
                exit(0);
            case 't':
                test_signal=1;
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
            default:
                printf("Error parsing: %c\n", (char)opt);
                exit(-1);
        }
    }

    //Velocidad del bus SPI en función de velocidad de muestreo (para poder leer todas las muestras)
    spi_freq=(sample_rate>SRATE_1K)?FREQ_500kHz:FREQ_100kHz;
    
    time(&current_time);
    time_info=localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%F_%H-%M-%S.dat", time_info);

    if ( (pf_data=fopen(time_string, "w")) == NULL ) {
        perror("Error al fichero de datos");
        exit(EXIT_FAILURE);
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

    // set up the pointer to PRU0 RAM0 bank
    if( prussdrv_map_prumem(PRUSS0_PRU0_DATARAM, (void**)&pru1_data0) == -1 ){
        printf("PRU RAM0 map error.\n");
        exit(EXIT_FAILURE);
    }

    // set up the pointer to PRU0 RAM1 bank
    if( prussdrv_map_prumem(PRUSS0_PRU1_DATARAM, (void**)&pru1_data1) == -1 ){
        printf("PRU RAM1 map error.\n");
        exit(EXIT_FAILURE);
    }

    memset(pru1_data0, 0x00, 8192);
    memset(pru1_data1, 0x00, 8192);
    
    if( prussdrv_exec_program (0, "pru_ads.bin") == -1 ){
        printf("PRU exec error.\n");
        exit(EXIT_FAILURE);
    } 
    sleep(1);
    
    // Especificamos velocidad de transmisión y leemos regtistros de configuración
    if(config_ads(pru1_data0, FREQ_25kHz, &id, &config1, &config2)){
        printf("Error inicializando ADS\n");
        exit(-1);
    }
        
    //Print POR registers
    
    //Set Sample Rate & Enable internal reference
    ads_set_rate_intref(pru1_data0, &config1, &config2, sample_rate);
   
 
    //Set Test signal
    if(test_signal){
        printf("Set test signals\n");
        ads_test_signal(pru1_data0, &config2);
    }

    //Print POR registers
    
    //End config and start auto sample
    end_config_ads(pru1_data0,spi_freq);
    
    //4*2^4*6Bytes
    for(i=0; i<N_BANKS; i++){
        //wait till it will be completed
        prussdrv_pru_wait_event(PRU_EVTOUT_1);
        prussdrv_pru_clear_event(PRU_EVTOUT_1,  PRU1_ARM_INTERRUPT);

        pRAM=(i%2)?(uint16_t*)pru1_data1:(uint16_t*)pru1_data0;
        
        fwrite(pRAM, BYTES_PER_SAMPLE*SAMPLES_PER_BANK, 1, pf_data);
        
        //printf("Data Bank RAM%d:\n", i%2);
        //for(j=0; j<(4*SAMPLES_PER_BANK); j+=4){
        //    printf("sNumber: 0x%04x Status: 0x%04x Ch1: 0x%04x Ch2: 0x%04x\n", pRAM[j], pRAM[j+1], pRAM[j+2], pRAM[j+3]);
        //}
    }
    
    printf("Finishing...\n");

    //Send an event to the PRU
    prussdrv_pru_send_event(ARM_PRU0_INTERRUPT);
   

    // Wait for event completion from PRU
    int n = prussdrv_pru_wait_event (PRU_EVTOUT_0);  // This assumes the PRU generates an interrupt
                                                     // connected to event out 0 immediately before halting
    prussdrv_pru_clear_event (PRU_EVTOUT_0,  PRU0_ARM_INTERRUPT  );
    printf("PRU program completed, event number %d.\n", n);


    // Disable PRU and close memory mappings
    if( prussdrv_pru_disable(0) == -1 ){
        printf("PRU disable error.\n");
        exit(EXIT_FAILURE);
    } 
    
    prussdrv_exit ();
    
    return EXIT_SUCCESS;
}
