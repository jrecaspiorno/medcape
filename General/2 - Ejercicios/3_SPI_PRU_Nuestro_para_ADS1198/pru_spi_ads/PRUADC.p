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
#define PRU_EVTOUT_1	4        //allows notification of sample ready in RAM to be given to host program(in C)

#define SIZE_CHUNK_RAM 10 //Size of chunks(1chunk=1sample, we could change this) of the circular buffer to store values in RAM
						  //E.g when we write the 2nd chunk(here in the pru), at the same time we are reading the 1st chunk(in host program)

// Constants from the MCP3004/3008 datasheet 
#define TIME_CLOCK      12       // T_hi and t_lo = 125ns = 25 instructions (min)

//---------Translation of Sleep function (in C) to assembly language---------------
#define DELAY_SECONDS 2 // adjust this
#define CLOCK 200000000 // PRU is always clocked at 200MHz
#define CLOCKS_PER_LOOP 2 // loop contains two instructions, one clock each
#define DELAYCOUNT DELAY_SECONDS * CLOCK / CLOCKS_PER_LOOP
//--------------------------------------------------------------------------------
START:
//Set output pins to 0 (just in case):
	CLR r30.t7
	CLR r30.t0
	SET r30.t5 //CS is active low
	SET r30.t1 //MOSI is active in original hardware SPI
	CLR r30.t2
	
 // Enable the OCP master port -- allows transfer of data to Linux userspace
	LBCO    r0, C4, 4, 4     // load SYSCFG reg into r0 (use c4 const addr)
	CLR     r0, r0, 4        // clear bit 4 (STANDBY_INIT)
	SBCO    r0, C4, 4, 4     // store the modified r0 back at the load addr

	MOV	r1, 0x00000000	 // load the base address into r1

	LBBO    r8, r1, 16, 20     // load the Linux address that is passed into r8 -- to store sample values
	LBBO	r9, r1, 20, 24	 // load the size that is passed into r9 -- the number of samples to take
	LBBO	r11, r1, 24, 28	 // spi speed

	// clear registers to receive the response from the ADS
	MOV	r3,  0x000000
	MOV	r17, 0x000000 
	MOV	r18, 0x000000	
	MOV	r19, 0x000000	 
	MOV	r20, 0x000000	
	MOV	r23, 0x000000	 
	MOV r27, 0x000000	

	//Counter of bits written into register of their comment:
	MOV	r22, 0 //r3
	MOV	r25, r8	 //backup register of initial cell of RAM memory
	MOV	r26, 0	//r20
	
	
	MOV r14, 32 //Size of 1 PRU register
	MOV	r24, 0	//Counter of samples taken(it resets every loop of the circular buffer)
	
/* Original Signals(commands) to ADS in C program:
Reset
SDATAC
(Sleep 1)
Set_ads_sample_rate
(Sleep 1)
ads_enable_intref
(Sleep 2)
Start
RDATAC
*/
//================================================================================================
//-----------Signals(commands) to ADS:--------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
//----------------------RESET----------------------
	SET		r30.t0 //Reset
//------------------SLEEP 2 SECONDS---------------------------
MOV	r12, DELAYCOUNT	
CALL DELAY_FUNCTION
//----------------------SDATAC----------------------
SEND_SDATAC:
	LBBO	r2, r1, 0, 4	 // Load sdatac command=0x11
	//We only want 8 bits, but 1 register=32 bits and we can only take 1 bit per clock edge,
	//so we do lsl(logical shift left) to move our LSB to MSB positions(i.e. first 8 positions)
	SUB r13, r14, 8
	CALL TRUNCATE
	//-----------------------------------------------------------------------------------------
	CLR	r30.t5		 // set the CS line low (active low)
	MOV	r4, 8		 // going to write/read 8 bits (1 byte)
	CALL	SPICLK_LOOP           // repeat call the SPICLK procedure until all 8 bits written/read
	SET	r30.t5		 // pull the CS line high (end of sample)
