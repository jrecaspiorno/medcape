.setcallreg r29.w2                  // set a non-default CALL/RET register
.origin     0                       // start of program in PRU memory
.entrypoint MAIN                   // program entry point (for a debugger)

//#include "PRU_timer0Interrupt.hp"
//#define CONST_IEP C26
#define CONST_PRUSSINTC     C0
#define SICR_OFFSET       0x24

// SPI
#define CLK     r30.t0              // P9_31   pr1_pru0_pru_r30_0
#define MISO    r31.t1              // P9_29   pr1_pru0_pru_r31_1
#define MOSI    r30.t2              // P9_30   pr1_pru0_pru_r30_2
#define nCS     r30.t3              // P9_28   pr1_pru0_pru_r30_3

//GPIOs
#define nRESET  r30.t7              // P9_25   pr1_pru0_pru_r30_7
#define nDRDY   r31.t5              // P9_27   pr1_pru0_pru_r31_5
#define START   r30.t6              // P9_41b  pr1_pru0_pru_r30_6

// Freq=1/( (2xCLK_LEVEL_DELAY) × (2×5×10^(−9)) )
//#define CLK_LEVEL_DELAY     49995   // 1KHz

#define DELAY_1S        100000000

//Registro r31 sirve para generar interrupciones.
//Hay que poner el bit 5 a 1 y en los bits [3-0] el número de interrupción (10 xxxx)
// PRU0_R31_VEC_VALID | PRU_EVTOUT_0 = 10 0011
// PRU0_R31_VEC_VALID | PRU_EVTOUT_1 = 10 0100
#define PRU0_R31_VEC_VALID  32      // allows notification of events
#define PRU_EVTOUT_0        3       // Evento de finalización
#define PRU_EVTOUT_1        4       // Evento de notificación de tarea completada


//
// delay: delay of DEL_CYCLES*10ns+5ns
//
//  Parameters:
//      - REG:          tmp register
//      - DEL_CYCLES:   10ns delay cicles
//
//  Usage for 1sec:
//      DELAY_MACRO r0, 100000000
//
.macro DELAY_MACRO
.mparam     REG, DEL_CYCLES

    mov     REG, DEL_CYCLES
M_DELAY:
    sub     REG, REG, 1             // decrement loop counter
    qbne    M_DELAY, REG, 0         // repeat loop unless zero

    .endm



//Variables
#define CLK_DEL             r0      // 1KHz
#define tmp                 r4      // tmp
#define pRAM                r5      // RAM pointer
#define iter                r6      // Iterator
#define sNumber             r7      // Sample number
    
   
//--------------------------------------------------------------------------------
//  Registers used:
//      - R0: CLK_LEVEL_DELAY
//      - R1: number of bist to be sent
//      - R2: bits for MOSI
//      - R3: bits received from MISO
//      - R4: tmp
//      - R5: puntero de lectura en RAM
//      - R6: numero de envíos

MAIN:
    //Set output pins to 0 (just in case):
    CLR     START
    SET     nCS                     //CS is active low
    SET     MOSI                    //MOSI is active in original hardware SPI
    CLR     CLK

    // Reset
    SET     nRESET
    DELAY_MACRO tmp, DELAY_1S
    CLR     nRESET
    DELAY_MACRO tmp, DELAY_1S
    SET     nRESET
    DELAY_MACRO tmp, DELAY_1S

    MOV     pRAM, 0                 // load the base address
    LBBO    CLK_DEL, pRAM, 0, 4     // load CLK_DEL, 4Bytes from pRAM

MORE_MSGs:
    // clear interrupt
    MOV     tmp, 21
    SBCO    tmp, CONST_PRUSSINTC, SICR_OFFSET, 4
    // wait for interrupt
    WBS     r31, #30
    // clear interrupt
    MOV     tmp, 21
    SBCO    tmp, CONST_PRUSSINTC, SICR_OFFSET, 4

    MOV     pRAM, 4                 // load the base address
    LBBO    iter,   pRAM, 0, 4      // load nMsgs, 4Bytes from pRAM+4
    MOV     pRAM, 8
    QBEQ    START_DATA, iter, 0     // No MSGs?

   
