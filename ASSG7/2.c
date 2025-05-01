#include <sys/types.h>
#include <unistd.h>
#include <stdio.h>
#include <errno.h>
#include <string.h> 


void * base = NULL;

struct meta_block{
	int free;
	size_t size;
	struct meta_block * next;
	struct meta_block * prev;
	void * ptr;
	char data[];
};


#define META_BLOCK_SIZE sizeof(struct meta_block)
#define align4(x) (((((x)-1) >> 2) << 2) + 4)
#define align8(x) (((x) + 7) & ~7)  // Aligns `x` to the next multiple of 8

void * brk_i;

void set_brk_i(){
	brk_i = sbrk(0);
}

typedef struct meta_block * meta_ptr;

void print_meta_block(meta_ptr block) {
/*
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
  */  
}

void print_meta_block1(meta_ptr block) {

    if (!block) {
        printf("NULL block\n");
        return;
    }
	
    printf("Meta Block at %p:\n", (void *)block);
    /*
    printf("  Size       : %zu bytes\n", block->size);
    
    printf("  Free       : %s\n", block->free ? "Yes" : "No");
    printf("  Next       : %p\n", (void *)block->next);
    printf("  Prev       : %p\n", (void *)block->prev);
    printf("  Data Ptr   : %p\n", (void *)block->ptr);
    */
    printf("--------------------------------\n");
    
}

void traverse_and_print() {
    meta_ptr b = base; // Assuming 'base' is the head of the memory block list
    
    printf("---------------------------------------Traversal Start----------------------------------------\n");
    while (b) {
        print_meta_block1(b); // Print details of the current block
        b = b->next;          // Move to the next block
    }
    printf("---------------------------------------Traversal END----------------------------------------\n");
}

meta_ptr find_block(meta_ptr * last,size_t size){
	meta_ptr b = base;
	while(b && !(b->free && b->size >= size)){
		print_meta_block(b);
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

meta_ptr global_array[1000];
int global_i = 0;

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
	global_array[global_i++] = old_break;
	return (old_break);
}

void global_print(){
	printf("----------------------STARTING GLOBAL-------------------------\n");
	for(int j =0 ; j < global_i ; j++){
		print_meta_block1(global_array[j]);
	}
	printf("----------------------ENDING GLOBAL-------------------------\n");
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
		print_meta_block(block);
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
			print_meta_block(block);
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

meta_ptr merge_blocks2(meta_ptr block) {
    if (block->next && block->next->free) {
        printf("Merging block at %p (size: %zu) with next free block at %p (size: %zu)\n", 
               (void *)block, block->size, (void *)block->next, block->next->size);

        block->size = block->size + META_BLOCK_SIZE + block->next->size;
        block->next = block->next->next;

        if (block->next) {
            block->next->prev = block;
        }

        printf("New merged block at %p has size: %zu\n", (void *)block, block->size);
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

void my_free2(void * p){
	if(is_addr_valid(p)){
		printf("Address given for free valid\n");
		meta_ptr block = get_block_addr(p);
		printf("freeing block \n");
		print_meta_block(block);
		block->free = 1;
		if(block->prev && block->prev->free){
			block = merge_blocks(block->prev);
		}
		if(block->next && block->next->free){
			block = merge_blocks(block);
		}
		if(!block->next){
			printf("Im block %p and Im at the end\n",(void *) block);
			
			print_meta_block(block);
			if(block->prev){
				printf("hello world\n");
				fflush(stdout);
				block->prev->next = NULL;
				brk(block->prev->data + block->prev->size);		//in the case our ll is ever non contigous
				printf("old break = %p\n",(void *)sbrk(0));
			}
			else{
				//only 1 block in free list
				printf("hello not so world\n");
				fflush(stdout);
				base = NULL;
				printf("Block end: %p, Program break: %p\n", (void *)((char *)block + META_BLOCK_SIZE + block->size),(void *)sbrk(0));
				fflush(stdout);

				if (brk((void *)block) == -1) {
					fprintf(stderr, "Error: brk() failed! errno: %d (%s)\n", errno, strerror(errno));
				}
				else {
					printf("brk() successful, new break set at %p\n", (void *)sbrk(0));
				}
				printf("old break = %p\n",(void *)sbrk(0));
			}
			
			
		}
		
	}
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

void * my_calloc(size_t num,size_t size){
	void * new;
	
	//printf("calloc\ninitial size :%zu\n",size);
	//size = align8(size);
	//printf("after alignment :%zu\n",size);
	new = my_malloc(num * size);
	if(new){
		memset(new, 0, num * size);
	}
	return new;
}

int ** create_matrix(int rows,int cols){
	int ** array = (int **)my_calloc(rows , sizeof(int *));
	if(!array){
		return NULL;
	}
	
	for(int i = 0 ; i < rows ; i++){
		array[i] = (int *)my_calloc(cols , sizeof(int));
		if (!array[i]) {
			printf("mem alloc failed at row:%d\n", i);
			for (int j = 0; j < i; j++) {
				my_free(array[j]);
			}
			my_free(array);
		return NULL;
		}
	}
	return array;
}

void fill_matrix(int ** mat,int rows,int cols){
	for(int i = 0 ; i < rows; i++){
		for(int j = 0; j < cols ; j++){
			mat[i][j] = (i * j) + i + j;
		}
	}
}

void print_matrix(int ** mat,int rows,int cols){
	if(!mat){
		printf("Matrix doesnt exist\n");
	}
	for(int i = 0 ; i < rows ; i ++){
		for(int j = 0; j < cols ; j++){
			printf("%d ",mat[i][j]);
		}
		printf("\n");
	}
	printf("\n");
}

void free_matrix(int ** mat,int rows,int cols){
	for(int i = 0 ; i < rows ; i++){
		my_free(mat[i]);
	}
	my_free(mat);
}

int main(){

	int rows = 5 , cols = 7;
	int ** arr = create_matrix(rows,cols);
	if(!arr){
		return 1;
	}
	
	fill_matrix(arr,rows,cols);
	//traverse_and_print();
	print_matrix(arr,rows,cols);
	free_matrix(arr,rows,cols);
	//global_print();
	//printf("bobby big guy-----------------------------------\n");
	//traverse_and_print();
	
	rows = 4 , cols = 4;
	int ** arr1 = create_matrix(rows,cols);
	//traverse_and_print();
	if(!arr1){
		return 1;
	}
	//global_print();
	
	for(int i = 0 ; i < rows; i++){
		for(int j = 0; j < cols ; j++){
			arr1[i][j] = 1234;
		}
	}
	print_matrix(arr,5,7);
	print_matrix(arr1,4,4);
	
}
