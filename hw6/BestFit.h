#include <iostream>
#include "AbstractAllocator.h"
#include <cstddef>
#include <stdexcept>

class BestFit : public AbstractAllocator
{
public:
    BestFit() { 
      chunk c(0, poolSize);
      free.push_front(c);
      for (int i = 0; i < poolSize; i ++) {
        pool[i] = '0';
      }
    }

    static bool compLocation(const chunk &c1, const chunk &c2) { 
      return c1.loc < c2.loc; 
    }
    
    void* alloc(size_t bytes) {
      std::list<chunk>::iterator it;
      std::list<chunk>::iterator minit;
      int min = 1024;
      bool flag = false;
      for (it = free.begin(); it != free.end(); it++) {
        if (it->size >= bytes) {
            if (it->size - bytes < min) {
                min = it->size;
                minit = it;
                flag = true;
            }
        }
      }
      if (flag) {
        std::cout << "Allocating " << bytes << " bytes at memory location " 
        << minit->loc << "." << std::endl;
        size_t allocation = minit->loc;
        allocated.insert(std::pair<size_t, size_t>(allocation, bytes));
        if (minit->size - bytes) {
          minit->size = minit->size - bytes;
          minit->loc = minit->loc + bytes;
        } else {
          free.erase(minit);
        }
        return &pool[allocation];
      } else  {
        std::cout << "Could not allocate " << bytes << " bytes." << std::endl;
        throw std::bad_alloc();
      }
    }

    void dealloc(void* ptr) {
      size_t index = (char *)ptr - &pool[0];
      size_t bytes = allocated[index];
      std::cout << "Deallocating " << bytes << " bytes at memory location "
                << index << "." << std::endl;
      std::map<size_t, size_t>::iterator it;
      it = allocated.find(index);
      if (it != allocated.end())
        allocated.erase(it);
      chunk c(index, bytes);
      free.push_back(c);
      free.sort(compLocation);
      merge();
    }

    void merge() {
      std::list<chunk>::iterator outer = free.begin();
      std::list<chunk>::iterator inner;
      while (outer != free.end()) {
        inner = free.begin();
        while (inner != free.end()) {
          std::cout << (outer->loc + outer->size) << ", " << inner->loc << std::endl;
          if ((outer->loc + outer->size) == inner->loc) {           
            outer->size = outer->size + inner->size;      
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
      for (int i = 0; i < poolSize; i++) {
        std::cout << pool[i];
      }
      std::cout << std::endl;
    }
};