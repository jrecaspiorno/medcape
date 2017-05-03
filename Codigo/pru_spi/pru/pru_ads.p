.setcallreg r29.w2                  // set a non-default CALL/RET register
.origin     0                       // start of program in PRU memory
.entrypoint MAIN                   // program entry point (for a debugger)


// SPI
#define CLK     r30.t0              // P9_31   pr1_pru0_pru_r30_0
#define MISO    r31.t1              // P9_29   pr1_pru0_pru_r31_1
#define MOSI    r30.t2              // P9_30   pr1_pru0_pru_r30_2
#define nCS     r30.t3              // P9_28   pr1_pru0_pru_r30_3

//GPIOs
#define nRESET  r30.t7              // P9_25   pr1_pru0_pru_r30_7
#define nDRDY   r31.t5              // P9_27   pr1_pru0_pru_r31_5
#define START   r30.t6              // P9_41b  pr1_pru0_pru_r30_6

#define CLK_LEVEL_DELAY     49995   // Freq=1/(CLK_LEVEL_DELAY×5×10^(−9)×2)
#define DELAY_1S        100000000

#define PRU0_R31_VEC_VALID  32      // allows notification of program completion
#define PRU_EVTOUT_0        3       // the event number that is sent back
#define PRU_EVTOUT_1        4       //allows notification of sample ready in RAM to be given to host program(in C)


//
// delay: delay loop
//
//  Parameters:
//      - REG:          tmp register
//      - DEL_CYCLES:   10ns delay cicles
//
//  Usage for 1sec:
//      DELAY_MACRO r0, 100000000
//
.macro DELAY_MACRO
.mparam     REG=r0, DEL_CYCLES=100

    mov     REG, DEL_CYCLES
M_DELAY:
    sub     REG, REG, 1             // decrement loop counter
    qbne    M_DELAY, REG, 0         // repeat loop unless zero

    .endm


//--------------------------------------------------------------------------------
MAIN:
    //Set output pins to 0 (just in case):
    CLR     START
    SET     nCS                     //CS is active low
    SET     MOSI                    //MOSI is active in original hardware SPI
    CLR     CLK

    /*
    MOV     r1, 0                   // load the base address into r1
    LBBO    r2,  r1,  0,  8         // load firs 8b
    SBBO    r2,  r1,  8,  8         // write them in the following bytes
    */

    // Reset
    SET     nRESET
    DELAY_MACRO r0, DELAY_1S
    CLR     nRESET
    DELAY_MACRO r0, DELAY_1S
    SET     nRESET
    DELAY_MACRO r0, DELAY_1S

    // Send SDATAC
    MOV     r1, 8                   // Nº of bit to be sent
    MOV     r2, 0x11                // Data to be sent
    CLR     nCS
    CALL    FUNCT_SPI 
    SET     nCS

    DELAY_MACRO r0, DELAY_1S
    
    // Ckeck ID
    CLR     nCS

    MOV     r1, 16                  // Nº of bit to be sent
    MOV     r2, 0x2002              // Data to be sent
    CALL    FUNCT_SPI 

    MOV     r1, 24                  // Nº of bit to be sent
    MOV     r2, 0x0                 // Data to be sent
    CALL    FUNCT_SPI 

    SET     nCS

    
    MOV     r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0
    HALT                            // End of program



//
//  Send/receive data MOSI/SIMO
//  
//  Registers used:
//      - RO: tmp
//      - R1: number of bist to be sent
//      - R2: bits for MOSI
//      - R3: bits received from MISO
//  

FUNCT_SPI:
    RSB     r0, r1, 32
    LSL     r2, r2, r0.b0           // Ponemos el primer bit a enviar en r2.t31
    
FSPI_SEND_BIT:
    DELAY_MACRO r0, CLK_LEVEL_DELAY // delay for low clock
    QBBC    FSPI_MO_0, r2.t31       // Set/clear MOSI
    SET     MOSI
    QBA     FSPI_MO_1
FSPI_MO_0:
    CLR     MOSI
FSPI_MO_1:
    SET     CLK                     // Set the clock high
    DELAY_MACRO r0, CLK_LEVEL_DELAY
    CLR     CLK                     // set the clock low
    QBBC    FSPI_SI_0, MISO         // read the response on MISO
    OR      r3, r3, 0x00000001
FSPI_SI_0:  

    LSL     r2, r2, 1
    LSL     r3, r3, 1 

    SUB     r1, r1, 1
    QBNE    FSPI_SEND_BIT, r1, 0    // Any bits left?

    SET     MOSI                    //MOSI is active in original hardware SPI
    DELAY_MACRO r0, CLK_LEVEL_DELAY // Last delay, CS can now be desabled
    
    RET
    





