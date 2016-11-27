#ifndef _ADS_H_
#define _ADS_H_

#define ADS1198
//#define ADS1192

#ifdef ADS1198
    #include "ads1198.h"
#endif

#ifdef ADS1192
    #include "ads1192.h"
#endif

int ads_init_gpios(void);
int ads_reset(void);
int ads_start(void);
int ads_rdatac(void);
int ads_sdatac(void);
int ads_print_registers(void);
//int ads1191_write_register(uint8_t reg, uint8_t data);
int ads_command(uint8_t command);
int ads_read_registers(uint8_t reg_inic, uint8_t reg_fin, uint8_t *data);
int ads_write_registers(uint8_t reg_inic, uint8_t reg_fin, uint8_t *data);
int ads_read_sample(uint8_t *data);
int ads_enable_intref(void);
int ads_check_id(void);
int ads_set_test(void);
void ads_print_data(uint8_t *data);
int ads_set_rate(uint8_t rate);

#endif
