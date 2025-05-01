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
    printf("  Size       : %zu bytes\n", block->size);
    printf("  Free       : %s\n", block->free ? "Yes" : "No");
    printf("  Next       : %p\n", (void *)block->next);
    printf("  Prev       : %p\n", (void *)block->prev);
    printf("  Data Ptr   : %p\n", (void *)block->ptr);
    printf("--------------------------------\n");
    
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
	
	old_break->size = size;
	old_break->free = 0;
	old_break->next = NULL;
	old_break->prev = NULL;
	old_break->ptr = old_break->data;
	
	if(last != NULL){
		(*last)->next = old_break;
		old_break->prev = *last;

	}
	
	return (old_break);
}

void * my_malloc(size_t size){
	meta_ptr block;
	if(size <= 0){
		return NULL;
	}
	size = align8(size);
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
			printf("block sized found\n");
			print_meta_block1(block);
			block->free = 0;
			split_space(block,size);
			printf("block split done\n");
			print_meta_block1(block);
		}
	
	}
	
	return (meta_ptr)((char *)block + META_BLOCK_SIZE);
	
}

meta_ptr merge_blocks(meta_ptr block){
	if(block->next && block->next->free){
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
                base = NULL;
            }
            if(((char *)block + META_BLOCK_SIZE + block->size) == sbrk(0)){
            	brk(block);            
            }
        }
    }
}

struct Node {
    int data;
    struct Node *next;
};

struct Node *insert_head(struct Node *head, int value) {
    struct Node *new_node = (struct Node *) my_malloc(sizeof(struct Node));
    if (!new_node) return head;  // Handle memory failure

    new_node->data = value;
    new_node->next = head;
    return new_node;
}

struct Node *insert_tail(struct Node *head, int value) {
    struct Node *new_node = (struct Node *) my_malloc(sizeof(struct Node));
    if (!new_node) return head;

    new_node->data = value;
    new_node->next = NULL;

    if (!head) return new_node;

    struct Node *temp = head;
    while (temp->next) temp = temp->next;
    temp->next = new_node;

    return head;
}

void print_list(struct Node *head) {
    struct Node *temp = head;
    while (temp) {
        printf("%d -> ", temp->data);
        temp = temp->next;
    }
    printf("NULL\n");
}
void free_list(struct Node *head) {
    struct Node *temp;
    while (head) {
        temp = head;
        head = head->next;
        my_free(temp);
    }
}




struct Node *delete_at_index(struct Node *head, int index) {
    if (!head || index < 0) return head;  // Handle empty list or invalid index

    // If deleting the head node
    if (index == 0) {
        struct Node *temp = head;
        head = head->next;
        my_free(temp);
        return head;
    }

    struct Node *curr = head;
    struct Node *prev = NULL;
    int count = 0;

    // Traverse the list to find the node at index
    while (curr && count < index) {
        prev = curr;
        curr = curr->next;
        count++;
    }

    // If index is out of bounds
    if (!curr) return head;

    // Remove node from the list
    prev->next = curr->next;
    my_free(curr);

    return head;
}


int main(){
	set_brk_i();
/*
	int *arr1 = (int *)my_malloc(15 * sizeof(int));
   	 int *arr2 = (int *)my_malloc(20 * sizeof(int));
	//int * arr2 = NULL;
    if (arr1) {
        for (int i = 0; i < 10; i++) {
            arr1[i] = i * 10;
        }
        printf("Array 1: ");
        for (int i = 0; i < 10; i++) {
            printf("%d ", arr1[i]);
        }
        printf("\n");
    }
    if (arr2) {
        for (int i = 0; i < 20; i++) {
            arr2[i] = i ;
        }
        printf("Array 2: ");
        for (int i = 0; i < 20; i++) {
            printf("%d ", arr2[i]);
        }
        printf("\n");
    }
    	
    	
    	
    	*/
    	struct Node *head = NULL;
	head = insert_head(head, 10);
	head = insert_tail(head, 20);
	head = insert_tail(head, 30);

	printf("Linked List: ");
	print_list(head);
	
	meta_ptr block;
    	meta_ptr last = (meta_ptr) base;
	block = find_block(&last,23 * sizeof(int));
	
	head = delete_node(head,30);
	print_list(head);
	free_list(head);
	
	
	
	if(base == NULL){
		printf("reset done\n");
	}
	else{
		//printf("Base Block at %p:\n", base);
	}
	
	int *arr1 = (int *)my_malloc(15 * sizeof(int));
   	 int *arr2 = (int *)my_malloc(20 * sizeof(int));
	//int * arr2 = NULL;
    if (arr1) {
        for (int i = 0; i < 10; i++) {
            arr1[i] = i * 10;
        }
        printf("Array 1: ");
        for (int i = 0; i < 10; i++) {
            printf("%d ", arr1[i]);
        }
        printf("\n");
    }
    if (arr2) {
        for (int i = 0; i < 20; i++) {
            arr2[i] = i ;
        }
        printf("Array 2: ");
        for (int i = 0; i < 20; i++) {
            printf("%d ", arr2[i]);
        }
        printf("\n");
    }
    
    last = (meta_ptr) base;
	block = find_block(&last,23 * sizeof(int));
    
    	my_free(arr1);
    	my_free(arr2);
    	
    
	
	
    	last = (meta_ptr) base;
	block = find_block(&last,23 * sizeof(int));
	
}