//------------------SLEEP 2 SECONDS---------------------------
MOV	r12, DELAYCOUNT	
CALL DELAY_FUNCTION
//---------------------SET_ADS_SAMPLE_RATE	(i.e. data_ready rate)----------------------
/*
SET_ADS_SAMPLE_RATE:
	//LBBO	r2, r1, 8, 12	 // Load sample rate speed
//1st bit=============
	MOV r2, 0x41
	//We only want 8 bits, but 1 register=32 bits and we can only take 1 bit per clock edge,
	//so we do lsl(logical shift left) to move our LSB to MSB positions(i.e. first 8 positions)
	SUB r13, r14, 8
	CALL TRUNCATE
	//-----------------------------------------------------------------------------------------
	CLR	r30.t5		 // set the CS line low (active low)
	MOV	r4, 8		 // going to write/read 8 bits (1 byte)
	CALL	SPICLK_LOOP           // repeat call the SPICLK procedure until all 8 bits written/read
	
	MOV	r12, 1000 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo	
	CALL DELAY_FUNCTION
//2nd bit=============	
	MOV r2, 0x00000000
	MOV	r4, 8		 // going to write/read 8 bits (1 byte)
	CALL	SPICLK_LOOP           // repeat call the SPICLK procedure until all 8 bits written/read
	
	MOV	r12, 1000	 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
	CALL DELAY_FUNCTION
//3rd bit=============	
	MOV r2, 0x00000000
	MOV	r4, 8		 // going to write/read 8 bits (1 byte)
	CALL	SPICLK_LOOP           // repeat call the SPICLK procedure until all 8 bits written/read

	MOV	r12, 1000	//Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
	CALL DELAY_FUNCTION
	
	SET	r30.t5		 // pull the CS line high (end of sample)
	SET r30.t1 //MOSI is active in original hardware SPI
//------------------SLEEP 2 SECONDS---------------------------
MOV	r12, DELAYCOUNT
CALL DELAY_FUNCTION
*/
//---------------------SET_INTERNAL_REFERENCE_ADS(i.e. data_ready rate)----------------------
SET_INTERNAL_REFERENCE_ADS:
//1st bit=============	
	//LBBO	r2, r1, 8, 12	 // Load sample rate speed
	MOV r2, 0x43
	//We only want 8 bits, but 1 register=32 bits and we can only take 1 bit per clock edge,
	//so we do lsl(logical shift left) to move our LSB to MSB positions(i.e. first 8 positions)
	SUB r13, r14, 8
	CALL TRUNCATE
	
	CLR	r30.t5		 // set the CS line low (active low)
	MOV	r4, 8		 // going to write/read 8 bits (1 byte)
	CALL	SPICLK_LOOP           // repeat call the SPICLK procedure until all 8 bits written/read
	
	MOV	r12, 1000	//Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
	CALL DELAY_FUNCTION	
//2nd bit=============	
	MOV r2, 0x00000000
	MOV	r4, 8		 // going to write/read 8 bits (1 byte)
	CALL	SPICLK_LOOP           // repeat call the SPICLK procedure until all 8 bits written/read
	
	MOV	r12, 1000	//Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
	CALL DELAY_FUNCTION	
//3rd bit=============	
	MOV r2, 0xC0
	SUB r13, r14, 8
	//We only want 8 bits, but 1 register=32 bits and we can only take 1 bit per clock edge,
	//so we do lsl(logical shift left) to move our LSB to MSB positions(i.e. first 8 positions)
	CALL TRUNCATE
	
	MOV	r4, 8		 // going to write/read 8 bits (1 byte)
	CALL	SPICLK_LOOP           // repeat call the SPICLK procedure until all 8 bits written/read

	MOV	r12, 1000	//Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
	CALL DELAY_FUNCTION
	
	SET	r30.t5		 // pull the CS line high (end of sample)
	SET r30.t1 //MOSI is active in original hardware SPI
//------------------SLEEP 2 SECONDS---------------------------
MOV	r12, DELAYCOUNT	
CALL DELAY_FUNCTION
//------------------START--------------------------
		SET 	r30.t7 //Start
