#include<iostream>
#include<stdexcept>
#include<forward_list>
#include<list>
#include<tuple>
#include<fstream>
#include<vector>

#include "FirstFit.h"
#include "BestFit.h"
#include "NextFit.h"

using namespace std;

int main()
{
  // make an allocator pointer to point to each type of allocator
  AbstractAllocator* allocator;
  vector<void*> allocations;
  
  int tmp;
  char op;
  int operand;
  size_t max, avg, min, free, numChunks;

  //open file
  ifstream trace;
  ofstream file;
  string fileName;
  for (int i = 0; i < 3; i ++) {
    switch (i) {
      case 0: allocator = new FirstFit();
              fileName = "firstFit.csv";
              break;
      case 1: allocator = new BestFit();
              fileName = "bestFit.csv";
              break;
      case 2: allocator = new NextFit();
              fileName = "nextFit.csv";
              break;
    }
    trace.open("trace.txt");   
    file.open(fileName);
    file << "Total Free, Max Free, Avg Free, Min Free, Num Chunks, \n";
    try{
      while(trace >> tmp && trace >> op && trace >> operand)
      {
        switch(op)
        {
          case 'a':
            //start get measurements

            free = allocator->totalFree();
            max = allocator->maxFree();
            avg = allocator->avgFree();
            min = allocator->minFree();
            numChunks = allocator->numChunks();
            file << free <<","<< max <<","<< avg <<","<< min << "," << numChunks <<",\n";      
            //end get measurements
            allocations.push_back(allocator->alloc(operand));
            break;
          case 'd':
            //start get measurements
            free = allocator->totalFree();
            max = allocator->maxFree();
            avg = allocator->avgFree();
            min = allocator->minFree();
            numChunks = allocator->numChunks();
            file << free <<","<< max <<","<< avg <<","<< min << "," << numChunks <<",\n";       
            //end get measurements
            allocator->dealloc(allocations[operand]);
            allocations[operand] = nullptr;
            break;
        }
      }
    }
    catch(bad_alloc& b)
    {
      cout << "Too much stuff! " << b.what() << endl;
      cout << op << " " << operand << endl;
    }
    // close file and delete allocator so no memory leak
    file.close();
    trace.close();
    allocations.clear();
    delete allocator;
  }
  for(int i = 0; i < allocations.size(); i++)
  {
    if(allocations[i] != nullptr)
    {
      cout << i << " " << allocations[i] << endl;
    }
  }
  // ofstream stats("first_stats.csv");
  // a.print_stats(stats);
  return 0;
}