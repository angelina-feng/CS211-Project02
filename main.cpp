/*main.cpp*/

/**
  * @brief simple debugger for nuPython programs.
  *
  * To run:
  *
  *   ./a.out filename.py
  *
  * @author original template by Prof. Joe Hummel
  * @author ?
  * @author ?
  * @author ?  
  * @note Northwestern University
  */

#include <iostream>
#include <string>
#include <set>

using std::cin;
using std::cout;
using std::endl;
using std::string;
using std::getline;
using std::stoi;
using std::set;

#include "pgutil.h"

//
// Since we are working in C++, we have to tell the
// C++ compiler that the functions in the provided
// nuPython code are C functions:
//
extern "C" {
  #include "tokenqueue.h"
  #include "parser.h"
  #include "programgraph.h"
  #include "ram.h"
  #include "execute.h"
}


/**
  * @brief help command
  *
  * @return void
  */
void help_command()
{
  cout << "Available commands:" << endl;
  cout << "h -> help with list of commands" << endl;
  cout << "q -> quit the debugger" << endl;
  cout << "b n -> set breakpoint at line n" << endl;
  cout << "rb n -> remove breakpoint at line n" << endl;
  cout << "cb -> clear all breakpoints" << endl;
  cout << "lb -> list all breakpoints" << endl;
  cout << "pg -> print entire program graph" << endl;
  cout << "r -> run the program from the current stmt" << endl;
  cout << "s -> step by executing the current stmt" << endl;
  cout << "sm -> show memory contents" << endl;
  cout << "w -> what line will execute next?" << endl;
}


/**
  * @brief main
  *
  * usage: ./a.out filename.py
  * 
  * If a filename is given, the file is opened and serves as
  * input to the debugger.
  *
  * @param argc # of arguments to program
  * @param argv Array of actual arguments
  * @return 0 Success
  */
int main(int argc, char* argv[])
{
  FILE* input = nullptr;

  cout << "** Simple gdb for nuPython **" << endl;

  if (argc < 2) {
    //
    // missing filename:
    //
    cout << "usage: ./a.out filename.py" << endl;
    return 0;
  }

  //
  // assume 2nd arg is a nuPython file, try to open:
  //
  char* filename = argv[1];

  input = fopen(filename, "r");

  if (input == nullptr) // unable to open:
  {
    cout << "**ERROR: unable to open input file "
         << "'" << filename << "' for input."
         << endl;

    return 0;
  }

  //
  // command loop:
  //
  string cmd;

  struct TokenQueue* tokens = parser_parse(input);
  if(tokens == nullptr){
        fclose(input);
        return 0;
      }
  STMT* program = programgraph_build(tokens);
  RAM* memory = ram_init();

  set<int> breakpoints;

  while (true) {

    cout << endl;
    cout << "Enter a command, h for help >" << endl;
    cin >> cmd;

    if (cmd == "q") {
      //
      // quit:
      //
      break;
    }
    else if (cmd == "h") { 
      help_command();
    }
    else if(cmd == "pg"){
      programgraph_print(program);

    }
    else if(cmd == "sm"){
      ram_print(memory);
    }
    else if(cmd == "b"){
      int breakpoint;
      cin >> breakpoint;

      if(breakpoints.contains(breakpoint)){
        cout << "breakpoint exists" << endl;
      }
      else{
        breakpoints.insert(breakpoint);
        cout << "breakpoint set" << endl;
      }
    }
    else if(cmd == "rb"){
      int breakpoint;
      cin >> breakpoint;
      if(!breakpoints.contains(breakpoint)){
        cout << "no such breakpoint" << endl;
      }
      else{
        breakpoints.erase(breakpoint);
        cout << "breakpoint removed" << endl;
      }
    }
    else if(cmd == "cb"){
      breakpoints.clear();
      cout << "all breakpoints removed" << endl;
    }
    else if(cmd == "lb"){
      cout << "breakpoints: ";
      if(breakpoints.empty()){
        cout << "none" << endl;;
      }
      else{
        for (int b : breakpoints){
          cout << b;
          if(b != *breakpoints.rbegin()){
            cout << ", ";
          }
        }
        cout << endl;
      }
    }
    else {
      cout << "unknown command" << endl;
    }

  }//while

  //
  // done: free data structures, close file, and return:
  //
  cout << endl;
  cout << "** Done **" << endl;

  programgraph_destroy(program);
  tokenqueue_destroy(tokens);
  ram_destroy(memory);

  fclose(input);

  return 0;
}