//------------------SLEEP 2 SECONDS---------------------------
/*
MOV	r12, DELAYCOUNT	
CALL DELAY_FUNCTION
*/
//---------------------RDATAC----------------------
SEND_RDATAC:
	LBBO	r2, r1, 4, 8	 // Load rdatac command
	SUB r13, r14, 8
	//We only want 8 bits, but 1 register=32 bits and we can only take 1 bit per clock edge,
	//so we do lsl(logical shift left) to move our LSB to MSB positions(i.e. first 8 positions)
	CALL TRUNCATE
	
	CLR	r30.t5		 // set the CS line low (active low)
	MOV	r4, 8		 // going to write/read 8 bits (1 byte)
	CALL	SPICLK_LOOP           // repeat call the SPICLK procedure until all 8 bits written/read

	SET	r30.t5		 // pull the CS line high (end of sample)
//------------------SLEEP 2 SECONDS---------------------------
/*
MOV	r12, DELAYCOUNT	
CALL DELAY_FUNCTION
*/
//--------------------------------------------------------------------------------------------------
//--------------------------------------------------------------------------------------------------
	MOV r16, 0  //Counter of bits received
// Need to wait at this point until it is ready to take a sample
	POLLING_DATA_READY_HIGH: 
		QBBS	POLLING_DATA_READY_HIGH, r31.t16
	
	MOV r21, 0x00000000 //Auxiliary register, to store 32 bits: it's stored in a register and then set to 0 every 32 bits
	MOV r22, 0
	MOV r26, 1

	/*
	//Para evitar coger el 1er byte (FF) que nos llega de la muestra,
	//supuestamente es problema de que el data_ready hace un pico a 1 antes de ponerse a 1 definitivamente
		MOV	r12, 35 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
		CALL DELAY_FUNCTION
		*/
	//--------------------------------------------------------------------------------------
GET_SAMPLE: //Receive 1 sample (18 bytes)
	CLR	r30.t5		 // set the CS line low (active low)
	// Para evitar coger el 1er byte (FF) que nos llega de la muestra, no sabemos por qué
		MOV	r12, 200 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
		CALL DELAY_FUNCTION
		
	//--------------------------------------------------------------------------------------

	MOV r15, 21 //Number of bytes/sample -- 19 Bytes/Sample (basandose en el SPI original por hardware)
	GET_BYTE:
		
		MOV	r4, 8		 // going to write/read 1 byte (8 bits)
		MOV r2, 0x00000000 				//What we want to write (i.e 0)
		CALL	SPICLK_LOOP           // repeat call the SPICLK procedure until all 8 bits written/read		
	
		MOV	r12, 800 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
		CALL DELAY_FUNCTION
				
		SET	r30.t1 //MOSI: 1 (before start taking the sample)
		
		MOV	r12, 800 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
		CALL DELAY_FUNCTION
			
		CLR	r30.t1 //MOSI: 0 (before start taking the sample)

		SUB	r15, r15, 1     // decrement loop counter

	QBNE	GET_BYTE, r15, 0  // repeat loop unless zero
	
	SET	r30.t5		 // pull the CS line high (end of sample)
	SET	r30.t1 //MOSI: 1 (before start taking the sample)
	
	
	LSR	r3, r3, 1        // SPICLK shifts left too many times left, shift right once
	LSR	r17, r17, 1        // SPICLK shifts left too many times left, shift right once
	LSR	r18, r18, 1        // SPICLK shifts left too many times left, shift right once
	LSR	r19, r19, 1        // SPICLK shifts left too many times left, shift right once
	LSR	r20, r20, 1        // SPICLK shifts left too many times left, shift right once
	LSR	r23, r23, 1        // SPICLK shifts left too many times left, shift right once
	LSR	r27, r27, 1        // SPICLK shifts left too many times left, shift right once