NEXT_MSG:    
    QBEQ    FIN_MSGs, iter, 0          // More MSGs?

        // Send MSG
        LBBO    r1, pRAM, 0, 8          // r1=bits to be sent, r2=data (from pRAM)
        CLR     nCS
        CALL    FUNCT_SPI 
        SBBO    r3, pRAM, 4, 4          // r3=reply -> en pRAM[data]

        SUB     iter, iter, 1           // iterator--
        LBBO    r1, pRAM, 8, 4          // delay
        ADD     pRAM, pRAM, 12          // Pointer+=12Bytes
        
        //Delay??
        QBEQ    NEXT_MSG, r1, 0

        //Delay SPI * 2^Delay
        SET     nCS
        LSL     r1, CLK_DEL, r1
        DELAY_MACRO tmp, r1

        JMP NEXT_MSG
        
FIN_MSGs:
        
        MOV     r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_1  // Evento de mensajes enviados
        JMP     MORE_MSGs 

START_DATA:    


    //RDATAC
    CLR     nCS
    MOV     r1, 8
    MOV     r2, 0x10
    CALL    FUNCT_SPI 
    SET     nCS

    DELAY_MACRO tmp, CLK_DEL

    // load new CLK_DEL
    MOV     pRAM, 0                 // load the base address of Data RAM0
    LBBO    CLK_DEL, pRAM, 0, 4

    // clear interrupt
    MOV     tmp, 21
    SBCO    tmp, CONST_PRUSSINTC, SICR_OFFSET, 4

    SET     START
    MOV     r2,   0                 // Data for MOSI
    MOV     sNumber, 0              // Sample Number
    
NEXT_BANK:    
    MOV     iter, 0                 // nSamples in the buffer

NEXT_SAMPLE:
    
    WBC     nDRDY                   //Wait for data

    // End of sampling inte received?
    QBBS    END, r31, #30

    SBBO    sNumber, pRAM, 0, 2     // Sample number(2B) en pRAM
    
    CLR     nCS

    //Read status
    MOV     r1, 16
    CALL    FUNCT_SPI 
    SBBO    r3, pRAM, 2, 2          // Status(2B) en pRAM
    //Read data
    MOV     r1, 32
    CALL    FUNCT_SPI 
    SBBO    r3, pRAM, 4, 4          // Channles(2B*2Ch) en pRAM

    SET     nCS
    ADD     pRAM, pRAM, 8
    ADD     iter, iter, 1
    ADD     sNumber, sNumber, 1

    
    QBBC    NEXT_SAMPLE, iter.t10   //2^10 muestras leidas? (6KiB)

    MOV     r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_1  // Evento de muestras leídas

    // Cambiamos de banco
    //QBBS    RAM0, pRAM.t13          // Banco 0??
    QBBS    RAM0, pRAM.t14          // Banco 0?? con banco lleno!!
    MOV     pRAM, 1                 // load the base address of Data RAM1
    LSL     pRAM, pRAM, 13
    JMP     NEXT_BANK
    
RAM0:    
    MOV     pRAM, 0                 // load the base address of Data RAM0
    JMP     NEXT_BANK

END:
   
    CLR     START

    DELAY_MACRO tmp, DELAY_1S
    
    MOV     r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0  //Generamos evento de finalización
    HALT                            // End of program



//
//  Send/receive data MOSI/SIMO
//  
//  Registers used:
//      - R0: CLK_LEVEL_DELAY (solo lectura)
//      - R1: number of bist to be sent
//      - R2: bits for MOSI
//      - R3: bits received from MISO
//      - R4: tmp
//  

FUNCT_SPI:
    MOV     r3, 0
    RSB     r4, r1, 32
    LSL     r2, r2, r4.b0           // Ponemos el primer bit a enviar en r2.t31
    
FSPI_SEND_BIT:
    LSL     r3, r3, 1               // Hacemos hueco para el nuevo bit MISO
    DELAY_MACRO r4, r0              // delay for low clock
    QBBC    FSPI_MO_0, r2.t31       // Set/clear MOSI
    SET     MOSI
    QBA     FSPI_MO_1
FSPI_MO_0:
    CLR     MOSI
FSPI_MO_1:
    SET     CLK                     // Set the clock high
    DELAY_MACRO r4, r0
    CLR     CLK                     // set the clock low
    QBBC    FSPI_SI_0, MISO         // read the response on MISO
    OR      r3, r3, 1
FSPI_SI_0:  

    LSL     r2, r2, 1

    SUB     r1, r1, 1
    QBNE    FSPI_SEND_BIT, r1, 0    // Any bits left?

    DELAY_MACRO r4, r0              // Last delay, CS can now be desabled
    CLR     MOSI                    //MOSI low as in datasheet
    
    RET
    





