//                     /\         /\__
//                   // \       (  0 )_____/\            __
//                  // \ \     (vv          o|          /^v\
//                //    \ \   (vvvv  ___-----^        /^^/\vv\
//              //  /     \ \ |vvvvv/               /^^/    \v\
//             //  /       (\\/vvvv/              /^^/       \v\
//            //  /  /  \ (  /vvvv/              /^^/---(     \v\
//           //  /  /    \( /vvvv/----(O        /^^/           \v\
//          //  /  /  \  (/vvvv/               /^^/             \v|
//        //  /  /    \( vvvv/                /^^/               ||
//       //  /  /    (  vvvv/                 |^^|              //
//      //  / /    (  |vvvv|                  /^^/            //
//     //  / /   (    \vvvvv\          )-----/^^/           //
//    // / / (          \vvvvv\            /^^^/          //
//   /// /(               \vvvvv\        /^^^^/          //
//  ///(              )-----\vvvvv\    /^^^^/-----(      \\
// //(                        \vvvvv\/^^^^/               \\
///(                            \vvvv^^^/                 //
//                                \vv^/         /        //
//                                             /<______//
//                                            <<<------/
//                                             \<
//                                              \
//**************************************************
//* RIPTIDE_Assembler.c             SOURCE FILE    *
//* Copyright (C) 2019 Esteban Looser-Rojas.       *
//* Contains assembler program for 8X300, 8X305,   *
//* 8X-RIPTIDE, RIPTIDE-II, and RIPTIDE-III CPUs.  *
//**************************************************

#include "RIPTIDE_Assembler.h"
#include "range_sort.h"
#include "label_hash.h"

