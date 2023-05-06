#define true 1
typedef struct S_RANGE
{
	unsigned long start_address;
	unsigned long end_address;
} s_range;

s_range* range_table = NULL;

void range_heapify_down(s_range* heap, unsigned int heap_size, unsigned int index)
{
	unsigned int left_index;
	unsigned int right_index;
	unsigned int largest;
	s_range temp;
	while(true)
	{
		left_index = index * 2 + 1;
		right_index = index * 2 + 2;
		largest = index;
		if((left_index < heap_size) && (heap[left_index].start_address > heap[largest].start_address))
			largest = left_index;
		if((right_index < heap_size) && (heap[right_index].start_address > heap[largest].start_address))
			largest = right_index;
		if(largest == index)
			return;
		temp = heap[index];
		heap[index] = heap[largest];
		heap[largest] = temp;
		index = largest;
	}
}

void range_heapify(s_range* elements, unsigned int size)
{
	for(unsigned int index = size - 1; index != (unsigned int)(-1); --index)
	{
		range_heapify_down(elements, size, index);
	}
	return;
}

void range_heapsort(s_range* elements, unsigned int size)
{
	s_range temp;
	range_heapify(elements, size);
	for(unsigned int end = size - 1; end != (unsigned int)(-1); --end)
	{
		temp = elements[end];
		elements[end] = elements[0];
		elements[0] = temp;
		--size;
		range_heapify(elements, size);
	}
	return;
}

void build_range_table(linked_binary_segment* segments, unsigned int num_segments)
{
	range_table = (s_range*)malloc(num_segments * sizeof(s_range));
	unsigned int index = 0;
	while(1)
	{
		segments = segments->next;
		if(segments == NULL)
			break;
		range_table[index].start_address = segments->start_address;
		range_table[index].end_address = segments->end_address;
		++index;
	}
	range_heapsort(range_table, num_segments);
	return;
}

void print_range_table(unsigned int num_segments)
{
	for(unsigned int d = 0; d < num_segments; ++d)
	{
		printf("Segment: %u\n", d);
		printf("\t Start address: %lu\n", range_table[d].start_address / 2);
		printf("\t End address: %lu\n", range_table[d].end_address / 2);
	}
	return;
}

void check_range_table(unsigned int num_segments)
{
	--num_segments;
	unsigned int d = 0;
	while(d < num_segments)
	{
		unsigned long end = range_table[d].end_address;
		unsigned long start = range_table[++d].start_address;
		if(end >= start)
			printf("Warning: Segment at offset %lu overlaps with previous segment.\n", (range_table[d].start_address) / 2);
	}
	return;
}

void free_range_table()
{
	free(range_table);
	return;
}