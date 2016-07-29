#ifndef _ADS1192_H_
#define _ADS1192_H_


// The number of the definitions are referenced to the GPIO in BBB
//#define GPIO_DATA_READY (32*1+17)       // ~DRDY -> GPIO1_17
#define GPIO_DATA_READY (32*3+19)       // ~DRDY -> GPIO3_19        ÑAPA!!
//#define GPIO_RESET      VDD           // En MedCape está a VDD!!
//#define GPIO_START      GND           // En MedCape está a GND!!
//#define GPIO_CLKSEL     VDD           // En MedCape está a VDD!! -> reloj interno


#define N_SAPLES     2                      // Samples per Int
#define N_BPS        2                      // Bytes per sample
#define N_DATA       (2+N_SAPLES*N_BPS)     // Status + Bytes per Int


//#define ADS_SPI_HZ        300000        // Conservador
//#define ADS_SPI_HZ       1000000        // Máximo teórico SCLK=2*CLK=2*512=1024KHz
#define ADS_SPI_HZ       3000000        // Parece funcionar...
//#define ADS_SPI_HZ      10000000        // Peta un poco...

#define ID_VALUE     0x51                   // ID value for ADS1192

// System Commands
#define WAKEUP      0x02                // Wake-up from standby mode
#define STANDBY     0x04                // Enter standby mode
#define RESET       0x06                // Reset the device
#define START       0x08                // Start/restart (synchronize) conversions
#define STOP        0x0A                // Stop conversion
#define OFFSETCAL   0x1A                // Channel offset calibration

// Data Read Commands
#define RDATAC      0x10                // Enable Read Data Continuous mode. This mode is the default mode at power-up.
#define SDATAC      0x11                // Stop Read Data Continuously mode
#define RDATA       0x12                // Read data by command; supports multiple read back.

// Register Commands
#define RREG        0x20                // Read  n nnnn registers starting at address r rrrr -> 1B: 001r rrrr (2xh)  2B: 000n nnnn 
#define WREG        0x40                // Write n nnnn registers starting at address r rrrr -> 1B: 010r rrrr (4xh)  2B: 000n nnnn

// Registers
#define ID         0x00
#define CONFIG1    0x01
#define CONFIG2    0x02
#define LOFF       0x03
#define CH1SET     0x04
#define CH2SET     0x05
#define RLD_SENS   0x06
#define LOFF_SENS  0x07
#define LOFF_STAT  0x08
#define MISC1      0x09
#define MISC2      0x0A
#define GPIO       0x0B

#define NUM_REGS GPIO+1


#define SRATE_125  000
#define SRATE_250  001
#define SRATE_500  010                  //Default
#define SRATE_1K   011
#define SRATE_2K   100
#define SRATE_4K   101
#define SRATE_8K   110


#endif