int main(int argc, char** argv)
{
	linked_line* head = (linked_line*)malloc(sizeof(linked_line));
	linked_line* current_node = head;
    linked_macro* macro_head = (linked_macro*)malloc(sizeof(linked_macro));
	linked_macro* current_macro = macro_head;
	linked_binary_segment* binary_segment_head = (linked_binary_segment*)malloc(sizeof(linked_binary_segment));
	linked_binary_segment* current_binary_segment = binary_segment_head;
	linked_source_segment* source_segment_head = (linked_source_segment*)malloc(sizeof(linked_source_segment));
	linked_source_segment* current_source_segment = source_segment_head;
	linked_source* current_source = NULL;
	linked_instruction* current_instruction = NULL;
	
	for(unsigned int d = 0; d < 256; ++d)
	{
		name_table[d] = NULL;
	}
	
	//initialize linked lists
	head->n_line = 0xffffffff;
	head->s_label = NULL;
	head->s_line = NULL;
	head->next = NULL;

	macro_head->line_head = NULL;
	macro_head->macro_name = NULL;
	macro_head->formal_parameters = NULL;
	macro_head->next = NULL;

	binary_segment_head->start_address = 0xffffffff;
	binary_segment_head->end_address = 0xffffffff;
	binary_segment_head->instruction_head = NULL;
	binary_segment_head->next = NULL;
	
	source_segment_head->offset = 0xffffffff;
	source_segment_head->source_head = NULL;
	source_segment_head->next = NULL;
	
	//Parse program arguments
	unsigned int arg = 1;
	char flag[5];
	if(argc == 1)
	{
		printf("Usage: -ASM <source file> -BIN <output binary> -COE <output coe file> -MIF <output mif file> -DEBUG\n");
		printf("-BIN, -COE, -MIF are optional, but at least one must be specified\n");
		printf("-DEBUG is optional\n");
		exit(1);
	}
	while(arg < (unsigned int)argc)
	{
		if(argv[arg][0] == '-')
		{
			for(unsigned int d = 0; d < 4; ++d)
			{
				flag[d] = argv[arg][d];
				if(flag[d] == 0x00)
					break;
				if((flag[d] > 0x60) & (flag[d] < 0x7b))
					flag[d] = flag[d] - 0x20;
			}
			flag[4] = 0x00;
			if(str_comp_partial(asm_string, flag))
			{
				asm_index = ++arg;
			}
			else if(str_comp_partial(bin_string, flag))
			{
				bin_index = ++arg;
			}
			else if(str_comp_partial(coe_string, flag))
			{
				coe_index = ++arg;
			}
			else if(str_comp_partial(mif_string, flag))
			{
				mif_index = ++arg;
			}
			else if(str_comp_partial(debug_string, flag))
			{
				debug_enable = 0xFF;
			}
			++arg;
		}
		else
		{
			asm_index = arg++;
		}
	}
	if(arg > (unsigned int)argc)
	{
		printf("Invalid arguments!\n");
		exit(1);
	}
	if(asm_index == 0)
	{
		printf("No source file specified!\n");
		exit(1);
	}
	if(bin_index == 0 && coe_index == 0 && mif_index == 0)
	{
		printf("No output file specified!\n");
		exit(1);
	}

	//Load source file
	load_file(argv[asm_index], head);
	
	//Replace includes with source
	linked_line* new_head = (linked_line*)malloc(sizeof(linked_line));
	linked_line* prev_node = NULL;
	new_head->n_line = 0xffffffff;
	new_head->s_label = NULL;
	new_head->s_line = NULL;
	while(1)
	{
		prev_node = current_node;
		current_node = current_node->next;
		if(current_node == NULL)
			break;
		if(current_node->s_label == NULL)
			continue;
		if (str_comp_partial(current_node->s_label, include_string))
		{
			char* line = current_node->s_line;
	
			unsigned int start = 0;
			unsigned int end = 0;
			for(unsigned int i = 0; line[i]; i++)
			{
				if (line[i] == '"')
				{
					start = i+1;
					break;
				}
			}
			for(unsigned int i = start; line[i]; i++)
			{
				if (line[i] == '"')
				{
					end = i-1;
					break;
				}
			}
			
			// Didn't find two quotes
			if (end == 0)
			{
				fprintf(stderr, "Syntax error in Include statement in file %s at line: %lu\n", name_table[current_node->name_index], current_node->n_line);
				exit(1);
			}
			
			unsigned int buf_i = 0;
			for(unsigned int i = start; i <= end; i++)
			{
				include_name_buf[buf_i] = line[i];
				++buf_i;
			}
			include_name_buf[buf_i]='\0';

			load_file(include_name_buf, new_head);
			include_merge(head, current_node, new_head);
			free(current_node->s_label);
			free(current_node->s_line);
			free(current_node);
			current_node = prev_node;
		}
	}
	free(new_head);

	//find and replace defines
	current_node = head;
	while(1)
	{
		prev_node = current_node;
		current_node = current_node->next;
		if(current_node == NULL)
			break;
		if(str_comp_partial(current_node->s_line, equ_string))
		{
			find_and_replace(current_node->next, current_node->s_label, current_node->s_line + 3);	//s_new may have trailing and leading spaces
			prev_node->next = current_node->next;
			free(current_node->s_label);
			free(current_node->s_line);
			free(current_node);
			current_node = prev_node;
		}
	}
	
    //build list of macros
	linked_line* prev_macro_line = NULL;
	linked_line* current_macro_line = NULL;
	current_node = head;
	while(1)
	{
		prev_node = current_node;
		current_node = current_node->next;
		if(current_node == NULL)
			break;
		if(current_node->s_label && str_comp_partial(current_node->s_label, macro_string))	//found start of a macro
		{
			//create new macro node
			current_macro->next = (linked_macro*)malloc(sizeof(linked_macro));
			current_macro = current_macro->next;
			current_macro->next = NULL;
			//fill line number and file fields
			current_macro->n_line = current_node->n_line;
			current_macro->name_index = current_node->name_index;
			//fill macro name and formal parameters fields
			current_macro->macro_name = current_node->s_line;
			char* formal_parameters = current_macro->macro_name;
			while(1)	//find first space after macro name to terminate the name string
			{
				if(*formal_parameters == 0x00)
				{
					formal_parameters = NULL;	//found null before any parameters...
					break;
				}
				if(*formal_parameters == ' ' || *formal_parameters == '\t')
				{
					*formal_parameters = 0x00;	//terminate macro name
					++formal_parameters;
					while(1)	//look for the start of the formal parameters
					{
						if(*formal_parameters == 0x00)
						{
							formal_parameters = NULL;	//no parameters
							break;
						}
						if(*formal_parameters != ' ' && *formal_parameters != '\t')	//found start of parameters
						{
							break;
						}
						++formal_parameters;
					}
					break;
				}
				++formal_parameters;
			}
			current_macro->formal_parameters = formal_parameters;
			//create dummy line head
			current_macro->line_head = (linked_line*)malloc(sizeof(linked_line));
			current_macro_line = current_macro->line_head;
			current_macro_line->name_index = 0;
			current_macro_line->s_label = NULL;
			current_macro_line->s_line = NULL;
			//attach lines to macro
			current_macro_line->next = current_node->next;
			//find end of macro
			while(1)
			{
				prev_macro_line = current_macro_line;
				current_macro_line = current_macro_line->next;
				if(current_macro_line == NULL)
				{
					printf("Could not find end of macro at line %lu in file %s\n", current_macro->n_line, name_table[current_macro->name_index]);
					exit(1);
				}
				if(current_macro_line->s_label && str_comp_partial(current_macro_line->s_label, endmacro_string))	//found end of macro
				{
					//terminate macro line list and detach from main list
					prev_node->next = current_macro_line->next;
					prev_macro_line->next = NULL;
					//free macro start and end
					//for the macro start we still need the line and next, since those now belong to the macro
					free(current_node->s_label);
					free(current_node);
					current_node = prev_node;
					//for the macro end we only need the next, since it still forms part of the main list
					free(current_macro_line->s_label);
					free(current_macro_line->s_line);
					free(current_macro_line);
					break;
				}
			}
		}
	}

	//print macros
	if(debug_enable)
	{
		current_macro = macro_head;
		while(1)
		{
			current_macro = current_macro->next;
			if(current_macro == NULL)
				break;
			printf("%lu ", current_macro->n_line);
			printf("MACRO ");
			printf("    %s ", current_macro->macro_name);
			printf("%s\n", current_macro->formal_parameters);
			current_node = current_macro->line_head;
			while(1)
			{
				current_node = current_node->next;
				if(current_node == NULL)
					break;
				printf("%lu    ", current_node->n_line);
				if(current_node->s_label)
					printf("%s", current_node->s_label);
				printf("    %s\n", current_node->s_line);
			}
		}
	}

	//expand macros
	linked_line* macro_copy_line_head = NULL;
	linked_line* current_macro_copy_line = NULL;
	current_macro = macro_head;
	while(1)
	{
		current_macro = current_macro->next;
		if(current_macro == NULL)
			break;
		char* macro_name = current_macro->macro_name;
		char* formal_params_copy = NULL;
		if(current_macro->formal_parameters)
			formal_params_copy = (char*)malloc(str_size(current_macro->formal_parameters));	//will need copy of macro params
		//find macro instances
		current_node = head;
		while(1)
		{
			prev_node = current_node;
			current_node = current_node->next;
			if(current_node == NULL)
				break;
			char* s_line = current_node->s_line;	//we can modify this only if it is a macro instance
			if(str_comp_word(s_line, macro_name))
			{
				printf("Found instance of macro %s in line %lu of file %s\n", macro_name, current_node->n_line, name_table[current_node->name_index]);
				//create copy of macro code
				macro_copy_line_head = (linked_line*)malloc(sizeof(linked_line));
				current_macro_copy_line = macro_copy_line_head;
				current_macro_line = current_macro->line_head;
				while(1)
				{
					current_macro_line = current_macro_line->next;
					if(current_macro_line == NULL)
						break;
					//create new line
					current_macro_copy_line->next = (linked_line*)malloc(sizeof(linked_line));
					current_macro_copy_line = current_macro_copy_line->next;
					//copy line data
					current_macro_copy_line->s_line = (char*)malloc(str_size(current_macro_line->s_line));
					strcpy(current_macro_copy_line->s_line, current_macro_line->s_line);
					current_macro_copy_line->s_label = NULL;
					if(current_macro_line->s_label)
					{
						current_macro_copy_line->s_label = (char*)malloc(str_size(current_macro_line->s_label));
						strcpy(current_macro_copy_line->s_label, current_macro_line->s_label);
					}
					current_macro_copy_line->n_line = current_macro_line->n_line;
					current_macro_copy_line->name_index = current_macro_line->name_index;
					current_macro_copy_line->mnemonic_index = current_macro_line->mnemonic_index;
					current_macro_copy_line->mnemonic_end = current_macro_line->mnemonic_index;
					current_macro_copy_line->next = NULL;
				}
				//find and replace formal parameters
				if(current_macro->formal_parameters)	//the thing might not have any parameters
				{
					strcpy(formal_params_copy, current_macro->formal_parameters);
					remove_spaces(formal_params_copy);
					char* current_macro_param = formal_params_copy;	//this is what we want to replace
					char* current_macro_param_end = current_macro_param;
					char* current_instance_param = s_line;	//this is what we want to replace it with
					char* current_instance_param_end = current_instance_param;
					while(*current_instance_param_end)	//the first thing in the line is the macro name, skip
					{
						if(*current_instance_param_end == ' ')
						{
							*current_instance_param_end = 0x00;
							++current_instance_param_end;
							break;
						}
						++current_instance_param_end;
					}
					current_instance_param = current_instance_param_end;
					while(*current_macro_param && *current_instance_param)
					{
						//find end of current_macro_param
						while(*current_macro_param_end)
						{
							if(*current_macro_param_end == ',')
							{
								*current_macro_param_end = 0x00;
								++current_macro_param_end;
								break;
							}
							++current_macro_param_end;
						}
						while(*current_instance_param_end)
						{
							if(*current_instance_param_end == ',')
							{
								*current_instance_param_end = 0x00;
								++current_instance_param_end;
								break;
							}
							++current_instance_param_end;
						}
						find_and_replace(macro_copy_line_head->next, current_macro_param, current_instance_param);
						current_instance_param = current_instance_param_end;	//advance to next instance param
						current_macro_param = current_macro_param_end;	//advance to next macro param (if any)
					}
					//check that instance has the right number of params
					if(*current_macro_param || *current_instance_param)
					{
						printf("Warning: Macro instance at line %lu in file %s does not match macro definition.\n", current_node->n_line, name_table[current_node->name_index]);
					}
				}
				//attach macro copy to main list
				prev_node->next = macro_copy_line_head->next;
				current_macro_copy_line->next = current_node->next;
				//free copy line head
				free(macro_copy_line_head);
				//keep macro reference label (if any)
				if(current_node->s_label)
                {
                    if(!prev_node->next->s_label)
                        prev_node->next->s_label = current_node->s_label;
                    else
                        free(current_node->s_label);
                }
                //free current node
				free(current_node->s_line);
				free(current_node);
				current_node = prev_node;
			}
		}
		//free formal_params_copy
		free(formal_params_copy);
	}

	//free macros
	current_macro = macro_head;
	free_macro(current_macro);
		
	//print lines
	if(debug_enable)
	{
		current_node = head;
		while(1)
		{
			current_node = current_node->next;
			if(current_node == NULL)
				break;
			printf("%lu ", current_node->n_line);
			if(current_node->s_label)
				printf("%s", current_node->s_label);
			printf("\t%s\n", current_node->s_line);
		}
	}

	//complete the node list
	current_node = head;
	while(1)
	{
		current_node = current_node->next;
		if(current_node == NULL)
			break;
		//get mnemonic index
		for(unsigned int d = 0; d <= N_MNEMONICS; ++d)
		{
			if(d == N_MNEMONICS)
			{
				printf("Invalid mnemonic or key word: %s in line: %lu in file: %s\n", current_node->s_line, current_node->n_line, name_table[current_node->name_index]);
				exit(1);
			}
			for(unsigned int i = 0; d < N_MNEMONICS; ++i)
			{
				if(mnemonics[d][i] == 0x00)
				{
					current_node->mnemonic_index = d;
					current_node->mnemonic_end = i;
					d = 0x7FFF;	//break out of both loops
					break;
				}
				if(current_node->s_line[i] != mnemonics[d][i])
					break;
			}
		}
	}

	//create linked source segments
	current_node = head->next;
	current_source_segment = source_segment_head;
	unsigned int num_labels = 0;
	while(1)
	{
		if(current_node == NULL)
			break;
		//find the start of the segment
		if(current_node->mnemonic_index != 11)
		{
			printf("Code before ORG in line %lu in file %s\n", current_node->n_line, name_table[current_node->name_index]);
			exit(1);
		}
		if(!(current_node->next))
			continue;
		if(current_node->next->mnemonic_index == 11)	//empty segment
			continue;
		//create new segment
		current_source_segment->next = (linked_source_segment*)malloc(sizeof(linked_source_segment));
		current_source_segment = current_source_segment->next;
		current_source_segment->next = NULL;
		//get offset
		for(unsigned int d = 0; 1; ++d)
		{
			if(current_node->s_line[current_node->mnemonic_end + d] == 0x00)
			{
				printf("invalid operand in line %lu in file %s\n", current_node->n_line, name_table[current_node->name_index]);
				exit(1);
			}
			if(current_node->s_line[current_node->mnemonic_end + d] == '@')	//octal
			{
				current_source_segment->offset = (unsigned long)strtol((current_node->s_line + current_node->mnemonic_end + d + 1), NULL, 8);
				break;
			}
			if(current_node->s_line[current_node->mnemonic_end + d] == '$')	//hexadecimal
			{
				current_source_segment->offset = (unsigned long)strtol((current_node->s_line + current_node->mnemonic_end + d + 1), NULL, 16);
				break;
			}
			if(current_node->s_line[current_node->mnemonic_end + d] == '%')	//binary
			{
				current_source_segment->offset = (unsigned long)strtol((current_node->s_line + current_node->mnemonic_end + d + 1), NULL, 2);
				break;
			}
			if((0x30 <= (current_node->s_line[current_node->mnemonic_end + d])) && (0x39 >= (current_node->s_line[current_node->mnemonic_end + d])))	//decimal
			{
				current_source_segment->offset = (unsigned long)strtol((current_node->s_line + current_node->mnemonic_end + d), NULL, 10);
				break;
			}
		}
		//create source head
		current_source_segment->source_head = (linked_source*)malloc(sizeof(linked_source));
		current_source = current_source_segment->source_head;
		current_source->mnemonic_index = 0xffff;
		current_source->n_line = 0xffffffff;
		current_source->next = NULL;
		current_source->s_label = NULL;
		current_source->s_operands = NULL;
		//create source nodes
		while(1)
		{
			current_node = current_node->next;
			if(current_node == NULL)
				break;
			if(current_node->mnemonic_index == 11)	//org marks start of next segment and end of current segment
				break;
			current_source->next = (linked_source*)malloc(sizeof(linked_source));
			current_source = current_source->next;
			current_source->mnemonic_index = current_node->mnemonic_index;
			current_source->n_line = current_node->n_line;
			current_source->name_index = current_node->name_index;
			current_source->next = NULL;
			if(current_node->s_label)
			{
				current_source->s_label = (char*)malloc(sizeof(char) * (str_size(current_node->s_label)));
				strcpy(current_source->s_label, current_node->s_label);
				++num_labels;
			}
			else
				current_source->s_label = NULL;
			current_source->s_operands = (char*)malloc(sizeof(char) * (str_size(current_node->s_line + current_node->mnemonic_end)));
			strcpy(current_source->s_operands, current_node->s_line + current_node->mnemonic_end);
		}
	}

	//free memory
	current_node = head;
	free_node(current_node);

	//build label map
	build_label_map(source_segment_head, num_labels);

	//create linked binary segments
	current_source_segment = source_segment_head;
	current_binary_segment = binary_segment_head;
	while(1)
	{
		current_source_segment = current_source_segment->next;
		if(current_source_segment == NULL)
			break;
		//create binary segment
		current_binary_segment->next = (linked_binary_segment*)malloc(sizeof(linked_binary_segment));
		current_binary_segment = current_binary_segment->next;
		current_binary_segment->next = NULL;
		current_binary_segment->start_address = current_source_segment->offset;
		//create linked instruction head
		current_instruction = (linked_instruction*)malloc(sizeof(linked_instruction));
		current_binary_segment->instruction_head = current_instruction;
		//convert source into machine code
		current_source = current_source_segment->source_head;
		for(unsigned long d = (current_source_segment->offset) * 2; 1; d = d + 2)
		{
			current_source = current_source->next;
			if(current_source == NULL)
				break;
			//create instruction
			current_instruction->next = (linked_instruction*)malloc(sizeof(linked_instruction));
			current_instruction = current_instruction->next;
			current_instruction->next = NULL;
			current_instruction->address = d;	//this is the byte address, not the word address
			if(debug_enable && current_source->s_label)
				printf("%s\n", current_source->s_label);
			switch(current_source->mnemonic_index)
			{
				case 0:	//MOVE
					m_move(current_source, current_instruction, source_segment_head);
					break;
				case 1:	//ADD
					m_add(current_source, current_instruction, source_segment_head);
					break;
				case 2:	//AND
					m_and(current_source, current_instruction, source_segment_head);
					break;
				case 3:	//XOR
					m_xor(current_source, current_instruction, source_segment_head);
					break;
				case 4:	//XEC
					m_xec(current_source, current_instruction, source_segment_head);
					break;
				case 5:	//NZT
					m_nzt(current_source, current_instruction, source_segment_head);
					break;
				case 6:	//XMIT
					m_xmit(current_source, current_instruction, source_segment_head);
					break;
				case 7:	//JMP
					m_jmp(current_source, current_instruction, source_segment_head);
					break;
				case 8:	//CALL
					m_call(current_source, current_instruction, source_segment_head);
					break;
				case 9:	//RET
					m_ret(current_source, current_instruction, source_segment_head);
					break;
				case 10:	//NOP
					m_nop(current_source, current_instruction, source_segment_head);
					break;
				default:
					p_error(current_source, current_instruction, source_segment_head);
					break;
			}//end case
		}//end for
	}//end while
	
	//free source segments
	current_source_segment = source_segment_head;
	free_source_segment(current_source_segment);

	//free label map
	free_label_map();

	//complete binary segments
	current_binary_segment = binary_segment_head;
	unsigned int num_segments = 0;
	while(1)
	{
		current_binary_segment = current_binary_segment->next;
		if(current_binary_segment == NULL)
			break;
		current_binary_segment->start_address = current_binary_segment->instruction_head->next->address;
		current_binary_segment->end_address = get_binary_segment_end(current_binary_segment);
		++num_segments;
	}

	//free name table
	for(unsigned int d = 0; d < 256; ++d)
	{
		if(name_table[d] == NULL)
			break;
		free(name_table[d]);
	}

	//check for segment overlap
	build_range_table(binary_segment_head, num_segments);
	print_range_table(num_segments);
	check_range_table(num_segments);
	free_range_table();

	//write binary file
	unsigned long prg_size = get_binary_size(binary_segment_head);
	printf("Prog size: %lu\n", prg_size);
	uint8_t* output_arr = (uint8_t*)malloc(prg_size);

	for(unsigned long d = 0; d < prg_size; ++d)
	{
		output_arr[d] = 0xFF;
	}

	fill_buf(binary_segment_head, output_arr);
	free_binary_segment(binary_segment_head);
	
	if(bin_index)
	{
		unsigned long written = 0;
		FILE *f = fopen(argv[bin_index], "wb");
		while (written < prg_size)
		{
			written += (unsigned long)fwrite(output_arr + written, sizeof(uint8_t), prg_size - written, f);
			if (written == 0)
			{
				printf("Error writing output file!\n");
			}
		}
		fclose(f);
	}
	if(mif_index)
		write_mif(argv[mif_index], output_arr, prg_size);
	if(coe_index)
		write_coe(argv[coe_index], output_arr, prg_size);
	free(output_arr);
	return 0;
}

