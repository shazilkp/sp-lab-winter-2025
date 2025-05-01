#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h> 



void * base = NULL;

struct meta_block{
	int free;
	size_t size;
	struct meta_block * prev;
	struct meta_block * next;
	void * ptr;
	char data[];
};
typedef struct meta_block * meta_ptr;

#define META_BLOCK_SIZE sizeof(struct meta_block)
#define align8(x) (((((x)-1) >> 2) << 2) + 4)  // Aligns `x` to the next multiple of 8

void print_meta_block(meta_ptr block) {
    if (!block) {
        printf("NULL block\n");
        return;
    }

    printf("Meta Block at %p:\n", (void *)block);
    printf("  Size       : %zu bytes\n", block->size);
    printf("  Free       : %s\n", block->free ? "Yes" : "No");
    printf("  Next       : %p\n", (void *)block->next);
    printf("  Prev       : %p\n", (void *)block->prev);
    printf("  Data Ptr   : %p\n", (void *)block->ptr);
    printf("--------------------------------\n");
}

void traverse_and_print() {
    meta_ptr b = base; // Assuming 'base' is the head of the memory block list
    
    printf("---------------------------------------Traversal Start----------------------------------------\n");
    while (b) {
        print_meta_block(b); // Print details of the current block
        b = b->next;          // Move to the next block
    }
    printf("---------------------------------------Traversal END----------------------------------------\n");
}

meta_ptr find_block(meta_ptr * last,size_t size){
	meta_ptr b = base;
	while(b && !(b->free && b->size >= size)){
		//print_meta_block(b);
		*last = b;
		b = b->next;
	}
	return b;
}


void split_space(meta_ptr block, size_t size){
	if (block->size - size <= META_BLOCK_SIZE) {
		fprintf(stderr,"Given block too small to split");
		fprintf(stderr,"Requesting size %zu,Available space: %zu, META_BLOCK_SIZE: %zu\n",size, block->size - size, META_BLOCK_SIZE);
		return; // Avoid creating tiny unusable fragments
	}
	
	meta_ptr new_block;
	//new_block = block->data + size;
	new_block = (meta_ptr)((char *)block + META_BLOCK_SIZE + size);

	new_block->size = block->size - size - META_BLOCK_SIZE;
	new_block->next = block->next;
	new_block->free = 1;
	new_block->ptr = new_block->data;
	new_block->prev = block;
	block->next = new_block;
	/*
	printf("----------------------SIZE UPDATION-------------------------\n");
	print_meta_block1(block);
	block->size = size;
	print_meta_block1(block);
	printf("----------------------SIZE UPDATION OVER-------------------------");
	*/
	block->size = size;
	if(new_block->next){
		new_block->next->prev = new_block;
	}
}

meta_ptr extend_heap(meta_ptr * last,size_t size){
	meta_ptr old_break, new_break;
	old_break = sbrk(0);		//find the current break position
	//printf("old break = %p\n",(void *)old_break);
	new_break = sbrk(META_BLOCK_SIZE + size);
	//printf("new break = %p\n",(void *)new_break);
	

	//printf("sbrk request: %zu bytes\n", META_BLOCK_SIZE + size);

	//printf("Heap extended by: %ld bytes\n", (char *)new_break - (char *)old_break);
	
	
	if(new_break == (void *)-1){
		return NULL;
	}
	
	/*
	printf("----------------------Extend SIZE UPDATION-------------------------\n");
	print_meta_block1(old_break);
		
	print_meta_block1(old_break);
	printf("----------------------EXTEND SIZE UPDATION OVER-------------------------");
	*/
	old_break->size = size;
	
	old_break->free = 0;
	old_break->next = NULL;
	old_break->prev = NULL;
	old_break->ptr = old_break->data;
	
	if(last != NULL){
		(*last)->next = old_break;
		old_break->prev = *last;

	}
	//global_array[global_i++] = old_break;
	return (old_break);
}