STORE_DATA:                      // store the sample value in memory
	SUB	r9, r9, 21	 // reducing the number of bytes - 18 bytes per sample (1 sample = int size)
	
	//Reset Counter of bits written(They need to be clear for next sample to take) into register of their comment:
	MOV	r22, 0 //r3
	MOV	r26, 1	//r20
	
	
/*
	//-----TESTING ONLY (REMOVE)------
	MOV r3,  0x00000000

	MOV r17, 0x00000000
	
	MOV r18, 0x00000000
	MOV r19, 0x00000000
	MOV r20, 0x00000000
	MOV r23, 0x00000000
	//-------------------------------
*/

	
	//-----Store in RAM the whole sample (18 bytes)------
	SBBO	r3, r8, 0, 3	//Last register only needs 2 bytes (It has byte 16 and byte 17) 
	ADD		r8, r8, 3	 	// shifting RAM addres by 2 bytes (1 register = 3bytes, but this one only needs 2 bytes)
	
	MOV	r12, 50 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
	CALL DELAY_FUNCTION
	
	SBBO	r17, r8, 0, 3	
	ADD		r8, r8, 3	

	MOV	r12, 50 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
	CALL DELAY_FUNCTION
	
	SBBO	r18, r8, 0, 3	 
	ADD		r8, r8, 3	 
	
	MOV	r12, 50 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
	CALL DELAY_FUNCTION
	
	SBBO	r19, r8, 0, 3	 
	ADD		r8, r8, 3	
	
	MOV	r12, 50 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
	CALL DELAY_FUNCTION
	
	SBBO	r20, r8, 0, 3	 // store the value r3 in memory (It has byte 0, byte 1, byte 2, and byte 3)
	ADD		r8, r8, 3	 // shifting RAM addres by 3 bytes (1 register = 3bytes)
	
	MOV	r12, 50 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
	CALL DELAY_FUNCTION
	
	SBBO	r23, r8, 0, 3	 // store the value r3 in memory (It has byte 0, byte 1, byte 2, and byte 3)
	ADD		r8, r8, 3	 // shifting RAM addres by 4 bytes (1 register = 4bytes)
	
	MOV	r12, 50 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
	CALL DELAY_FUNCTION
	
	SBBO	r27, r8, 0, 3	 // store the value r3 in memory (It has byte 0, byte 1, byte 2, and byte 3)
	ADD		r8, r8, 3	 // shifting RAM addres by 4 bytes (1 register = 4bytes)
	
	MOV	r12, 50 //Numero aleatorio(se debería calcular cuanto es lo justo) para hacer un sleep de un poco de tiempo
	CALL DELAY_FUNCTION
	
	//============================================= //Comment this section to store everything without limit in RAM(without storing->removin->storing->removing), it'll cause kernel exceptions when full
	/*
	QBLE NOT_REDUCE_SIZE_CHUNK_RAM, SIZE_CHUNK_RAM, r9/18 //samples_left <= size_chunk_ram
		MOV SIZE_CHUNK_RAM, r9/18
		
	NOT_REDUCE_SIZE_CHUNK_RAM:
	*/
	QBNE CHECK_RAM_FULL, r24, SIZE_CHUNK_RAM-1
		// generate an interrupt to notificate a new chunk of samples is ready in RAM to be given to host program(in C)
		MOV R31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_1
	CHECK_RAM_FULL:
	QBNE CONTINUE_THIS_LOOP_RAM, r24, (SIZE_CHUNK_RAM*2)-1
		// generate an interrupt to notificate a new chunk of samples is ready in RAM to be given to host program(in C)
		MOV R31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_1
		MOV r8, r25 //Reset ram direction to points to initial cell of RAM memory(r25)
		MOV r24, 0
		QBA END_PROCESS_RAM_BUFFER
	
	CONTINUE_THIS_LOOP_RAM:
	ADD r24, r24, 1
	//=============================================0
	END_PROCESS_RAM_BUFFER:
	
	
	//------------------------------------------------------
		// clear registers to receive the response from the ADS
	MOV	r3,  0x000000
	MOV	r17, 0x000000	 
	MOV	r18, 0x000000	
	MOV	r19, 0x000000	 
	MOV	r20, 0x000000	
	MOV	r23, 0x000000	
	MOV	r27, 0x000000	

	
	
	//QBGE	END, r9, 0       //Have taken the full set of samples. Comment if we want while(1)
	//It's QBGE( r9 <= 0 ) and not QBEQ (r0 == 0),  just in case we receive more samples than we expect
	
	POLLING_DATA_READY_LOW: //We have already taken the sample(SPI speed faster than sample rate) so we've plenty of time here...
	QBBC	POLLING_DATA_READY_LOW, r31.t16 
	
		
	QBA	POLLING_DATA_READY_HIGH


