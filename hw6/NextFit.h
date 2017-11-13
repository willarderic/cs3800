#include <iostream>
#include "AbstractAllocator.h"
#include <cstddef>
#include <stdexcept>

class NextFit : public AbstractAllocator
{
private:
    std::list<chunk>::iterator next;
public:
    NextFit() { 
      chunk c(0, poolSize);
      free.push_front(c);
      for (int i = 0; i < poolSize; i ++) {
        pool[i] = '0';
      }
      next = free.begin();
    }

    static bool compLocation(const chunk &c1, const chunk &c2) { 
      return c1.loc < c2.loc; 
    }
    
    void* alloc(size_t bytes) {
      std::list<chunk>::iterator it = next;
      bool iterating = true;
      
      while (iterating) {
        if (it->size >= bytes) {
          std::cout << "Allocating " << bytes << " bytes at memory location " 
               << it->loc << "." << std::endl;
          size_t allocation = it->loc;
          // insert this allocation into a map of allocations
          allocated.insert(std::pair<size_t, size_t>(allocation, bytes));
          if (it->size - bytes) {
            it->loc = it->loc + bytes;
            it->size = it->size - bytes;
            next = it;
          } else  {
            next = it;
            // we are deleting this iterator because 
            // there is no more memory at the location
            // we check if this or what should be the next iterator
            // is the end of the list
            
            next++;
            if (next == free.end()) {
              next = free.begin();
            }
            free.erase(it);
          }
          if (next == free.end()) {
              next = free.begin();
            }
          // return address of the memory allocated
          return &pool[allocation];
        }
        // increment and check if the iterator == next which 
        // is the iterator we also start at
        if (it == free.end())  {
          it = free.begin();
        }
        it++;
        if (it == free.end()) {
          it = free.begin();
        }
        if (it == next) {
          iterating = false;
        }
      }
      // if we never hit the return statement we never allocated
      std::cout << "Could not allocate " << bytes << " bytes." << std::endl;
      throw std::bad_alloc();
    }

    void dealloc(void* ptr) {
      // get index of where memory is allocated
      size_t index = (char *)ptr - &pool[0];
      // get number of bytes allocated there
      size_t bytes = allocated[index];
      
      std::cout << "Deallocating " << bytes << " bytes at memory location "
                << index << "." << std::endl;
      // for printing out prettily
      std::map<size_t, size_t>::iterator it;
      // get iterator at index to delete from allocations
      it = allocated.find(index);
      if (it != allocated.end())
        allocated.erase(it);
      // create a new free memory chunk and push onto list
      chunk c(index, bytes);
      free.push_back(c);
      // sort and then merge free memory chunks into one if there is no
      // allocated memory between them
      free.sort(compLocation);   
      merge();
    }

    void merge() {
      std::list<chunk>::iterator outer = free.begin();
      std::list<chunk>::iterator inner;
      while (outer != free.end()) {
        inner = free.begin();
        while (inner != free.end()) {
          // check to see if the free chunks are adjacent
          if ((outer->loc + outer->size) == inner->loc) { 
            // fill out the outer chunk with the size of inner          
            outer->size = outer->size + inner->size;
            // if the inner iterator is the next iterator
            // then we need to make the next iterator point to
            // the next thing so we don't delete it
            if (next == inner) {
              next++;
              // wrap around to beginning
              if (next == free.end()) {
                next = free.begin();
              }
            }      
            free.erase(inner++);
            continue;
          }
          inner++;
        }
        outer++;
      }
    }

    void fill(size_t loc, size_t bytes, bool alloc) {
      for (int i = loc; i < loc + bytes; i ++) {
        pool[i] = alloc ? '1': '0';
      }
    }
    void print() {
      std::list<chunk>::iterator it;
      std::cout << "Free -> [";
      for (it = free.begin(); it != free.end(); it ++) {
        std::cout << "(" << it->loc << ", " << it->size << "),";
      }
      std::cout << "]" << std::endl;
      for (int i = 0; i < poolSize; i++) {
        std::cout << pool[i];
      }
      std::cout << std::endl;
      
    }
};