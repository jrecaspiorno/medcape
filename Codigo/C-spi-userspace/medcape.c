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

volatile uint32_t ints=0;

static void *thread_spidata(void *arg) {
    uint8_t data[N_DATA];
    struct gpio_edge st_edge;
    //int nbuff=0, nsample=0;
    time_t current_time;
    struct tm *time_info;
    char time_string[30];  // space for "YYYY-MM-DD_HH-MM-SS\0"
    FILE *pf_data;


    printf("SPI Thread\n");

    time(&current_time);
    time_info=localtime(&current_time);
    strftime(time_string, sizeof(time_string), "%F_%H-%M-%S.dat", time_info);


    if ( (pf_data=fopen(time_string, "w")) == NULL ) {
        perror("Error al fichero de datos");
        pthread_exit(NULL);
    }

    //Init GPIO, must be an input pin to poll its value
    if ( set_gpio_edge(GPIO_DATA_READY, "falling", &st_edge) ) {
        pthread_exit(NULL);
    }

    printf("SPI Thread waiting...\n");
    while (1) {
        wait_for_edge(&st_edge);

        if (ads_read_sample(data) == -1) {
            printf("can't read IDReg\n");
            pthread_exit(NULL);
        }

        fwrite(data, N_DATA, 1, pf_data);
        //ads_print_data(data);
        ints++;
    }
}

int main(int argc, char *argv[]) {
    int i;
    pthread_t thid;
    int opt, spi_hz=ADS_SPI_HZ, test_signal=0, tmp_srate=1000, sample_rate=SRATE_1K, secs=20;


    //Parse command-line options
    while((opt = getopt(argc, argv, "ts:r:hl:")) != -1) {
        switch(opt) {
            case 'h':
                printf("%s [-t] [-s SPI_HZ] [-r SAMPLE_RATE]\n", argv[0]);
                printf("-t:\t\t\ttest signal enabled\n");
                printf("-s SPI_HZ:\t\t300000 to 3000000 [Baud]\n");
                printf("-r SAMPLE_RATE:\t\t125, 250, 500, 1000, 2000, 4000, 8000 [Hz]\n");
                exit(0);
            case 't':
                test_signal=1;
                break;
            case 's':
                spi_hz=atoi(optarg);
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
 
    printf("Init GPIOs\n");
    if ( ads_init_gpios() == -1 ) {
        printf("can't init GPIOs\n");
        return -1;
    }

    printf("Conf SPI\n");
    init_spi(spi_hz>>5);
    //init_spi(ADS_SPI_HZ);
    sleep(1);

    printf("Reset cycle\n");
    if ( ads_reset() == -1 ) {
        printf("can't reset ADS\n");
        return -1;
    }

    printf("Stop Read Data Continuously mode (just in case)\n");
    if ( ads_sdatac() == -1 ) {
        printf("can't stop read data continuously\n");
        return -1;
    }
    sleep(1);

    printf("POR registers\n");
    //ads_print_registers();

    printf("Set sample rate %d\n", tmp_srate);
    //if ( ads_set_rate(SRATE_500) == -1 ) {
    if ( ads_set_rate(sample_rate) == -1 ) {
        //if( ads_set_rate(SRATE_125) == -1 ){
        printf("can't set sample rate!!\n");
        return -1;
    }
    sleep(1);

    printf("Enable internal reference\n");
    if ( ads_enable_intref() == -1 ) {
        printf("Error enabling internal reference!!\n");
        return -1;
    }
    sleep(2);

    //Just in case...
    printf("Checl ID\n");
    if ( ads_check_id() == -1 ) {
        printf("can't read IDReg\n");
        return -1;
    }

    if(test_signal){
        printf("Set test signals\n");
        if( ads_set_test() == -1 ){
            printf("can't read IDReg\n");
            return -1;
        }
    }

    ads_print_registers();

    printf("START: start continuous reading\n");
    if ( ads_start() == -1 ) {
        printf("can't start conversion\n");
        return -1;
    }

    printf("RDATAC: read data continuously\n");
    if ( ads_command(RDATAC) == -1 ) {
        printf("can't read data continuously\n");
        return -1;
    }

    close_spi();
    init_spi(spi_hz);


    //NOTE no muevas esto de aquí, siempre se genera una interrupción inicial
    //y podemos estar jodiendo la comunicación SPI!!!
    system("cat /proc/interrupts | grep gpiolib | echo Initial interrupts: $(awk '{print $2}')");
    //Create thread
    if (pthread_create(&thid, NULL, &thread_spidata, NULL)) {
        perror("Failed to create the toggle thread");
        return -1;
    }
    //while(1){
    for (i=0; i<secs; i++) {
        printf("Main sleep...\n");
        printf("Ints: %u\n", ints);
        sleep(1);
    }

    printf("Stop Read Data Continuously mode (just in case)\n");
    if ( ads_command(SDATAC) == -1 ) {
        printf("can't send SDATAC\n");
        return -1;
    }
    sleep(1);

    printf("Ints: %u\n", ints);
    system("cat /proc/interrupts | grep gpiolib | echo final interrupts: $(awk '{print $2}')");

    return 0;

}