void load_file(char* file_name, linked_line* head)
{
	linked_line* current_node = head;
	unsigned int line_start = 0;
	unsigned int line_end = 0;
	unsigned int label_end;
	unsigned long current_source_line = 0;
	char* floating_label = (char*)malloc(sizeof(int) * 128);
	unsigned int floating_label_size = 0;
	char* current_line = (char*)malloc(sizeof(char) * 256);
	uint8_t name_index = 0;
	
	while(name_table[name_index])
	{
		++name_index;
	}
	name_table[name_index] = (char*)malloc(str_size(file_name));
	strcpy(name_table[name_index], file_name);

	FILE* fp_in = fopen(file_name, "r");
	if(fp_in == NULL)
	{
		printf("Failed to open input file: %s\n", file_name);
		exit(1);
	}
	while (fgets(current_line, 256, fp_in))
	{
		++current_source_line;
		//convert to all caps
		for(int i = 0; i < 256; ++i)
		{
			if(current_line[i] == 0x00)
				break;
			if((current_line[i] > 0x60) & (current_line[i] < 0x7b))
				current_line[i] = current_line[i] - 0x20;
		}
		if(current_line[0] != ';')
		{
			if((current_line[0] > 0x40) && (current_line[0] < 0x5b))	//alpha char
			{
				//line contains a label
				//extract label
				label_end = 0;
				while((current_line[label_end] != 0x20) && (current_line[label_end] != 0x0a) && (current_line[label_end] != 0x09) && (current_line[label_end] != 0x0d) && (current_line[label_end] != 0x00))
				{
					++label_end;
					if(label_end == 128)
					{
						printf("syntax error in line %lu in file %s\n", current_source_line, name_table[name_index]);
						exit(1);
					}
				}
				//extract rest of the line
				line_start = label_end;
				line_end = 0;
				//find line start
				while(((current_line[line_start] < 0x41) || (current_line[line_start] > 0x5A)) && (current_line[line_start] != 0x22))
				{
					if((current_line[line_start] == ';') || (current_line[line_start] == 0x0a) || (current_line[line_start] == 0x0D) || (current_line[line_start] == 0x00))
					{
						//line is only a label
						line_start = 0;
						break;
					}
					++line_start;
					if(line_start == 256)
					{
						printf("syntax error in line %lu in file %s\n", current_source_line, name_table[name_index]);
						exit(1);
					}
				}
				if(line_start == 0)
				{
					//line is only a label
					for(unsigned int i = 0; i < label_end; ++i)
					{
						floating_label[i] = current_line[i];
					}
					floating_label[label_end] = 0x00;
					floating_label_size = label_end + 1;
				}
				else
				{
					//line has code
					//find line end
					line_end = line_start;
					while((current_line[line_end] != ';') && (current_line[line_end] != 0x0a) && (current_line[line_end] != 0x0D) && (current_line[line_end] != 0x00))
					{
						++line_end;
						if(line_end == 256)
						{
							printf("syntax error in line %lu in file %s\n", current_source_line, name_table[name_index]);
							exit(1);
						}
					}
					//create new node
					floating_label_size = 0;	//this line has its own label
					current_node->next = (linked_line*)malloc(sizeof(linked_line));
					current_node = current_node->next;
					current_node->next = NULL;
					current_node->n_line = current_source_line;
					current_node->name_index = name_index;
					current_node->s_label = (char*)malloc(sizeof(char) * (label_end + 1));
					current_node->s_line = (char*)malloc(sizeof(char) * (line_end + 1 - line_start));
					for(unsigned int i = 0; i < label_end; ++i)
					{
						current_node->s_label[i] = current_line[i];
					}
					current_node->s_label[label_end] = 0x00;
					for(unsigned int i = 0; i < (line_end - line_start); ++i)
					{
						current_node->s_line[i] = current_line[i + line_start];
					}
					current_node->s_line[line_end - line_start] = 0x00;
				}
			}
			else if((current_line[0] == 0x20) || (current_line[0] == 0x09))
			{
				//line has no label
				line_start = 0;
				line_end = 0;
				//find line start
				while((current_line[line_start] < 0x41) || (current_line[line_start] > 0x5A))
				{
					if((current_line[line_start] == ';') || (current_line[line_start] == 0x0a) || (current_line[line_start] == 0x0D) || (current_line[line_start] == 0x00))
					{
						//line is empty
						line_start = 0;
						break;
					}
					++line_start;
					if(line_start == 256)
					{
						printf("syntax error in line %lu in file %s\n", current_source_line, name_table[name_index]);
						exit(1);
					}
				}
				if(line_start != 0)
				{
					//line is not empty
					//find line end
					line_end = line_start;
					while((current_line[line_end] != ';') && (current_line[line_end] != 0x0a) && (current_line[line_end] != 0x0D) && (current_line[line_end] != 0x00))
					{
						++line_end;
						if(line_end == 256)
						{
							printf("syntax error in line %lu in file %s\n", current_source_line, name_table[name_index]);
							exit(1);
						}
					}
					//create new node
					current_node->next = (linked_line*)malloc(sizeof(linked_line));
					current_node = current_node->next;
					current_node->next = NULL;
					current_node->n_line = current_source_line;
					current_node->name_index = name_index;
					if(floating_label_size)
					{
						current_node->s_label = (char*)malloc(sizeof(char) * floating_label_size);
						for(unsigned int i = 0; i < floating_label_size; ++i)
						{
							current_node->s_label[i] = floating_label[i];
						}
					}
					else
						current_node->s_label = NULL;
					current_node->s_line = (char*)malloc(sizeof(char) * (line_end + 1 - line_start));
					for(unsigned int i = 0; i < (line_end - line_start); ++i)
					{
						current_node->s_line[i] = current_line[i + line_start];
					}
					current_node->s_line[line_end - line_start] = 0x00;
					floating_label_size = 0;	//floating label has been used
				}
			}
			else if((current_line[0] != ';') && (current_line[0] != 0x0A) && (current_line[0] != 0x0D) && (current_line[0] != 0x00))
			{
				printf("syntax error in line %lu in file %s\n", current_source_line, name_table[name_index]);
				exit(1);
			}
		}
	}
	free(floating_label);
	free(current_line);
	fclose(fp_in);
}

