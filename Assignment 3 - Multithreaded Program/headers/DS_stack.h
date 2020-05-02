/*
 *  File: DS_stack.h
 *  
 *  Contents: 
 *    Stack data structure structs, stack limits, and stack function prototypes
 */
#include <stdbool.h>

/* 
 *  Limits for stack size and domain name length
 */
#define MAX_NAME_LENGTH    1024
#define MAX_STACK_SIZE      400

/* 
 *  Domain name struct
 */
typedef struct node {
    char name[MAX_NAME_LENGTH]; 
    struct node* next;         
} domain_name;

/* 
 *  Stack struct
 */
typedef struct {
    domain_name* top;
    int size;         
    bool empty;
    bool full;
} stack_ds;

/* 
 *  Stack function prototypes
 */
void init_stack(stack_ds* stack);
int push(stack_ds* stack, char* str);
void pop(stack_ds* stack, char* str);
bool is_empty(stack_ds* stack);
bool is_full(stack_ds* stack);
int get_size(stack_ds* stack);
void print_stack(stack_ds* stack);
void free_stack(stack_ds* stack);
