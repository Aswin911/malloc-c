#include "malloc.h"
#include <unistd.h>   // sbrk()
#include <string.h>   // memset(), memcpy()

// Block header — sits just before every chunk of user data
typedef struct block_header {
    size_t size;               // bytes of user data (not including header)
    int    free;               // 1 = free, 0 = in use
    struct block_header *next; // next block in linked list
} block_header_t;

#define HEADER_SIZE sizeof(block_header_t)

static block_header_t *free_list = NULL; // head of all blocks

// -------------------------------------------------------
// Internal helpers
// -------------------------------------------------------

// Walk the list; return first free block large enough
static block_header_t *find_free_block(size_t size) {
    block_header_t *current = free_list;
    while (current) {
        if (current->free && current->size >= size)
            return current;
        current = current->next;
    }
    return NULL;
}

// Ask the OS to extend the heap and carve out a new block
static block_header_t *request_space(size_t size) {
    block_header_t *block = sbrk(0);             // current brk
    void *request = sbrk(HEADER_SIZE + size);    // grow heap
    if (request == (void *)-1) return NULL;      // sbrk failed (OOM)

    block->size = size;
    block->free = 0;
    block->next = NULL;
    return block;
}

// -------------------------------------------------------
// Public API
// -------------------------------------------------------

void *my_malloc(size_t size) {
    if (size == 0) return NULL;

    block_header_t *block;

    if (!free_list) {
        // First-ever allocation
        block = request_space(size);
        if (!block) return NULL;
        free_list = block;
    } else {
        block = find_free_block(size); // first-fit search
        if (!block) {
            // No suitable free block — grow the heap
            block = request_space(size);
            if (!block) return NULL;
            // Append to end of the list
            block_header_t *last = free_list;
            while (last->next) last = last->next;
            last->next = block;
        } else {
            block->free = 0; // reclaim existing free block
        }
    }

    // Return pointer just past the header — this is what the caller uses
    return (block_header_t *)block + 1;
}

void my_free(void *ptr) {
    if (!ptr) return;

    // Step back one header-width to reach the header
    block_header_t *block = (block_header_t *)ptr - 1;
    block->free = 1;

    // Coalesce: merge with the immediately following block if it is also free
    // This prevents heap fragmentation over repeated malloc/free cycles
    if (block->next && block->next->free) {
        block->size += HEADER_SIZE + block->next->size;
        block->next  = block->next->next;
    }
}

void *my_calloc(size_t num, size_t size) {
    void *ptr = my_malloc(num * size);
    if (ptr) memset(ptr, 0, num * size); // zero-initialise
    return ptr;
}

void *my_realloc(void *ptr, size_t size) {
    if (!ptr) return my_malloc(size);

    block_header_t *block = (block_header_t *)ptr - 1;
    if (block->size >= size) return ptr; // already fits

    void *new_ptr = my_malloc(size);
    if (!new_ptr) return NULL;

    memcpy(new_ptr, ptr, block->size); // copy old data
    my_free(ptr);
    return new_ptr;
}
