# 8X-RIPTIDE_Assembler
Simple assembler for 8X300 and 8X-RIPTIDE CPUs  

## Usage example:  
  * 8xasm -asm test.asm -bin output.bin -mif output.mif -coe output.coe  
  * 8xasm -asm test.asm -bin output.bin -mif output.mif -coe output.coe -debug  

The above example will generate the output file in 3 formats: bin , mif, and coe.    
These files are used for initalizing memory in FPGAs, which is useful when working with FPGA implementations of the supported processors.  
