#ifndef ABSTRACT_ALLOCATOR
#define ABSTRACT_ALLOCATOR

#include <list>
#include <cstddef>
#include <map>

// Data structure for a chunk of memory
struct chunk {
    // The location in memory where the start of the chunk of memory is
    size_t loc;

    // The size of the chunk of memory
    size_t size;
    chunk(size_t loc, size_t size) { this->loc = loc; this->size = size; }
};

/*
 *  AbstractAllocator Class
 *  Used for the basis of what every allocator we make should have
 *  Classes to be implemented including this: 
 *      - First-fit
 *      - Next-fit
 *      - Best-fit
 */
class AbstractAllocator 
{
protected:
    // The size of our "memory"
    static const size_t poolSize =  1024;

    // Our "memory"
    char pool[poolSize];

    // List of all the free chunks of memory
    std::list<chunk> free;
    // Map for the allocated chunks of memory
    std::map<size_t, size_t> allocated;

public:
    /*
     *  Function for allocating the specified number of bytes
     *  to be implemented specific to each type of allocation algorithm
     */
    virtual void* alloc(size_t bytes) = 0;

    /*
     *  Function for deallocating the memory at the position of pointer
     *  To be implemented specific to each type of allocation algorithm
     */
    virtual void dealloc(void* pointer) = 0;
};

#endif