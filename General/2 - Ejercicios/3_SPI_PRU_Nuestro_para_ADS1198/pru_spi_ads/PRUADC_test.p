// PRU program to communicate to the MCPXXXX family of SPI ADC ICs. The program 
// generates the SPI signals that are required to receive samples. To use this 
// program as is, use the following wiring configuration:

//   Data_ready        :   P9_24    pr1_pru0_pru_r31_16 r31.t16
//   Start             :   P9_25    pr1_pru0_pru_r30_7  r30.t7
//   Reset             :   P9_31    pr1_pru0_pru_r30_0  r30.t0

//   Chip Select (CS):   P9_27    pr1_pru0_pru_r30_5  r30.t5
//   MOSI            :   P9_29    pr1_pru0_pru_r30_1  r30.t1
//   MISO            :   P9_28    pr1_pru0_pru_r31_3  r31.t3
//   CLK             :   P9_30    pr1_pru0_pru_r30_2  r30.t2

//   Sample Clock    :   P8_46    pr1_pru1_pru_r30_1  -- for testing only


// This program was writen by Derek Molloy to align with the content of the book 
// Exploring BeagleBone. See exploringbeaglebone.com/chapter13/

.setcallreg  r29.w2		 // set a non-default CALL/RET register
.origin 0                        // start of program in PRU memory
.entrypoint START                // program entry point (for a debugger)

#define PRU0_R31_VEC_VALID 32    // allows notification of program completion
#define PRU_EVTOUT_0    3        // the event number that is sent back

// Constants from the MCP3004/3008 datasheet 
#define TIME_CLOCK      12       // T_hi and t_lo = 125ns = 25 instructions (min)

//---------Translation of Sleep function (in C) to assembly language---------------
#define DELAY_SECONDS 2 // adjust this
#define CLOCK 200000000 // PRU is always clocked at 200MHz
#define CLOCKS_PER_LOOP 2 // loop contains two instructions, one clock each
#define DELAYCOUNT DELAY_SECONDS * CLOCK / CLOCKS_PER_LOOP
//--------------------------------------------------------------------------------
START:
	SET		r30.t0 //Reset
END:
	MOV	r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0	
	HALT                     // End of program -- below are the "procedures"