void include_merge(linked_line* prev_node, linked_line* include_node, linked_line* new_head)
{
	linked_line* current_node = new_head;
	prev_node->next = new_head->next;
	while(1)
	{
		if(current_node->next == NULL)
		{
			current_node->next = include_node->next;
			break;
		}
		current_node = current_node->next;
	}
}

void m_move(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head)
{
	remove_spaces(current_source->s_operands);
	char* first = NULL;
	char* second = NULL;
	char* third = NULL;
	unsigned long source;
	unsigned long dest;
	unsigned long rotate;

	int n_operands = split_operands(current_source->s_operands, &first, &second, &third);

	if (n_operands < 2 || n_operands > 3)
	{
		fprintf(stderr, "Invalid number of operands in MOV instruction at line: %lu in file %s\n", current_source->n_line, name_table[current_source->name_index]);
		exit(1);
	}
	if(n_operands == 2)	//register to register or register to IV bus address
	{
		unsigned int len_first = (unsigned int)strlen(first);
		
		source = regliv_machine_val(first, current_source->n_line, current_source->name_index);
		dest = regliv_machine_val(second, current_source->n_line, current_source->name_index);
		rotate = 0;
		
		if (first[len_first - 1] == ')') 	// Rotate specified
		{
			rotate = strtol(first + len_first - 2, NULL, 10) & 7;
		}	
	}
	else	//register to IV bus, IV bus to register, or IV bus to IV bus
	{
		source = regliv_machine_val(first, current_source->n_line, current_source->name_index);
		dest = regliv_machine_val(third, current_source->n_line, current_source->name_index);
		rotate = strtol(second, NULL, 10) & 7;
	}
	
	current_instruction->instruction_high = source & 0x1F;
	current_instruction->instruction_low = (rotate << 5) | (dest & 0x1F);
	//debug output
	if(debug_enable)
	{
		printf("%lX\t%X %X\tMOVE\t%s %s ", current_instruction->address >> 1, current_instruction->instruction_high, current_instruction->instruction_low, first, second);
		if(third)
			printf("%s\n", third);
		else
			printf("\n");
	}
}