void * my_malloc(size_t size){
	meta_ptr block;
	if(size <= 0){
		return NULL;
	}
	//printf("malloc\ninitial size :%zu\n",size);
	size = align8(size);
	//printf("after alignment :%zu\n",size);
	if(base == NULL){
		block = extend_heap(NULL,size);
		//print_meta_block(block);
		if(!block){
			return NULL;
		}
		base = block;
	}
	else{
		meta_ptr last = (meta_ptr) base;
		block = find_block(&last,size);
		if(!block){
			block = extend_heap(&last,size);
			//printf("Just extended\n");
			//print_meta_block(block);
			if(!block){
				return NULL;
			}
		}
		else{
			
			block->free = 0;
			split_space(block,size);
			
		}
	
	}
	
	return (meta_ptr)((char *)block + META_BLOCK_SIZE);
	
}

meta_ptr merge_blocks(meta_ptr block){
	if(block->next && block->next->free){
		//block->size = block->size + META_BLOCK_SIZE + block->next->size;
		/*
		printf("----------------------MERGE SIZE UPDATION-------------------------\n");
		print_meta_block1(block);
			block->size = block->size + META_BLOCK_SIZE + block->next->size;
		print_meta_block1(block);
		printf("----------------------MERGE SIZE UPDATION OVER-------------------------");
		*/
		block->size = block->size + META_BLOCK_SIZE + block->next->size;
		block->next = block->next->next;
	}
	if(block->next){
		block->next->prev = block;
	}
	return block;
}

meta_ptr get_block_addr(void * p){
	char * tmp = p;
	tmp = tmp - META_BLOCK_SIZE;
	
	return (meta_ptr)tmp;
}

int is_addr_valid(void * p){
	if(!p){
		return 0;
	}
	if(base){				//check if allocation started
		if(p > base && p < sbrk(0)){	//if within allocated heap
			return(p == (get_block_addr(p)->ptr));
		}
	}
	return 0;
}

void my_free(void *ptr)
{
    if (is_addr_valid(ptr))
    {
    	//printf("Address given for free valid\n");
        meta_ptr block = get_block_addr(ptr);
        block->free = 1;
        if (block->prev && block->prev->free)
        {
            block = merge_blocks(block->prev);
        }

        if (block->next)
        {
            block = merge_blocks(block);
        }
        else
        {
            if (block->prev)
            {
                block->prev->next = NULL;
            }
            else
            {
            	//printf("Block end: %p, Program break: %p\n", (void *)((char *)block + META_BLOCK_SIZE + block->size),(void *)sbrk(0));
		//fflush(stdout);
                //base = NULL;
            }
           
            if(((char *)block + META_BLOCK_SIZE + block->size) == sbrk(0)){
            //	printf("sbrk executes\n");
            	brk(block);            
            }
        }
    }
}

void copy_data(meta_ptr src,meta_ptr dest){
	memcpy(dest->data,src->data,(src->size)*sizeof(int));
}

void *my_realloc1(void *ptr, size_t size) {
    void *new_ptr;

    // Case 1: If ptr is NULL, simply allocate new memory.
    if (ptr == NULL) {
        return my_malloc(size);
    }

    // Case 2: If the given pointer is invalid, return NULL.
    if (!is_addr_valid(ptr)) {
        return NULL;
    }

    meta_ptr old_block = get_block_addr(ptr);
    size_t s = align8(size);

    // Case 3: If the existing block is already large enough
    if (old_block->size >= s) {
        if (old_block->size >= (META_BLOCK_SIZE + s)) {
            split_space(old_block, s);
        }
        return (char *)old_block + META_BLOCK_SIZE; // Return the correct pointer
    }

    // Case 4: Try merging with the next free block
    if (old_block->next && old_block->next->free &&
        (old_block->size + old_block->next->size + META_BLOCK_SIZE) >= s) {
        
        old_block = merge_blocks(old_block); // Ensure we get the updated block

        if (old_block->size >= (META_BLOCK_SIZE + s)) {
            split_space(old_block, s);
        }
        return (char *)old_block + META_BLOCK_SIZE; // Return the correct pointer
    }

    // Case 5: Allocate a new block and copy the data
    new_ptr = my_malloc(size);
    if (!new_ptr) {
        return NULL;
    }

    meta_ptr new_block = get_block_addr(new_ptr);
    copy_data(old_block, new_block);
    my_free(ptr); // Free the old block

    return (char *)new_block + META_BLOCK_SIZE;
}