END:
	//Return all outputs to the original state
	CLR r30.t7
	CLR r30.t0
	SET r30.t5 //CS is active low
	SET r30.t1
	CLR r30.t2
	
	MOV	r31.b0, PRU0_R31_VEC_VALID | PRU_EVTOUT_0	
	HALT                     // End of program -- below are the "procedures"


// This procedure (SPI_CLK 1 BIT) applies an SPI clock cycle to the SPI clock and on the rising edge of the clock
// it writes the current MSB bit in r2 (i.e. r31) to the MOSI pin. On the falling edge, it reads
// the input from MISO and stores it in the LSB of r3. 
// The clock cycle is determined by the datasheet of the product where TIME_CLOCK is the
// time that the clock must remain low and the time it must remain high (assuming 50% duty cycle)
// The input and output data is shifted left on each clock cycle

SPICLK_LOOP: //LOOP through the X bits (read and write)
	/*
	1 sample = 18 bytes(size of int in C + status bytes), so it doesn't fit in 1 register (4 bytes).
	We need to use at least 6 registers 
	We use r16 to count how many bits are received, and r20 as auxiliary register
	Registers to store 1 sample
	r3: 1st register (3 bytes)
	r17: 2nd register (3 bytes)
	r18: 3rd register (3 bytes)
	r19: 4th register (3 bytes)
	r20: 5th register (3 bytes)
	r20: 5th register (3 bytes)
	Only 3 bytes are stored instead of 4 because 4th byte causes an error which we couldn't identify
	*/
	
	SPI_CLK_BIT:
			MOV	r0, r11	 // time for clock low -- assuming clock low before cycle
		CLKLOW:	
			SUB	r0, r0, 1	 // decrement the counter by 1 and loop (next line)
		QBNE	CLKLOW, r0, 0	 // check if the count is still low		
			// Take bit 31 (in the next loop bit 31 will be bit 30 because we do a LSL each loop)
			//So this way we take every bit we want
			QBBC	DATALOW, r2.t31  //QBC = Quick branch if bit clear (if bit=0)
			SET	r30.t1 //The bit that we want is 1, so we send a 1
			QBA	DATACONTD //QBA = Quick branch always
		DATALOW:
			CLR	r30.t1 //The bit that we want is 0, so we send a 0
		DATACONTD:
			SET	r30.t2		 // set the clock high
			MOV	r0, r11	 // time for clock high
		CLKHIGH:
			SUB	r0, r0, 1	 // decrement the counter by 1 and loop (next line)
		QBNE	CLKHIGH, r0, 0	 // check the count
			LSL	r2, r2, 1 //Permorm logical shif left to take the next bit we want to write
						 // clock goes low now -- read the response on MISO
			CLR	r30.t2		 // set the clock low
			
			//----Store 1 received bit in a register (r21)-----
			QBBC	DATAINLOW, r31.t3 //Take the MISO bit we receive from ADS
				OR	r21, r21, 0x000001 //bit received=1,so we set the LSB to 1
			DATAINLOW:	 //bit received=0,so we have nothing to do because we had already set all bits of the register to 0
				LSL	r21, r21, 1 
			//-----------------------------------------	
			
		QBNE END_TAKING_MISO, r26, 1 //Flag para detectar que empieza la toma de datos(antes solo era para congigurar el SPI)
			QBLE STORE_1ST_REGISTER, r22, 23
				QBA BREAK_IF //r22 > 23
					
			STORE_1ST_REGISTER: //Entra aqui si 23 <= r22
				QBNE STORE_2ND_REGISTER, r22, 23 //23 < r22
				MOV r3, r21 //If we wan to check how many bits have been 1, we put here: MOV r3, 23
				
				MOV r21, 0x000000
			//============================================================================================
			QBLE STORE_2ND_REGISTER, r22, (23*2)+1
				QBA BREAK_IF //r22 > 23
					
			STORE_2ND_REGISTER: //Entra aqui si r22 >= 23             23 <= r22
				QBNE STORE_3RD_REGISTER, r22, (23*2)+1 //23 < r22
				MOV r17, r21 //If we wan to check how many bits have been 1, we put here: MOV r3, 23
				
				MOV r21, 0x000000
			//============================================================================================
			QBLE STORE_3RD_REGISTER, r22, (23*3)+2
				QBA BREAK_IF //r22 > 23
					
			STORE_3RD_REGISTER: //Entra aqui si r22 >= 23             23 <= r22
				QBNE STORE_4TH_REGISTER, r22, (23*3)+2 //23 < r22
				MOV r18, r21 //If we wan to check how many bits have been 1, we put here: MOV r3, 23
				
				MOV r21, 0x000000
	
	//============================================================================================
			QBLE STORE_4TH_REGISTER, r22, (23*4)+3
				QBA BREAK_IF //r22 > 23
					
			STORE_4TH_REGISTER: //Entra aqui si r22 >= 23             23 <= r22
				QBNE STORE_5TH_REGISTER, r22, (23*4)+3 //23 < r22
				MOV r19, r21 //If we wan to check how many bits have been 1, we put here: MOV r3, 23
				
				MOV r21, 0x000000
			//============================================================================================
			QBLE STORE_5TH_REGISTER, r22, (23*5)+4
				QBA BREAK_IF //r22 > 23
					
			STORE_5TH_REGISTER: //Entra aqui si r22 >= 23             23 <= r22
				QBNE STORE_6TH_REGISTER, r22, (23*5)+4 //23 < r22
				MOV r20, r21 //If we wan to check how many bits have been 1, we put here: MOV r3, 23
				
				MOV r21, 0x000000
			
			//============================================================================================
			QBLE STORE_6TH_REGISTER, r22, (23*6)+5
				QBA BREAK_IF //r22 > 23
					
			STORE_6TH_REGISTER: //Entra aqui si r22 >= 23             23 <= r22
				QBNE STORE_7TH_REGISTER, r22, (23*6)+5 //23 < r22
				MOV r23, r21 //If we wan to check how many bits have been 1, we put here: MOV r3, 23
				
				MOV r21, 0x000000
			//============================================================================================
			QBLE STORE_7TH_REGISTER, r22, (23*7)+6
				QBA BREAK_IF //r22 > 23
					
			STORE_7TH_REGISTER: //Entra aqui si r22 >= 23             23 <= r22
				QBNE BREAK_IF, r22, (23*7)+6 //23 < r22
				MOV r27, r21 //If we wan to check how many bits have been 1, we put here: MOV r3, 23
				
				MOV r21, 0x000000
			//============================================================================================
			
			BREAK_IF:
				ADD r22, r22, 1	//Entra aqui si r22 < 23
		
		END_TAKING_MISO:
		
		SUB	r4, r4, 1        // count down through the bits
//--------------------------------	
		QBNE	SPICLK_LOOP, r4, 0		
	RET	
		
			
DELAY_FUNCTION:
	SUB	r12, r12, 1     // decrement loop counter
	QBNE	DELAY_FUNCTION, r12, 0  // repeat loop unless zero
	RET
	
TRUNCATE:	
	SUB	r13, r13, 1	 // decrement the counter by 1 and loop (next line)
	LSL	r2, r2, 1
	QBNE	TRUNCATE, r13, 0
	RET