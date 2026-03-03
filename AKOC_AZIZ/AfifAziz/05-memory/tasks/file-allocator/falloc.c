#include "falloc.h"
#include <fcntl.h>
#include <sys/mman.h>
#include <sys/stat.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdint.h>

// Helper function to count set bits in the page mask up to allowed_page_count
static uint64_t count_set_bits(uint64_t* mask, uint64_t allowed_page_count) {
    uint64_t count = 0;
    for (uint64_t i = 0; i < allowed_page_count; i++) {
        uint64_t word = i / 64;
        uint64_t bit = i % 64;
        if (mask[word] & (1ULL << bit)) {
            count++;
        }
    }
    return count;
}

void falloc_init(file_allocator_t* allocator, const char* filepath, uint64_t allowed_page_count) {
    // Initialize allocator fields
    allocator->fd = -1;
    allocator->base_addr = NULL;
    allocator->page_mask = NULL;
    allocator->curr_page_count = 0;
    allocator->allowed_page_count = allowed_page_count;

    // Ensure allowed_page_count does not exceed the maximum supported
    if (allowed_page_count > PAGE_MASK_SIZE * 8) { // 512 bytes * 8 bits = 4096 bits
        allowed_page_count = PAGE_MASK_SIZE * 8;
        allocator->allowed_page_count = allowed_page_count;
    }

    // Open or create the file
    int fd = open(filepath, O_RDWR | O_CREAT, 0666);
    if (fd < 0) {
        perror("Failed to open or create the file");
        return;
    }

    // Get the current size of the file
    struct stat st;
    if (fstat(fd, &st) < 0) {
        perror("Failed to get file status");
        close(fd);
        return;
    }

    off_t expected_size = PAGE_MASK_SIZE + ((off_t)allowed_page_count * PAGE_SIZE);
    if (st.st_size == 0) {
        // New file: set its size and initialize the page mask to 0
        if (ftruncate(fd, expected_size) < 0) {
            perror("Failed to set file size with ftruncate");
            close(fd);
            return;
        }

        // Map the file to initialize the page mask
        void* mapped = mmap(NULL, expected_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (mapped == MAP_FAILED) {
            perror("Failed to mmap the file");
            close(fd);
            return;
        }

        // Initialize page_mask to 0
        memset(mapped, 0, PAGE_MASK_SIZE);

        // Unmap after initialization
        if (munmap(mapped, expected_size) < 0) {
            perror("Failed to munmap after initialization");
            close(fd);
            return;
        }
    } else {
        // Existing file: verify its size matches the expected size
        if (st.st_size != expected_size) {
            fprintf(stderr, "Existing file size does not match expected size.\n");
            close(fd);
            return;
        }
    }

    // Map the file into memory
    void* mapped = mmap(NULL, expected_size, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
    if (mapped == MAP_FAILED) {
        perror("Failed to mmap the file");
        close(fd);
        return;
    }

    // Assign allocator fields
    allocator->fd = fd;
    allocator->page_mask = (uint64_t*)mapped;
    allocator->base_addr = (char*)mapped + PAGE_MASK_SIZE;

    // Count the number of currently allocated pages
    allocator->curr_page_count = count_set_bits(allocator->page_mask, allocator->allowed_page_count);
}

void falloc_destroy(file_allocator_t* allocator) {
    if (allocator->page_mask && allocator->base_addr) {
        size_t map_size = PAGE_MASK_SIZE + ((size_t)allocator->allowed_page_count * PAGE_SIZE);
        if (munmap((void*)allocator->page_mask, map_size) < 0) {
            perror("Failed to munmap");
        }
    }

    if (allocator->fd >= 0) {
        if (close(allocator->fd) < 0) {
            perror("Failed to close file descriptor");
        }
    }

    // Reset allocator fields
    allocator->fd = -1;
    allocator->page_mask = NULL;
    allocator->base_addr = NULL;
    allocator->curr_page_count = 0;
    allocator->allowed_page_count = 0;
}

void* falloc_acquire_page(file_allocator_t* allocator) {
    if (allocator->curr_page_count >= allocator->allowed_page_count) {
        return NULL;
    }

    for (uint64_t i = 0; i < allocator->allowed_page_count; i++) {
        uint64_t word = i / 64;
        uint64_t bit = i % 64;

        if (!(allocator->page_mask[word] & (1ULL << bit))) {
            // Mark the page as allocated
            allocator->page_mask[word] |= (1ULL << bit);
            allocator->curr_page_count++;

            // Calculate the address of the allocated page
            char* page_addr = (char*)allocator->base_addr + (i * PAGE_SIZE);
            return (void*)page_addr;
        }
    }

    // No free page found
    return NULL;
}

void falloc_release_page(file_allocator_t* allocator, void** addr) {
    if (addr == NULL || *addr == NULL) {
        return;
    }

    char* page_ptr = (char*)(*addr);
    char* base = (char*)allocator->base_addr;
    size_t total_size = allocator->allowed_page_count * PAGE_SIZE;

    // Ensure the address is within the allocated range
    if (page_ptr < base || page_ptr >= (base + total_size)) {
        return;
    }

    // Calculate the page index
    uint64_t page_index = (page_ptr - base) / PAGE_SIZE;
    if (page_index >= allocator->allowed_page_count) {
        return;
    }

    uint64_t word = page_index / 64;
    uint64_t bit = page_index % 64;

    // Check if the page is actually allocated
    if (!(allocator->page_mask[word] & (1ULL << bit))) {
        return;
    }

    // Mark the page as free
    allocator->page_mask[word] &= ~(1ULL << bit);
    allocator->curr_page_count--;

    // Nullify the pointer
    *addr = NULL;
}

