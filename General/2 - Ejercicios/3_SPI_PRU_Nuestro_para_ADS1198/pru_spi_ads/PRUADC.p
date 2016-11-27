// PRU program to communicate to the MCPXXXX family of SPI ADC ICs. The program 
// generates the SPI signals that are required to receive samples. To use this 
// program as is, use the following wiring configuration:
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

START:
	// Need to wait at this point until it is ready to take a sample - i.e., 0x00010000 
	// store the address in r5
	MOV	r5, 0x00010000   // LSB of value at this address is the clock flag
SAMPLE_WAIT_HIGH:		 // wait until the PRU1 sample clock goes high
	LBBO	r6, r5, 0, 4	 // load the value at address r5 into r6		
	QBNE	SAMPLE_WAIT_HIGH, r6, 1 // if the 

SAMPLE_WAIT_LOW:                 // need to wait here if the sample clock has not gone low
	LBBO	r6, r5, 0, 4	 // load the value in PRU1 sample clock address r5 into r6
	QBNE	SAMPLE_WAIT_LOW, r6, 0 // wait until the sample clock goes low (just in case)
END:
	MOV	r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0	
	HALT                     // End of program -- below are the "procedures"
