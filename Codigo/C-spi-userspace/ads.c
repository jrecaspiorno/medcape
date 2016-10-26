#include <stdio.h>
#include <stdint.h>
#include <unistd.h>

#include "ads.h"
#include "bbb_gpio.h"
#include "bbb_spi.h"



int ads_init_gpios(void) {

    //Init GPIOs
    if ( init_gpio(GPIO_DATA_READY) ||
         set_gpio_direction(GPIO_DATA_READY, "in") ) {
        return -1;
    }

#ifdef ADS1198
    if ( init_gpio(GPIO_RESET) ||
         set_gpio_direction(GPIO_RESET, "out") ) {
        return -1;
    }

    if ( init_gpio(GPIO_START) ||
         set_gpio_direction(GPIO_START, "out") ) {
        return -1;
    }
    set_gpio_value(GPIO_START, 0);
    usleep(10000);
#endif
    return 0;
}

int ads_reset(void) {

#ifdef ADS1192
    if ( ads_command(RESET) == -1 ) {
        printf("can't send SDATAC\n");
        return -1;
    }
    return 0;
#endif

#ifdef ADS1198
    set_gpio_value(GPIO_RESET, 1);
    sleep(1);
    set_gpio_value(GPIO_RESET, 0);
    sleep(1);
    set_gpio_value(GPIO_RESET, 1);
    sleep(1);
    return 0;
#endif
    return -1;
}

int ads_start(void) {

#ifdef ADS1192
    if ( ads_command(START) == -1 ) {
        printf("can't send START\n");
        return -1;
    }
    return 0;
#endif

#ifdef ADS1198
    set_gpio_value(GPIO_START, 1);
    return 0;
#endif
    return -1;
}

int ads_rdatac(void) {
    if ( ads_command(RDATAC) == -1 ) {
        printf("can't send RDATAC\n");
        return -1;
    }
    return 0;
}

int ads_sdatac(void) {
    if ( ads_command(SDATAC) == -1 ) {
        printf("can't send SDATAC\n");
        return -1;
    }
    return 0;
}

int ads_print_registers(void) {
    uint8_t data[NUM_REGS];
#ifdef ADS1192
    //Read actual CONFIG1
    if ( ads_read_registers(ID, GPIO, data) == -1 ) {
        printf("Read error!!\n");
        return -1;
    }

    printf("ID:        %02X\n", data[ID]);
    printf("CONFIG1:   %02X\n", data[CONFIG1]);
    printf("CONFIG2:   %02X\n", data[CONFIG2]);
    printf("LOF:       %02X\n", data[LOFF]);
    printf("CH1SET:    %02X\n", data[CH1SET]);
    printf("CH2SET:    %02X\n", data[CH2SET]);
    printf("RLD_SENS:  %02X\n", data[RLD_SENS]);
    printf("LOFF_SENS: %02X\n", data[LOFF_SENS]);
    printf("LOFF_STAT: %02X\n", data[LOFF_STAT]);
    printf("MISC1:     %02X\n", data[MISC1]);
    printf("MISC2:     %02X\n", data[MISC2]);
    printf("GPIO:      %02X\n", data[GPIO]);
#endif
#ifdef ADS1198
    //Read actual CONFIG1
    if ( ads_read_registers(ID, WCT2, data) == -1 ) {
        printf("Read error!!\n");
        return -1;
    }
    printf("ID:         %02X\n", data[ID]);
    printf("CONFIG1:    %02X\n", data[CONFIG1]);
    printf("CONFIG2:    %02X\n", data[CONFIG2]);
    printf("CONFIG3:    %02X\n", data[CONFIG3]);
    printf("LOFF:       %02X\n", data[LOFF]);
    printf("CH1SET:     %02X\n", data[CH1SET]);
    printf("CH2SET:     %02X\n", data[CH2SET]);
    printf("CH3SET:     %02X\n", data[CH3SET]);
    printf("CH4SET:     %02X\n", data[CH4SET]);
    printf("CH5SET:     %02X\n", data[CH5SET]);
    printf("CH6SET:     %02X\n", data[CH6SET]);
    printf("CH7SET:     %02X\n", data[CH7SET]);
    printf("CH8SET:     %02X\n", data[CH8SET]);
    printf("RLD_SENSP:  %02X\n", data[RLD_SENSP]);
    printf("RLD_SENSN:  %02X\n", data[RLD_SENSN]);
    printf("LOFF_SENSP: %02X\n", data[LOFF_SENSP]);
    printf("LOFF_SENSN: %02X\n", data[LOFF_SENSN]);
    printf("LOFF_FLIP:  %02X\n", data[LOFF_FLIP]);
    printf("LOFF_STATP: %02X\n", data[LOFF_STATP]);
    printf("LOFF_STATN: %02X\n", data[LOFF_STATN]);
    printf("GPIO:       %02X\n", data[GPIO]);
    printf("PACE:       %02X\n", data[PACE]);
    printf("RESERVED:   %02X\n", data[RESERVED]);
    printf("CONFIG4:    %02X\n", data[CONFIG4]);
    printf("WCT1:       %02X\n", data[WCT1]);
    printf("WCT2:       %02X\n", data[WCT2]);
#endif
    return 0;
}