void m_nop(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head)
{
	current_instruction->instruction_high = 0x00;
	current_instruction->instruction_low = 0x00;
	//debug output
	if(debug_enable)
		printf("%lX\t%X %X\tNOP\n", current_instruction->address >> 1, current_instruction->instruction_high, current_instruction->instruction_low);
}

void m_add(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head)
{
	remove_spaces(current_source->s_operands);
	char* first = NULL;
	char* second = NULL;
	char* third = NULL;
	unsigned long source;
	unsigned long dest;
	unsigned long rotate;

	int n_operands = split_operands(current_source->s_operands, &first, &second, &third);

	if (n_operands < 2 || n_operands > 3)
	{
		fprintf(stderr, "Invalid number of operands in ADD instruction at line: %lu in file %s\n", current_source->n_line, name_table[current_source->name_index]);
		exit(1);
	}
	if(n_operands == 2)	//register to register or register to IV bus address
	{
		unsigned int len_first = (unsigned int)strlen(first);
	
		source = regliv_machine_val(first, current_source->n_line, current_source->name_index);
		dest = regliv_machine_val(second, current_source->n_line, current_source->name_index);
		rotate = 0;
		
		if (first[len_first - 1] == ')') 	// Rotate specified
		{
			rotate = (unsigned long)strtol(first + len_first - 2, NULL, 10) & 7;
		}	
	}
	else	//register to IV bus, IV bus to register, or IV bus to IV bus
	{
		source = regliv_machine_val(first, current_source->n_line, current_source->name_index);
		dest = regliv_machine_val(third, current_source->n_line, current_source->name_index);
		rotate = strtol(second, NULL, 10) & 7;
	}
	
	current_instruction->instruction_high = (source & 0x1F) | 0x20;
	current_instruction->instruction_low = (rotate << 5) | (dest & 0x1F);
	//debug output
	if(debug_enable)
		printf("%lX\t%X %X\tADD\t%s %s %s\n", current_instruction->address >> 1, current_instruction->instruction_high, current_instruction->instruction_low, first, second, third);
}

void m_and(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head)
{
	remove_spaces(current_source->s_operands);
	char* first = NULL;
	char* second = NULL;
	char* third = NULL;
	unsigned long source;
	unsigned long dest;
	unsigned long rotate;

	int n_operands = split_operands(current_source->s_operands, &first, &second, &third);

	if (n_operands < 2 || n_operands > 3)
	{
		fprintf(stderr, "Invalid number of operands in AND instruction at line: %lu in file %s\n", current_source->n_line, name_table[current_source->name_index]);
		exit(1);
	}
	if(n_operands == 2)	//register to register or register to IV bus address
	{
		unsigned int len_first = (unsigned int)strlen(first);
	
		source = regliv_machine_val(first, current_source->n_line, current_source->name_index);
		dest = regliv_machine_val(second, current_source->n_line, current_source->name_index);
		rotate = 0;
		
		if (first[len_first - 1] == ')') 	// Rotate specified
		{
			rotate = (unsigned long)strtol(first + len_first - 2, NULL, 10) & 7;
		}	
	}
	else	//register to IV bus, IV bus to register, or IV bus to IV bus
	{
		source = regliv_machine_val(first, current_source->n_line, current_source->name_index);
		dest = regliv_machine_val(third, current_source->n_line, current_source->name_index);
		rotate = strtol(second, NULL, 10) & 7;
	}
	
	current_instruction->instruction_high = (source & 0x1F) | 0x40;
	current_instruction->instruction_low = (rotate << 5) | (dest & 0x1F);
	//debug output
	if(debug_enable)
		printf("%lX\t%X %X\tAND\t%s %s %s\n", current_instruction->address >> 1, current_instruction->instruction_high, current_instruction->instruction_low, first, second, third);
}

void m_xor(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head)
{
	remove_spaces(current_source->s_operands);
	char* first = NULL;
	char* second = NULL;
	char* third = NULL;
	unsigned long source;
	unsigned long dest;
	unsigned long rotate;

	int n_operands = split_operands(current_source->s_operands, &first, &second, &third);

	if (n_operands < 2 || n_operands > 3)
	{
		fprintf(stderr, "Invalid number of operands in XOR instruction at line: %lu in file %s\n", current_source->n_line, name_table[current_source->name_index]);
		exit(1);
	}
	if(n_operands == 2)	//register to register or register to IV bus address
	{
		unsigned int len_first = (unsigned int)strlen(first);
	
		source = regliv_machine_val(first, current_source->n_line, current_source->name_index);
		dest = regliv_machine_val(second, current_source->n_line, current_source->name_index);
		rotate = 0;
		
		if (first[len_first - 1] == ')') 	// Rotate specified
		{
			rotate = (unsigned long)strtol(first + len_first - 2, NULL, 10) & 7;
		}	
	}
	else	//register to IV bus, IV bus to register, or IV bus to IV bus
	{
		source = regliv_machine_val(first, current_source->n_line, current_source->name_index);
		dest = regliv_machine_val(third, current_source->n_line, current_source->name_index);
		rotate = strtol(second, NULL, 10) & 7;
	}
	
	current_instruction->instruction_high = (source & 0x1F) | 0x60;
	current_instruction->instruction_low = (rotate << 5) | (dest & 0x1F);
	//debug output
	if(debug_enable)
		printf("%lX\t%X %X\tXOR\t%s %s %s\n", current_instruction->address >> 1, current_instruction->instruction_high, current_instruction->instruction_low, first, second, third);
}

