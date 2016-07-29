#ifndef _ADS1198_H_
#define _ADS1198_H_

// Alimentacion
// J4 Pin        | J4 Pin
// :------------:|:-----------:
// 2             | 1
// 4             | 3
// 6             | 5 (GND->P9-1)
// 8             | 7
// 10 (5V->P9-5) | 9 (3V3->P9-3)


// Configuración de pines para SPI e interfaz digital
//  Signal        | J3 Pin Num  | J3 Pin Num  | Signal       
// :-------------:|:-----------:|:-----------:|:-----------:
//  CLKSEL        | 2           | 1  (P9-28)  | START/**CS** 
//  GND           | 4           | 3  (P9-31)  | CLK          
//  GPIO1         | 6           | 5           | NC           
//  RESETB        | 8  (P9-25)  | 7           | CS           
//  GND           | 10          | 9           | NC           
//  GPIO2         | 12          | 11 (P9-30)  | DIN          
//  NC/**START**  | 14 (P9-27)  | 13 (P9-23)  | DOUT         
//  NC            | 16          | 15 (P9-23)  | DRDYB        
//  GND           | 18          | 17          | NC           
//  NC            | 20          | 19          | NC  


//SPI1_D0   P9-23 (BBB) Input           ->  J3-13 DOUT (ADS1x98ECG)
//SPI1_D1   P9-30 (BBB) Output          ->  J3-11 DIN  (ADS1x98ECG)
//SPI1_SCLK P9-31 (BBB)                 ->  J3-03 SCLK (ADS1x98ECG)
//SPI1_CS0  P9-28 (BBB)                 ->  J3-01 nCS  (ADS1x98ECG) NOTE depende del JP21 de la placa RevC posición 1-2

// Pin configuration for digital interface
#define GPIO_DATA_READY (32*1+17)       // GPIO1_17 or GPIO_49  ->  P9-23 (BBB) or J3-15 DRDY   (ADS1x98ECG)
#define GPIO_RESET      (32*3+21)       // GPIO3_21 or GPIO_117 ->  P9-25 (BBB) or J3-08 nRESET (ADS1x98ECG)
#define GPIO_START      (32*3+19)       // GPIO3_19 or GPIO_115 ->  P9-27 (BBB) or J3-14 START  (ADS1x98ECG) NOTE JP22 en posición 2-3
//#define GPIO_CLKSEL     GND           // En placa de evaluación a GND -> Reloj externo


//#define ADS_SPI_HZ      1500000         // Minimum period 50ns Maximum ->20MHz pero no va muy bien... 1.5MHz mejor para la BBB
#define ADS_SPI_HZ      (1500000*2)         // Parece que va bien...

#define ID_VALUE        0xB6            // ID value for ADS1198

#define N_SAPLES     8                      // Samples per Int
#define N_BPS        2                      // Bytes per sample
#define N_DATA       (2+N_SAPLES*N_BPS)     // Status + Bytes per Int


// System Commands
#define WAKEUP      0x02                // Wake-up from standby mode
#define STANDBY     0x04                // Enter standby mode
#define RESET       0x06                // Reset the device
#define START       0x08                // Start/restart (synchronize) conversions
#define STOP        0x0A                // Stop conversion
                                         
// Data Read Commands                    
#define RDATAC      0x10                // Enable Read Data Continuous mode. This mode is the default mode at power-up.
#define SDATAC      0x11                // Stop Read Data Continuously mode
#define RDATA       0x12                // Read data by command; supports multiple read back.
                                        
// Register Commands
#define RREG        0x20                // Read  n nnnn registers starting at address r rrrr -> 1B: 001r rrrr (2xh)  2B: 000n nnnn 
#define WREG        0x40                // Write n nnnn registers starting at address r rrrr -> 1B: 010r rrrr (4xh)  2B: 000n nnnn
                                                                                                                              
// Registers
#define ID          0x00                                                                                                           
#define CONFIG1     0x01                                                                                                           
#define CONFIG2     0x02
#define CONFIG3     0x03
#define LOFF        0x04
#define CH1SET      0x05
#define CH2SET      0x06
#define CH3SET      0x07
#define CH4SET      0x08
#define CH5SET      0x09
#define CH6SET      0x0A
#define CH7SET      0x0B
#define CH8SET      0x0C
#define RLD_SENSP   0x0D
#define RLD_SENSN   0x0E
#define LOFF_SENSP  0x0F
#define LOFF_SENSN  0x10
#define LOFF_FLIP   0x11
#define LOFF_STATP  0x12
#define LOFF_STATN  0x13
#define GPIO        0x14
#define PACE        0x15
#define RESERVED    0x16
#define CONFIG4     0x17
#define WCT1        0x18
#define WCT2        0x19


#define NUM_REGS ((WCT2)+1)


#define SRATE_8K    000     // Demasiado para la BBB
#define SRATE_4K    001     // T=500us, máximo funcionando. Desde DRDY hasta ads_read_sample pasan 180us, en recoger los datos tarda 84us (SCLK3MHz), sobran 230us para procesamiento...
#define SRATE_2K    010     
#define SRATE_1K    011     
#define SRATE_500   100     // Default
#define SRATE_250   101     
#define SRATE_125   110     

#endif