//int ads_write_register(uint8_t reg, uint8_t data){
//    return(ads_write_registers(reg, reg, &data));
//}

int ads_command(uint8_t command) {

    tx[0] = command;
    if (tx_spi(1) == -1) {
        printf("can't send command\n");
        return -1;
    }

    return 0;
}

int ads_read_registers(uint8_t reg_inic, uint8_t reg_fin, uint8_t *data) {

    int i, nregs=reg_fin-reg_inic;

    //Just in case...
    for ( i=0; i<BUFFERSIZE; i++ ) {
        tx[i]=0x00;
        rx[i]=0xff;
    }

    tx[0] = RREG | reg_inic;    // Start reading register
    tx[1] = nregs;              // Number of registers to read -1
    if (tx_spi(2+nregs+1) == -1) {
        printf("can't read registers\n");
        return -1;
    }

    for ( i=0; i<=nregs; i++ )
        data[i]=rx[i+2];

    return 0;
    int ads_print_registers(void);
}                        //int ads_write_register(uint8_t reg, uint8_t data);


int ads_write_registers(uint8_t reg_inic, uint8_t reg_fin, uint8_t *data) {

    int i, nregs=reg_fin-reg_inic;

    //Just in case...
    for ( i=0; i<BUFFERSIZE; i++ ) {
        tx[i]=0x00;
        rx[i]=0xff;
    }

    tx[0] = WREG | reg_inic;    // Start writing on reg_inic
    tx[1] = nregs;              // Number of registers to read -1
    for ( i=0; i<=nregs; i++ )
        tx[i+2]=data[i];

    if (tx_spi(2+nregs+1) == -1) {
        printf("can't write registers\n");
        return -1;
    }

    return 0;
}


int ads_read_sample(uint8_t *data) {
    int i;

    /* ADS1192
    rx[0] = 0x00;       // Status0 (value) 1100 + LOFF_STAT[4:0] + GPIO[1:0] + 00000
    rx[1] = 0x00;       // Status1 (value)
    rx[2] = 0x00;       // Ch1_MSB (value)
    rx[3] = 0x00;       // Ch1_LSB (value)
    rx[4] = 0x00;       // Ch2_MSB (value)
    rx[5] = 0x00;       // Ch2_LSB (value)
    */
    //NOTE sin esto no funciona, puede haber un comando en el buffer de transmisión que
    //lo fastidie todo!!
    for ( i=0; i<BUFFERSIZE; i++ ) {
        tx[i]=0x00;
        rx[i]=0xff;
    }

    if (tx_spi(N_DATA) == -1) {
        printf("can't read sample\n");
        return -1;
    }

    for ( i=0; i<N_DATA; i++) {
        data[i]=rx[i];
    }

    return 0;
}

int ads_enable_intref(void) {
    uint8_t buff;

#ifdef ADS1192
    //Read actual CONFIG2
    if ( ads_read_registers(CONFIG2, CONFIG2, &buff) == -1 ) {
        printf("Read error!!\n");
        return -1;
    }

    buff |= 0x20;           // PDB_REFBUF:1
    if ( ads_write_registers(CONFIG2, CONFIG2, &buff) == -1 ) {
        printf("Write error!!\n");
        return -1;
    }
    return 0;
#endif


#ifdef ADS1198
    //Read actual CONFIG3
    if ( ads_read_registers(CONFIG3, CONFIG3, &buff) == -1 ) {
        printf("Read error!!\n");
        return -1;
    }

    buff |= 0x80;           // PD_REFBUF:1
    if ( ads_write_registers(CONFIG3, CONFIG3, &buff) == -1 ) {
        printf("Write error!!\n");
        return -1;
    }
    return 0;
#endif
    return -1;
}

int ads_check_id(void) {
    int i;

#ifdef ADS1192
    uint8_t buff[3];
    //Just in case...
    for ( i=0; i<3; i++ )  buff[i]=0xff;
    if ( ads_read_registers(ID, CONFIG2, buff) == -1 ) {
        printf("can't read IDReg\n");
        return -1;
    }
    printf("ID: %02X, CONFIG1: %02X, CONFIG2: %02X\n", buff[0], buff[1], buff[2]);
#endif


#ifdef ADS1198
    uint8_t buff[4];
    //Just in case...
    for ( i=0; i<4; i++ )  buff[i]=0xff;
    if ( ads_read_registers(ID, CONFIG3, buff) == -1 ) {
        printf("can't read IDReg\n");
        return -1;
    }
    printf("ID: %02X, CONFIG1: %02X, CONFIG2: %02X, CONFIG3: %02X\n", buff[0],
           buff[1], buff[2], buff[3]);
#endif

    if (buff[0]!=ID_VALUE) {
        printf("Devide ID error!!\n");
        return -1;
    }
    return 0;
}


