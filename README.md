# 8X-RIPTIDE_Assembler
Simple assembler for 8X300 and 8X-RIPTIDE CPUs  

* Change log:  
  * Fixed bug that caused incorrect machine code for XEC instructions.  
  * Fixed bug that cause incorrect values for literals represented in decimal.  
  * Fixed one of the example files.  
  * Changed get_label_address to look for labels in all segments.  
  * Changed str_find_word to recognize comas and parentheses as word boundaries.  
  * Added content to example files.  
  * Fixed bug that caused incorrect segment addresses.  
  * Fixed bug that caused the program size to be calculated incorrectly.  
  * Added a function to generate MIF and COE files for Quartus and Vivado.  
  * Removed HIGH and LOW keywords and replaced them with \`HIGH and \`LOW.  
   * Labels can now begin with the words HIGH and LOW, and the new keywords can be applied to literals.  
  * Added Visual C++ project with 32-bit Windows executable.  
  * Added an AMD64 Linux executable.  
  * Updated one of the example files.  
  * Added warning messages for NZT and XEC instructions when the target is out of range.  
  * Updated executables.  
  * Added -DEBUG flag that can be used to enable debug output.  
  * Fixed bug that caused incorrect rotation value for ADD, AND, and XOR instructions.  

* Usage example:  
  * 8xasm test.asm output.bin  
  * 8xasm test.asm output.bin -DEBUG  

The above example will generate not only the specified .bin file, but also a .mif and a .coe file.  
These files are used for initalizing memory in FPGAs, which is useful when working with FPGA implementations of the supported processors.  