void * my_realloc(void * ptr, size_t size){
	void * new_ptr;
	if(ptr == NULL){
		return my_malloc(size);
	}
	
	if(!is_addr_valid(ptr)){
		return NULL;
	}
	
	meta_ptr old_block = get_block_addr(ptr);
	size_t s = align8(size);
	
	if(old_block->size >= s){
		if(old_block->size >= (META_BLOCK_SIZE + s)){
			split_space(old_block,s);
		}
	}
	
	else{
		if(old_block->next && old_block->next->free && (old_block->size + old_block->next->size + META_BLOCK_SIZE >= s)){
			merge_blocks(old_block);
			if(old_block->size >= (META_BLOCK_SIZE + s)){
				split_space(old_block,s);
			}
			return (meta_ptr)((char *)old_block + META_BLOCK_SIZE);
		}
		else{
			new_ptr = my_malloc(size);
			if (!new_ptr)
				return NULL;
				
			meta_ptr new_block = get_block_addr(new_ptr);
			copy_data(old_block,new_block);
			my_free(ptr);
			return (meta_ptr)((char *)new_block + META_BLOCK_SIZE);
		}
	}
	return ptr;	
}


struct DynamicArray{
	int * data;
	int size;
	int capacity;
};

typedef struct DynamicArray DynamicArray;

void print_array(DynamicArray *arr){
	int *array = arr->data;
	for(int i = 0 ; i < arr->size ; i++){
		printf("%d ",array[i]);
	}
	printf("\n");
}



void init_array(DynamicArray *arr, int initial_size){
	int * ptr = (int*)my_malloc(initial_size);
	if(!ptr){
		return;
	}
	arr->data = ptr;
	arr->size = 0;
	arr->capacity = initial_size;
}

void insert_element(DynamicArray *arr, int value){
	
	if(arr->size >= arr->capacity){
		printf("doubling size\n");
		
		arr->data = (int *)my_realloc(arr->data, 2*arr->capacity);
		arr->capacity = 2*arr->capacity;
		
	}
	
	int old_size = arr->size;
	int *array = arr->data;
	
	array[old_size] = value;
	arr->size = arr->size + 1;	
}

void remove_element(DynamicArray *arr, int index){
	if(index >= arr->size){
		return;
	}
	int *array = arr->data;
	for(int i = index ; i < arr->size - 1 ; i++){
		array[i] = array[i+1];
	}
	array[arr->size - 1] = 0;
	arr->size = arr->size - 1;
	
	if(4 * arr->size <= arr->capacity){
		printf("reduction doen\n");
		arr->data = (int *)my_realloc(arr->data, ((arr->capacity) + 1) / 2);
		arr->capacity = ((arr->capacity) + 1) / 2;
	}
}

void resize_array(DynamicArray *arr, int new_size){
	int *array = arr->data;
	arr->data = (int *)my_realloc(arr->data, new_size);
	arr->capacity = new_size;
}

void free_array(DynamicArray *arr){
	my_free(arr->data);
}

int main(){
	DynamicArray arr;
	init_array(&arr, 5);
	insert_element(&arr, 10);
	insert_element(&arr, 20);
	insert_element(&arr, 30);
	print_array(&arr);
	for(int i = 0 ; i < 3 ; i++){
		insert_element(&arr, (i +1)%3 + 1);
	}
	print_array(&arr);
	for(int i = 0 ; i < 3 ; i++){
		remove_element(&arr, 0);
	}
	print_array(&arr);
	remove_element(&arr, 1);
	print_array(&arr);
	resize_array(&arr, 10);
	print_array(&arr);
	free_array(&arr);
}
