# 8X-RIPTIDE_Assembler
Simple assembler for 8X300 and 8X-RIPTIDE CPUs

*Change log:
  *Fixed bug that caused incorrect machine code for XEC instructions.
  *Fixed bug that cause incorrect values for literals represented in decimal.
  *Fixed one of the example files.
*Known issues:
  *When a label is referenced in a segment, the assembler will search for it only within that same segment.
