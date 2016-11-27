// Loads an arbitrary .bin file into PRU0 and waits for it to signal  
 // that it has finished.  
 //  
 // Pass in the filename of the .bin file on the command line, eg:  
 // $ ./pru_loader foo.bin  
 //  
 // Compile with:  
 // gcc -o pru_loader pru_loader.c -lprussdrv  
   
 #include <stdio.h>  
 #include <prussdrv.h>  
 #include <pruss_intc_mapping.h>  
   
 int main(int argc, char **argv) {  
  if (argc != 2) {  
   printf("Usage: %s pru_code.bin\n", argv[0]);  
   return 1;  
  }  
   
  // If this segfaults, make sure you're executing as root.  
  prussdrv_init();  
  if (prussdrv_open(PRU_EVTOUT_0) == -1) {  
   printf("prussdrv_open() failed\n");  
   return 1;  
  }  
    
  tpruss_intc_initdata pruss_intc_initdata = PRUSS_INTC_INITDATA;  
  prussdrv_pruintc_init(&pruss_intc_initdata);  
   
  // Change to 1 to use PRU1  
  int which_pru = 0;  
  printf("Executing program and waiting for termination\n");  
  prussdrv_exec_program(which_pru, argv[1]);  
   
  // Wait for the PRU to let us know it's done  
  prussdrv_pru_wait_event(PRU_EVTOUT_0);  
  printf("All done\n");  
   
  prussdrv_pru_disable(which_pru);  
  prussdrv_exit();  
   
  return 0;  
 }  