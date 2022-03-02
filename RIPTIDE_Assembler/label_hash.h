typedef struct LABEL_NODE
{
	char* key;
	unsigned long value;
	struct LABEL_NODE* next;
} label_node;

label_node* label_map = NULL;
unsigned int label_map_size;

unsigned long hash(unsigned char* str)
{
    unsigned long hash = 5381;
    int c;

    while (c = *str++)
        hash = ((hash << 5) + hash) + c; /* hash * 33 + c */

    return hash;
}

void insert_label(char* label, unsigned long n_line, uint8_t name_index, unsigned long address)
{
	unsigned int index = (unsigned int)(hash((unsigned char*)label) % (unsigned long)label_map_size);
	if(label_map[index].key)	//handle collision
	{
		label_node* current_node = label_map + index;
		label_node* last_node;
		while(current_node)	//advance to last node
		{
			if(!strcmp(label, current_node->key))
			{
				fprintf(stderr, "Label [%s] at line: %lu in file %s has already been declared\n", label, n_line, name_table[name_index]);
			}
			last_node = current_node;
			current_node = current_node->next;
		}
		//create new node
		last_node->next = (label_node*)malloc(sizeof(label_node));
		current_node = last_node->next;
		current_node->key = label;
		current_node->next = NULL;
		current_node->value = address;
	}
	else	//no collision
	{
		label_map[index].key = label;
		label_map[index].value = address;
	}
	return;
}

void build_label_map(linked_source_segment* source_segment_head, unsigned int num_labels)
{
	label_map_size = num_labels + (num_labels >> 1);
	label_map = (label_node*)malloc(label_map_size * sizeof(label_node));
	for(unsigned int d = 0; d < label_map_size; ++d)
	{
		label_map[d].key = NULL;
		label_map[d].next = NULL;
	}
	unsigned long offset;
	linked_source_segment* current_source_segment = source_segment_head->next;
	while(current_source_segment)
	{
		offset = current_source_segment->offset;
		linked_source* current_source = current_source_segment->source_head->next;
		while(current_source)
		{
			if(current_source->s_label)
				insert_label(current_source->s_label, current_source->n_line, current_source->name_index, offset);
			current_source = current_source->next;
			offset = offset + 1;
		}
		current_source_segment = current_source_segment->next;
	}
	return;
}

unsigned long get_label_value(char* label, unsigned int n_line, uint8_t name_index)
{
	unsigned int index = (unsigned int)(hash((unsigned char*)label) % (unsigned long)label_map_size);
	if(label_map[index].key)
	{
		label_node* current_node = label_map + index;
		while(current_node)
		{
			if(!strcmp(label, current_node->key))
			{
				return current_node->value;
			}
			current_node = current_node->next;
		}
	}
	fprintf(stderr, "Unable to find label [%s] at line: %lu in file %s\n", label, n_line, name_table[name_index]);
	exit(1);
}

inline void free_label_map()
{
	free(label_map);
	return;
}
