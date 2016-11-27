#ifndef _BBB_SPI_H_
#define _BBB_SPI_H_

#include <stdint.h>

#define BUFFERSIZE 32

extern uint8_t tx[BUFFERSIZE];
extern uint8_t rx[BUFFERSIZE];

int init_spi(uint32_t speed);
void close_spi(void);
int tx_spi(uint32_t len);

#endif