int ads_set_test(void) {

#ifdef ADS1192
    uint8_t buff[2];

    //Read actual CONFIG2
    if ( ads_read_registers(CONFIG2, CONFIG2, buff) == -1 ) {
        printf("Read error!!\n");
        return -1;
    }

    buff[0] |= 0x03;           // INT_TEST:1,  TEST_FREQ:1
    if ( ads_write_registers(CONFIG2, CONFIG2, buff) == -1 ) {
        printf("Write error!!\n");
        return -1;
    }

    buff[0]=0x05;           // PD1:0, GAIN1:000, MUX1:0101 (Test signal)
    buff[1]=0x05;           // PD2:0, GAIN2:000, MUX2:0101 (Test signal)
    if ( ads_write_registers(CH1SET, CH2SET, buff) == -1 ) {
        printf("Write error!!\n");
        return -1;
    }
    return 0;
#endif

#ifdef ADS1198
    uint8_t buff[9];

    buff[0] = 0x10;           // INT_TEST:1, TEST_AMP:0, TEST_FREQ:00
    if ( ads_write_registers(CONFIG2, CONFIG2, buff) == -1 ) {
        printf("Write error!!\n");
        return -1;
    }

    // PD:0, GAIN:000, MUXn:101
    buff[0] = 0x05;
    buff[1] = 0x05;
    buff[2] = 0x05;
    buff[3] = 0x05;
    buff[4] = 0x05;
    buff[5] = 0x05;
    buff[6] = 0x05;
    buff[7] = 0x05;
    buff[8] = 0x05;
    if ( ads_write_registers(CH1SET, CH8SET, buff) == -1 ) {
        printf("Write error!!\n");
        return -1;
    }
    return 0;
#endif
    return -1;

}


void ads_print_data(uint8_t *data) {
#ifdef ADS1192
    //The format of the 16 status bits is (1100 + LOFF_STAT[4:0] + GPIO[1:0] + 5 zeros).
    //The data format for each channel data are twos complement and MSB first.
    //When channels are powered down using the user register setting, the
    //corresponding channel output is set to '0'. However, the sequence of channel outputs remains the same.
    printf( "ST:  %02X%02X\t"
            "Ch1: %02X%02X\t"
            "Ch2: %02X%02X\t",
            data[0], data[1],
            data[2], data[3],
            data[4], data[5]);

    printf("%s", ((data[0]>>4)==0xC)?"\n":"*************\n");
    /*
    if( ((data[0]|0xf0) != 0xc0) || ((data[1]|0x1f) != 0x00) ) {
        printf("Data read error\n");
    }
    */

#endif

#ifdef ADS1198

    //The format of the 24 status bits is: (1100 + LOFF_STATP + LOFF_STATN + bits[4:7] of the GPIO register
    printf("ST: %02X%02X%02X\t", data[0], data[1], data[2]);

    //The data format for each channel data are twos complement and MSB first.
    //When channels are powered down using the user register setting, the
    //corresponding channel output is set to '0'. However, the sequence of channel outputs remains the same.
    printf("Ch1: %02X%02X\t",  data[3],  data[4]);
    printf("Ch2: %02X%02X\t",  data[5],  data[6]);
    printf("Ch3: %02X%02X\t",  data[7],  data[8]);
    printf("Ch4: %02X%02X\t",  data[9], data[10]);
    printf("Ch5: %02X%02X\t", data[11], data[12]);
    printf("Ch6: %02X%02X\t", data[13], data[14]);
    printf("Ch7: %02X%02X\t", data[15], data[16]);
    printf("Ch8: %02X%02X\t", data[17], data[18]);

    printf("%s", ((data[0]>>4)==0xC)?"\n":"*************\n");


#endif
}


int ads_set_rate(uint8_t rate) {
// For ADS1192 and ADS1198
#if defined(ADS1192) || defined(ADS1198)
    uint8_t buff;
    //Read actual CONFIG1
    if ( ads_read_registers(CONFIG1, CONFIG1, &buff) == -1 ) {
        printf("Read error!!\n");
        return -1;
    }

    buff = (buff&0xf8)|(rate&0x7);
    if ( ads_write_registers(CONFIG1, CONFIG1, &buff) == -1 ) {
        printf("Write error!!\n");
        return -1;
    }
    return 0;
#endif
    return -1;
}



/*
 *    printf("Set sample freq\n");
 *    //buff[0]=0x00;           // Continuous mode (SINGLE_SHOT=1), 0000, 125 SPS (DR:000)
 *    buff[0]=0x02;           // Continuous mode (SINGLE_SHOT:0), 500 SPS (DR:010)
 *    if( ads_write_registers(CONFIG1, CONFIG1, buff) == -1 ){
 *        printf("Write error!!\n");
 *        return -1;
 *    }
 */