void m_xec(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head)
{
	char* operands = current_source->s_operands;
	unsigned int operands_len = (unsigned int)strlen(operands);
	char* insides = NULL;
	char* size = NULL;
	unsigned int offset = 0;
	unsigned long literal_val;
	unsigned int l_field;
	unsigned int reg_value;

	remove_spaces(operands);
	for(int i = 0; i < operands_len; i++){
		if (operands[i] == '(')
		{
			operands[i] = '\0';
			for(int j = i + 1; j < operands_len; j++){
				if (operands[j] == ')') { 
					operands[j] = '\0';
					insides = operands + i + 1;
					size = operands + j + 1;
					break;
				}
			}
			break;
		}
	}

	if (insides == NULL)
	{
		printf("Syntax error parsing operands of xec instruction at line: %lu in file %s\n", current_source->n_line, name_table[current_source->name_index]);
		exit(1);
	}

	if(size[0])
	{
		if(size[0] != '[')
		{
			printf("Syntax error parsing range of xec instruction at line %lu in file %s\n", current_source->n_line, name_table[current_source->name_index]);
			exit(1);
		}
		size = size + 1;
		offset = atoi(size) - 1;
	}
	
	char* first = strtok(insides, ",");
	if (!first) 
	{
		printf("Error parsing operands of xec instruction at line: %lu in file %s\n", current_source->n_line, name_table[current_source->name_index]);
		exit(1);
	}
	char* second = strtok(NULL, ",");

	literal_val = label_or_immediate_value(operands, source_segment_head, current_source->n_line, current_source->name_index);
	reg_value = regliv_machine_val(first, current_source->n_line, current_source->name_index);
	current_instruction->instruction_high = 0x80 | reg_value;	
	if(second)	//uses IV as source
	{
		l_field = atoi(second);
		current_instruction->instruction_low = ((uint8_t)l_field << 5) | ((uint8_t)literal_val & 0x1F);
		if(((current_instruction->address >> 6) != (literal_val >> 5)) || ((literal_val >> 5) != ((literal_val + offset) >> 5)))
			printf("Warning: XEC instruction at line %lu in file %s cannot jump over 32 word boundary!\n", current_source->n_line, name_table[current_source->name_index]);
	}
	else	//uses register as source
	{
		current_instruction->instruction_low = (uint8_t)(literal_val & 0xff);
		if(((current_instruction->address >> 9) != (literal_val >> 8)) || ((literal_val >> 8) != ((literal_val + offset) >> 8)))
			printf("Warning: XEC instruction at line %lu in file %s cannot jump over 256 word boundary!\n", current_source->n_line, name_table[current_source->name_index]);
	}
	//debug output
	if(debug_enable)
		printf("%lX\t%X %X\tXEC\t%s %s %s [%u]\n", current_instruction->address >> 1, current_instruction->instruction_high, current_instruction->instruction_low, operands, first, second, offset + 1);
}

void m_nzt(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head)
{
	char* operands = current_source->s_operands;
	char* first = NULL;
	char* second = NULL;
	char* third = NULL;
	unsigned int reg_value;
	unsigned int l_field;
	unsigned long label_address;

	remove_spaces(operands);
	int n_operands = split_operands(current_source->s_operands, &first, &second, &third);
	if(n_operands < 2)
	{
		fprintf(stderr, "Error parsing operands of NZT instruction at line: %lu in file %s\n", current_source->n_line, name_table[current_source->name_index]);
		exit(1);
	}
	reg_value = regliv_machine_val(first, current_source->n_line, current_source->name_index);
	current_instruction->instruction_high = 0xA0 | reg_value;
	if(n_operands < 3)	//operand is a register
	{
		label_address = label_or_immediate_value(second, source_segment_head, current_source->n_line, current_source->name_index);
		current_instruction->instruction_low = (uint8_t)(label_address & 0xFF);
		if((current_instruction->address >> 9) != (label_address >> 8))
			printf("Warning: NZT instruction at line %lu in file %s cannot jump over 256 word boundary!\n", current_source->n_line, name_table[current_source->name_index]);
	}
	else	//operand is the IV bus
	{
		label_address = label_or_immediate_value(third, source_segment_head, current_source->n_line, current_source->name_index);
		l_field = atoi(second);
		current_instruction->instruction_low = (uint8_t)((l_field << 5) | (label_address & 0x1F));
		if((current_instruction->address >> 6) != (label_address >> 5))
			printf("Warning: NZT instruction at line %lu in file %s cannot jump over 32 word boundary!\n", current_source->n_line, name_table[current_source->name_index]);
	}
	//debug output
	if(debug_enable)
		printf("%lX\t%X %X\tNZT\t%s %s %s\n", current_instruction->address >> 1, current_instruction->instruction_high, current_instruction->instruction_low, first, second, third);
}

void m_xmit(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head)
{
	char* operands = current_source->s_operands;
	char* first = NULL;
	char* second = NULL;
	char* third = NULL;
	unsigned int reg_value;
	unsigned long immediate_value;
	unsigned int l_field;

	remove_spaces(operands);
	int n_operands = split_operands(current_source->s_operands, &first, &second, &third);
	if(n_operands < 2)
	{
		fprintf(stderr, "Error parsing operands of XMIT instruction at line: %lu in file %s\n", current_source->n_line, name_table[current_source->name_index]);
		exit(1);
	}
	reg_value = regliv_machine_val(second, current_source->n_line, current_source->name_index);
	current_instruction->instruction_high = 0xc0 | reg_value;
	immediate_value = label_or_immediate_value(first, source_segment_head, current_source->n_line, current_source->name_index);
	if(n_operands < 3)	//target is a register or IV bus address
	{
		current_instruction->instruction_low = immediate_value & 0xFF;
	}
	else	//target is the IV bus
	{
		l_field = atoi(third);
		current_instruction->instruction_low = (l_field << 5) | (immediate_value & 0x1F);
	}
	//debug output
	if(debug_enable)
		printf("%lX\t%X %X\tXMIT\t%s %s %s\n", current_instruction->address >> 1, current_instruction->instruction_high, current_instruction->instruction_low, first, second, third);
}

void m_jmp(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head)
{
	char* operands = current_source->s_operands;
	unsigned long immediate_value;
	remove_spaces(operands);
	immediate_value = label_or_immediate_value(operands, source_segment_head, current_source->n_line, current_source->name_index);
	current_instruction->instruction_high = 0xE0 | ((immediate_value >> 8) & 0x1F);
	current_instruction->instruction_low = immediate_value & 0xFF;
	if((current_instruction->address >> 14) != (immediate_value >> 13))
			printf("Warning: JMP instruction at line %lu in file %s cannot jump over 8192 word boundary!\n", current_source->n_line, name_table[current_source->name_index]);
	//debug output
	if(debug_enable)
		printf("%lX\t%X %X\tJMP\t%s (%lX)\n", current_instruction->address >> 1, current_instruction->instruction_high, current_instruction->instruction_low, operands, immediate_value);
}

