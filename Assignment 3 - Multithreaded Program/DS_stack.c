/*
 *  File: DS_stack.c
 *  
 *  Contents: 
 *    Stack function definitions.
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include "headers/DS_stack.h"

/***************************************************************
 *  Function:  init_stack
 *  ----------------------------------------
 *   stack: Pointer to a stack (stack_ds) data structure.
 * 
 *   Description:
 *     Initializes a stack by setting starting attributes.
 * 
 *   returns:
 *      none
 ***************************************************************/
void init_stack(stack_ds* stack) {
    stack->top = NULL;
    stack->size = 0;
    stack->empty = true;
    stack->full = false;
}

/***************************************************************
 *  Function:  push
 *  ----------------------------------------
 *   stack: Pointer to a stack (stack_ds) data structure.
 *     str: A string buffer containing a domain name.
 * 
 *   Description:
 *     Pushes a domain name onto the stack.
 * 
 *   returns:
 *      none
 ***************************************************************/
int push(stack_ds* stack, char* str) {
    /* Return immediately if the stack is full */
    if (stack->full) {
        errno = EPERM;
        return 0;
    }
    /* Create the domain name */
    domain_name* new_domain;
    if ((new_domain = (domain_name*) malloc(sizeof(domain_name))) == NULL) {
        fprintf(stderr, "Error: malloc in push");
        exit(EXIT_FAILURE);
    }
    strcpy(new_domain->name, str);
    new_domain->next = NULL;

    /* Add the domain name to the stack */
    if (stack->size == 0) {
        stack->empty = false;
    } else {
        new_domain->next = stack -> top;
    }
    stack->size++;
    if (stack->size == MAX_STACK_SIZE) {
        stack->full = true;
    }
    stack -> top = new_domain;
    return 1;
}

/***************************************************************
 *  Function:  pop
 *  ----------------------------------------
 *   stack: Pointer to a stack (stack_ds) data structure.
 *     str: A string buffer.
 * 
 *   Description:
 *     Pops a domain name from the stack and stores it in
 *     the buffer 'str'.
 * 
 *   returns:
 *      none
 ***************************************************************/
void pop(stack_ds* stack, char* str) {

    // return immediately if the stack is empty
    if (stack->empty) {
        errno = EPERM;
        return;
    }
    domain_name* top = stack -> top;   // get pointer to the top domain name
    strcpy(str, top->name);

    // Set a new top of the stack and update attributes
    stack->top = top -> next;
    if (stack->top == NULL) {
        stack->empty=true;
    }
    if (stack->full) {
        stack->full = false;
    }
    stack->size--;
    free(top);
}

/***************************************************************
 *  Function:  free_stack
 *  ----------------------------------------
 *   stack: Pointer to a stack (stack_ds) data structure.
 * 
 *   Description:
 *     Free's all allocated domain names held by the stack.
 * 
 *   returns:
 *      none
 ***************************************************************/
void free_stack(stack_ds* stack) {
    domain_name* i = stack -> top;
    domain_name* j = i;
    while(i != NULL) {
        i = i->next;
        free(j);
        j = i;
    }
}

/***************************************************************
 *  Function:  is_empty
 *  ----------------------------------------
 *   stack: Pointer to a stack (stack_ds) data structure.
 * 
 *   Description:
 *     Checks whether the stack is empty.
 * 
 *   returns:
 *      (bool) true  : If the stack is empty
 *      (bool) false : If the stack is not empty
 ***************************************************************/
bool is_empty(stack_ds* stack) {
    return stack -> empty;
}

/***************************************************************
 *  Function:  is_full
 *  ----------------------------------------
 *   stack: Pointer to a stack (stack_ds) data structure.
 * 
 *   Description:
 *     Checks whether the stack is full.
 * 
 *   returns:
 *      (bool) true  : If the stack is full
 *      (bool) false : If the stack is not full
 ***************************************************************/
bool is_full(stack_ds* stack) {
    return stack -> full;
}

/***************************************************************
 *  Function:  get_size
 *  ----------------------------------------
 *   stack: Pointer to a stack (stack_ds) data structure.
 * 
 *   Description:
 *     Checks the stack size.
 * 
 *   returns:
 *      (int) size : The number of domain names in the stack.
 ***************************************************************/
int get_size(stack_ds* stack) {
    return stack -> size;
}

/***************************************************************
 *  Function:  print_stack
 *  ----------------------------------------
 *   stack: Pointer to a stack (stack_ds) data structure.
 * 
 *   Description:
 *     Prints all domain names currently stored in the stack.
 * 
 *   returns:
 *      none
 ***************************************************************/
void print_stack(stack_ds* stack) {
    domain_name* cursor = stack -> top;
    printf("\n[**STACK TOP**]\n\n");
    while(cursor != NULL) {
        printf("%s\n", cursor->name);
        cursor = cursor->next;
    }
    printf("\n[**STACK BOTTOM**]\n\n");
}