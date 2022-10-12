#include <stdio.h> 
#include <stdlib.h> 
#include <stdint.h>
#include <string.h>

//Represents a line of the source code after initial parsing.
typedef struct LINKED_LINE
{
	char* s_label;			//label portion of source line (if any)
	char* s_line;			//mnemonic or key word portion of source line (always present)
	unsigned long n_line;		//source line number for printing and error mesage use only
	uint8_t name_index;		//identifies the file that contains this line of source code
	unsigned int mnemonic_index;	//number that uniquely identifies a mnemonic or key word
	unsigned int mnemonic_end;	//marks end of mnemonic and start of operands
	struct LINKED_LINE* next;
} linked_line;

typedef struct LINKED_MACRO
{
	linked_line* line_head;
	char* macro_name;
	char* formal_parameters;
	unsigned long n_line;
	uint8_t name_index;
	struct LINKED_MACRO* next;
} linked_macro;

typedef struct LINKED_SOURCE
{
	char* s_label;
	char* s_operands;
	unsigned long n_line;
	uint8_t name_index;
	unsigned int mnemonic_index;
	struct LINKED_SOURCE* next;
} linked_source;

typedef struct LINKED_SOURCE_SEGMENT
{
	unsigned long offset;
	linked_source* source_head;
	struct LINKED_SOURCE_SEGMENT* next;
} linked_source_segment;

typedef struct LINKED_INSTRUCTION
{
	unsigned long address;
	uint8_t instruction_high;
	uint8_t instruction_low;
	struct LINKED_INSTRUCTION* next;
} linked_instruction;

typedef struct LINKED_BINARY_SEGMENT
{
	unsigned long start_address;
	unsigned long end_address;
	linked_instruction* instruction_head;
	struct LINKED_BINARY_SEGMENT* next;
} linked_binary_segment;

const char* mnemonics[] = {"MOVE", "ADD", "AND", "XOR", "XEC", "NZT", "XMIT", "JMP", "CALL", "RET", "NOP", "ORG"};
#define N_MNEMONICS 12
const char include_string[] = "INCLUDE";
const char macro_string[] = "MACRO";
const char endmacro_string[] = "ENDMACRO";
const char equ_string[] = "EQU";
const char high_string[] = "`HIGH";
const char low_string[] = "`LOW";
const char debug_string[] = "-DEBUG";
const char asm_string[] = "-ASM";
const char bin_string[] = "-BIN";
const char coe_string[] = "-COE";
const char mif_string[] = "-MIF";
char include_name_buf[256];
char* name_table[256];
uint8_t debug_enable;
unsigned int asm_index = 0;
unsigned int bin_index = 0;
unsigned int coe_index = 0;
unsigned int mif_index = 0;

void load_file(char* file_name, linked_line* head);
void include_merge(linked_line* prev_node, linked_line* include_node, linked_line* new_head);
void m_move(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head);
void m_nop(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head);
void m_add(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head);
void m_and(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head);
void m_xor(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head);
void m_xec(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head);
void m_nzt(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head);
void m_xmit(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head);
void m_jmp(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head);
void m_call(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head);
void m_ret(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head);
void p_error(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head);
unsigned long regliv_machine_val(char* operand, unsigned long line_num, uint8_t name_index);
void remove_spaces(char* s);
int split_operands(char* operands, char** first, char** second, char** third);
//unsigned long get_label_address(linked_source_segment* source_segment_head, char* s_label, unsigned long line_num, uint8_t name_index);
unsigned long label_or_immediate_value(char* candidate, linked_source_segment* source_segment_head, unsigned long line_num, uint8_t name_index);
void free_node(linked_line* current_node);
void free_macro(linked_macro* current_macro);
void free_source_segment(linked_source_segment* current_source_segment);
void free_source(linked_source* current_source);
void free_binary_segment(linked_binary_segment* current_binary_segment);
void free_instruction(linked_instruction* current_instruction);
unsigned long get_binary_segment_end(linked_binary_segment* current_binary_segment);
unsigned long get_binary_size(linked_binary_segment* binary_segment_head);
void fill_buf(linked_binary_segment* binary_segment_head, uint8_t* buffer);
int str_comp_partial(const char* str1, const char* str2);
void find_and_replace(linked_line* current_node, char* s_replace, char* s_new);
int str_find_word(char* where, char* what, unsigned int* start, unsigned int* end);
void str_replace(char** where, char* s_new, unsigned int word_start, unsigned int word_end);
unsigned int str_size(char* s_input);
void replace_file_extension(char* new_ext, char* out_name, char* new_name);
void write_mif(char* out_name, uint8_t* out_data, unsigned long prg_size);
void write_coe(char* out_name, uint8_t* out_data, unsigned long prg_size);