void m_call(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head)
{
	char* operands = current_source->s_operands;
	unsigned long immediate_value;
	remove_spaces(operands);
	immediate_value = label_or_immediate_value(operands, source_segment_head, current_source->n_line, current_source->name_index);
	current_instruction->instruction_high = 0xA7;
	current_instruction->instruction_low = immediate_value & 0xFF;
	//debug output
	if(debug_enable)
		printf("%lX\t%X %X\tCALL\t%s (%lX)\n", current_instruction->address >> 1, current_instruction->instruction_high, current_instruction->instruction_low, operands, immediate_value);
}

void m_ret(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head)
{
	current_instruction->instruction_high = 0xAF;
	current_instruction->instruction_low = 0x00;
	//debug output
	if(debug_enable)
		printf("%lX\t%X %X\tRET\n", current_instruction->address >> 1, current_instruction->instruction_high, current_instruction->instruction_low);
}

void p_error(linked_source* current_source, linked_instruction* current_instruction, linked_source_segment* source_segment_head)
{
	printf("invalid mnemonic index in at line %lu in file %s\n", current_source->n_line, name_table[current_source->name_index]);
	exit(1);
}

unsigned long regliv_machine_val(char* operand, unsigned long line_num, uint8_t name_index)
{
	if ((operand[0] == 'L') && (operand[1] == 'I')) //LIV 
	{
		return (0x10 | (unsigned long)strtol(operand + 3, NULL, 8));
	}
	else if ((operand[0] == 'R') && (operand[1] == 'I'))	//RIV
	{
		return (0x18 | (unsigned long)strtol(operand + 3, NULL, 8));
	}
	else if (operand[0] == 'R') //Register 
	{
		return (unsigned long)strtol(operand + 1, NULL, 8);
	}

	fprintf(stderr, "Invalid register at line: %lu in file %s\n", line_num, name_table[name_index]);
	exit(1);	
}

void remove_spaces(char* s)
{
    char* d = s;
    do
    {
        while (*d == ' ' || *d == '\t')
        {
            ++d;
        }
		if(*d == ';')
		{
	    	*d = '\0';
		}
    } while ((*s++ = *d++));
}

int split_operands(char* operands, char** first, char** second, char** third)
{
	char delim[2];
	delim[0] = ',';
	delim[1] = 0;

	*first = strtok(operands, delim);
	if(!(*first))
		return 0;
	*second = strtok(NULL, delim);
	if(!(*second))
		return 1;
	*third = strtok(NULL, delim);
	if(!(*third))
		return 2;
	if (!strtok(NULL,delim))
		return 3;
	else
		return 4;
}

//Returns the value of the label address or the parsed immediate
unsigned long label_or_immediate_value(char* candidate, linked_source_segment* source_segment_head, unsigned long line_num, uint8_t name_index)
{
	//is label or has keyword?
	if((candidate[0] >= 0x41 && candidate[0] <= 0x5a) || (candidate[0] == '`'))
	{
		if(str_comp_partial(candidate, high_string))
			return label_or_immediate_value(candidate + 5, source_segment_head, line_num, name_index) >> 8;
		else if(str_comp_partial(candidate, low_string))
			return label_or_immediate_value(candidate + 4, source_segment_head, line_num, name_index) & 0xFF;
		else
			//return get_label_address(source_segment_head, candidate, line_num, name_index);
			return get_label_value(candidate, line_num, name_index);
	}
	//is octal
	if(candidate[0] == '@')
	{
		return (unsigned long)strtol((candidate + 1), NULL, 8);
	}
	//is hex
	if(candidate[0] == '$')
	{
		return (unsigned long)strtol((candidate + 1), NULL, 16);
	}
	//is binary
	if(candidate[0] == '%')
	{
		return (unsigned long)strtol((candidate + 1), NULL, 2);
	}
	//is char
	if(candidate[0] == '\'')
	{
		if(candidate[1] == '\\')
		{
			if(candidate[2] == 'S')
				return ' ';
			else if(candidate[2] == 'T')
				return '\t';
			else if(candidate[2] == 'N')
				return '\n';
			else if(candidate[2] == '\\')
				return '\\';
			else if(candidate[2] == '0')
				return '\0';
			else
			{
				fprintf(stderr, "Candidate [%s] did not match immediate char syntax at line: %lu in file %s\n", candidate, line_num, name_table[name_index]);
				exit(1);
			}	
		}
		else
		{
			return candidate[1];
		}
	}
	//is decimal
	if((0x30 <= candidate[0]) && (candidate[0] <= 0x39))
	{
		return (unsigned long)strtol(candidate, NULL, 10);
	}
	// throw error
	else
	{
		fprintf(stderr, "Candidate [%s] did not match label or immediate syntax at line: %lu in file %s\n", candidate, line_num, name_table[name_index]);
		exit(1);
	}
}

inline void free_node(linked_line* current_node)
{
	linked_line* next_node = NULL;
	while(1)
	{
		if(current_node == NULL)
			break;
		next_node = current_node->next;
		if(current_node->s_label)
			free(current_node->s_label);
		if(current_node->s_line)
			free(current_node->s_line);
		free(current_node);
		current_node = next_node;
	}
}

inline void free_macro(linked_macro* current_macro)
{
	linked_macro* next_macro = NULL;
	while(1)
	{
		if(current_macro == NULL)
			break;
		next_macro = current_macro->next;
		free_node(current_macro->line_head);
		free(current_macro->macro_name);	//this also frees the formal parameters
		free(current_macro);
		current_macro = next_macro;
	}
}

inline void free_source_segment(linked_source_segment* current_source_segment)
{
	linked_source_segment* next_source_segment = NULL;
	while(1)
	{
		if(current_source_segment == NULL)
			break;
		next_source_segment = current_source_segment->next;
		if(current_source_segment->source_head)
			free_source(current_source_segment->source_head);
		free(current_source_segment);
		current_source_segment = next_source_segment;
	}
}

inline void free_source(linked_source* current_source)
{
	linked_source* next_source = NULL;
	while(1)
	{
		if(current_source == NULL)
			break;
		next_source = current_source->next;
		if(current_source->s_label)
			free(current_source->s_label);
		if(current_source->s_operands)
			free(current_source->s_operands);
		free(current_source);
		current_source = next_source;
	}
}

inline void free_binary_segment(linked_binary_segment* current_binary_segment)
{
	linked_binary_segment* next_binary_segment = NULL;
	while(1)
	{
		if(current_binary_segment == NULL)
			break;
		next_binary_segment = current_binary_segment->next;
		if(current_binary_segment->instruction_head)
			free_instruction(current_binary_segment->instruction_head);
		free(current_binary_segment);
		current_binary_segment = next_binary_segment;
	}
}

inline void free_instruction(linked_instruction* current_instruction)
{
	linked_instruction* next_instruction = NULL;
	while(1)
	{
		if(current_instruction == NULL)
			break;
		next_instruction = current_instruction->next;
		free(current_instruction);
		current_instruction = next_instruction;
	}
}

inline unsigned long get_binary_segment_end(linked_binary_segment* current_binary_segment)
{
	unsigned long end_address;
	linked_instruction* current_instruction = current_binary_segment->instruction_head;
	end_address = current_instruction->next->address;	//start address
	while(1)
	{
		current_instruction = current_instruction->next;
		if(current_instruction == NULL)
			break;
		end_address = current_instruction->address;
	}
	return end_address;
}

