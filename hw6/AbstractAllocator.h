#ifndef ABSTRACT_ALLOCATOR
#define ABSTRACT_ALLOCATOR

#include <list>

// Data structure for a chunk of memory
struct chunk {
    // The location in memory where the start of the chunk of memory is
    void* loc;

    // The size of the chunk of memory
    size_t size;
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
    list<chunk> freeMem; 

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