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


    //get_gpio_ints();
    //return 0;


    printf("Init GPIOs\n");
    if ( ads_init_gpios() == -1 ) {
        printf("can't init GPIOs\n");
        return -1;
    }

    printf("Conf SPI\n");
    init_spi(ADS_SPI_HZ>>5);
//    init_spi(ADS_SPI_HZ);
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

    printf("Set sample rate\n");
    if ( ads_set_rate(SRATE_1K) == -1 ) {
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

    /*
    printf("Set test signals\n");
    if( ads_set_test() == -1 ){
        printf("can't read IDReg\n");
        return -1;
    }
    */

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
    init_spi(ADS_SPI_HZ);


    //NOTE no muevas esto de aquí, siempre se genera una interrupción inicial
    //y podemos estar jodiendo la comunicación SPI!!!
    system("cat /proc/interrupts | grep 177 | echo Initial interrupts: $(awk '{print $2}')");
    //Create thread
    if (pthread_create(&thid, NULL, &thread_spidata, NULL)) {
        perror("Failed to create the toggle thread");
        return -1;
    }
    //while(1){
    for (i=0; i<10; i++) {
        printf("Main sleep...\n");
        printf("Ints: %u\n", ints);

        sleep(5);
    }

    printf("Stop Read Data Continuously mode (just in case)\n");
    if ( ads_command(SDATAC) == -1 ) {
        printf("can't send SDATAC\n");
        return -1;
    }
    sleep(1);

    printf("Ints: %u\n", ints);
    system("cat /proc/interrupts | grep 177 | echo final interrupts: $(awk '{print $2}')");

    return 0;

}