inline unsigned long get_binary_size(linked_binary_segment* binary_segment_head)
{
	unsigned long end = 0;
	unsigned long start = 0xFFFFFFFF;
	linked_binary_segment* current_segment = binary_segment_head;

	while(1)
	{
		current_segment = current_segment->next;
		if(current_segment == NULL)
			break;
		if(current_segment->start_address < start)
			start = current_segment->start_address;
		if(current_segment->end_address > end)
			end = current_segment->end_address;
	}
	return (end - start) + 2;
}

inline void fill_buf(linked_binary_segment* binary_segment_head, uint8_t* buffer)
{
	linked_binary_segment* current_segment = binary_segment_head;

	while(1)
	{
		current_segment = current_segment->next;
		if(current_segment == NULL)
			break;
		
		linked_instruction* current_instruction = current_segment->instruction_head;
		while(1)
		{
			current_instruction = current_instruction->next;
			if(current_instruction == NULL)
				break;
			
			// Do stuff
			buffer[current_instruction->address] = current_instruction->instruction_low;
			buffer[current_instruction->address + 1] = current_instruction->instruction_high;
		}
	}
}

int str_comp_partial(const char* str1, const char* str2)
{
	for(int i = 0; str1[i] && str2[i]; ++i)
	{
		if(str1[i] != str2[i])
			return 0;
	}
	return 1;
}

int str_comp_word(const char* str1, const char* str2)
{
	unsigned int i;
	for(i = 0; str1[i] && str2[i]; ++i)
	{
		if(str1[i] != str2[i])
			return 0;
	}
	
	char next = str1[i] | str2[i];
	if(next == 0x00 || next == '\t' || next == ' ')
		return 1;
	return 0;
}

inline void find_and_replace(linked_line* current_node, char* s_replace, char* s_new)
{
	unsigned int word_start;
	unsigned int word_end;
	remove_spaces(s_new);
	while(1)
	{
		if(current_node == NULL)
			break;
		if(current_node->s_label)	//replace labels
			if(str_find_word(current_node->s_label, s_replace, &word_start, &word_end))
				str_replace(&(current_node->s_label), s_new, word_start, word_end);
		while(str_find_word(current_node->s_line, s_replace, &word_start, &word_end))	//replace operands and mnemonics
		{
			str_replace(&(current_node->s_line), s_new, word_start, word_end);
		}
		current_node = current_node->next;
	}
}

inline int str_find_word(char* where, char* what, unsigned int* start, unsigned int* end)
{
	unsigned int what_size = str_size(what);
	unsigned int where_size = str_size(where);
	if(what_size > where_size)
		return 0;
	for(unsigned int offset = 0; offset <= (where_size - what_size); ++offset)
	{
		for(unsigned int d = 0; d < what_size; ++d)
		{
			if(!what[d])
			{
				if(where[offset + d] && !(where[offset + d] == 0x09 || where[offset + d] == 0x20 || where[offset + d] == 0x2C || where[offset + d] == 0x28 || where[offset + d] == 0x29))
					break;
				if(offset)
					if(!(where[offset - 1] == 0x09 || where[offset - 1] == 0x20 || where[offset - 1] == 0x2C || where[offset - 1] == 0x28 || where[offset - 1] == 0x29))
						break;
				*start = offset;
				*end = offset + d;
				return 1;
			}
			if(where[offset + d] != what[d])
				break;
		}
	}
	return 0;
}

inline void str_replace(char** where, char* s_new, unsigned int word_start, unsigned int word_end)
{
	char* old_string = *where;
	unsigned int old_size = str_size(*where);
	unsigned int pre_length = word_start;
	unsigned int word_length = (unsigned int)strlen(s_new);
	unsigned int post_size = old_size - word_end;
	unsigned int new_size = pre_length + word_length + post_size;
	char* new_string = (char*)malloc(new_size);
	unsigned int offset = 0;
	for(unsigned int d = 0; d < pre_length; ++d)
	{
		new_string[offset] = old_string[d];
		++offset;
	}
	for(unsigned int d = 0; d < word_length; ++d)
	{
		new_string[offset] = s_new[d];
		++offset;
	}
	for(unsigned int d = word_end; d < old_size; ++d)
	{
		new_string[offset] = old_string[d];
		++offset;
	}
	free(*where);
	*where = new_string;
}

unsigned int str_size(char* s_input)
{
	unsigned int size = 0;
	for(unsigned int d = 0; 1; ++d)
	{
		++size;
		if(s_input[d] == 0x00)
			break;
	}
	return size;
}

void replace_file_extension(char* new_ext, char* out_name, char* new_name)
{
	uint8_t count = 0;
	uint8_t count_copy;
	//copy string and get its length
	do
	{
		new_name[count] = out_name[count];
		++count;
	}while(out_name[count]);
	count_copy = count;
	//search for a '.' starting from the end
	while(count)
	{
		count = count - 1;
		if(new_name[count] == '.')	//replace it and what follows with .mif
		{
			for(unsigned int d = 0; d < 5; ++d)
			{
				new_name[count] = new_ext[d];
				count = count + 1;
			}
			break;
		}
	}
	//check if the file name had no '.'
	if(count == 0)
	{
		count = count_copy;
		for(unsigned int d = 0; d < 5; ++d)
		{
			new_name[count] = new_ext[d];
			count = count + 1;
		}
	}
}

void write_mif(char* out_name, uint8_t* out_data, unsigned long prg_size)
{
	static char mif_string[] = ".mif";
	char new_name[32];
	replace_file_extension(mif_string, out_name, new_name);

	FILE* mif_file = fopen(new_name, "w");
	if(mif_file == NULL)
	{
		printf("Error creating mif file!");
		exit(1);
	}
	fprintf(mif_file, "DEPTH = %lu;\n", prg_size / 2);	//prg_size is in bytes, we want words
	fprintf(mif_file, "WIDTH = 16;\n");
	fprintf(mif_file, "ADDRESS_RADIX = HEX;\n");
	fprintf(mif_file, "DATA_RADIX = HEX;\n");
	fprintf(mif_file, "CONTENT\nBEGIN\n");
	for(unsigned long address = 0; address < (prg_size / 2); ++address)
	{
		fprintf(mif_file, "%lX : %04X;\n", address, ((out_data[address * 2 + 1] << 8) | out_data[address * 2]));
	}
	fprintf(mif_file, "END;\n");
	fclose(mif_file);
}

void write_coe(char* out_name, uint8_t* out_data, unsigned long prg_size)
{
	static char coe_string[] = ".coe";
	char new_name[32];
	replace_file_extension(coe_string, out_name, new_name);

	FILE* coe_file = fopen(new_name, "w");
	if(coe_file == NULL)
	{
		printf("Error creating coe file!");
		exit(1);
	}
	fprintf(coe_file, "memory_initialization_radix=16;\n");
	fprintf(coe_file, "memory_initialization_vector=\n");
	for(unsigned long address = 0; address < (prg_size / 2); ++address)
	{
		if((address + 1) < (prg_size / 2))
			fprintf(coe_file, "%04X,\n", ((out_data[address * 2 + 1] << 8) | out_data[address * 2]));
		else
			fprintf(coe_file, "%04X;\n", ((out_data[address * 2 + 1] << 8) | out_data[address * 2]));
	}
	fclose(coe_file);
}
