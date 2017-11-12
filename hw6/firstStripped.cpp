#include<iostream>
#include<stdexcept>
#include<forward_list>
#include<list>
#include<tuple>
#include<fstream>
#include<vector>

#include "FirstFit.cpp"

using namespace std;

int main()
{
  FirstFit a;
  vector<void*> allocations;
  ifstream trace("trace.txt");
  int tmp;
  char op;
  int operand;
  try{
    while(trace >> tmp && trace >> op && trace >> operand)
    {
      switch(op)
      {
        case 'a':
          allocations.push_back(a.alloc(operand));
          break;
        case 'd':
          a.dealloc(allocations[operand]);
          allocations[operand] = nullptr;
          break;
      }
      a.print();
      std::string tmp;
      std::cin >> tmp;
    }
  }
  catch(bad_alloc& b)
  {
    cout << "Too much stuff! " << b.what() << endl;
    cout << op << " " << operand << endl;
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

