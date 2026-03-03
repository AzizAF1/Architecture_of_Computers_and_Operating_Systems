#pragma once

#include <stdint.h>
#include <stdatomic.h>
#include <stdlib.h>
#include <stdbool.h>

typedef struct node {
  struct node* next;
  uintptr_t value;
} node_t;

typedef struct lfstack {
  // TODO: ??
  node_t* top;
} lfstack_t;

int lfstack_init(lfstack_t *stack) {
  // TODO: initialize stack
  stack->top = NULL;
  return 0; // success
}

int lfstack_push(lfstack_t *stack, uintptr_t value) {
  // TODO: push value into stack
  node_t* new_node = (node_t*)malloc(sizeof(node_t));
  if (new_node == NULL) {
    return -1;
  }
  new_node->value = value;
  node_t* fix_top = atomic_load(&stack->top);
  while(true) {
    new_node->next = fix_top;
    if (atomic_compare_exchange_strong(&stack->top, &fix_top, new_node)) {
          break;
    }
  }
  return 0; // success
}

int lfstack_pop(lfstack_t *stack, uintptr_t *value) {
  // TODO: pop from stack and store result in *value
  node_t* fix_top = atomic_load(&stack->top);
  if (fix_top == NULL) {
      *value = 0;
      return 0;
  }
  while (true) {
    node_t* fix_next = fix_top->next;
    if (atomic_compare_exchange_strong(&stack->top, &fix_top, fix_next)) {
        *value = fix_top->value;
        free(fix_top);
        break;
    }
  }
  return 0; // success
}

int lfstack_destroy(lfstack_t *stack) {
  // TODO: destroy empty stack
  while(true) {
    node_t* fix_top = atomic_load(&stack->top);
    if (fix_top == NULL) {
      break;
    }
    node_t* fix_next = fix_top->next;
    free(fix_top);
    if (atomic_compare_exchange_strong(&stack->top, &fix_top, fix_next)) {
          break;
    }
  }
  
  return 0; // success
}
