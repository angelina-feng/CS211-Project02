/*main.cpp*/

/**
  * @brief simple debugger for nuPython programs.
  *
  * To run:
  *
  *   ./a.out filename.py
  *
  * @author original template by Prof. Joe Hummel
  * @author Aimee Darlene Izere
  * @author Lauren Jene
  * @author Angelina Feng 
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

void pg_command(STMT* p){
  programgraph_print(p);
}

void sm_command(RAM* r){
  ram_print(r);
}

void b_command(set<int>& breakpoints){
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

void rb_command(set<int>& breakpoints){
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

void cb_command(set<int>& breakpoints){
  breakpoints.clear();
  cout << "all breakpoints removed" << endl;
}

void lb_command(set<int>& breakpoints){
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


/**
  * @brief This helper function print what line  will execute
  * next from a given statement in the program graph
  * @param cur a pointer to a program statement 
  * @return void
*/

void what_line(struct STMT* cur)
{
  if (cur == nullptr)
  {
    cout << "execution has completed" << endl;
    return;
  }

  struct PG_LINKS broken_links = break_pg(cur);
  programgraph_print(cur);
  repair_pg(cur,broken_links);
  return;
}

/**
  * @brief Steps through and executes a program statement by statement
  * @param cur a pointer to a program statement 
  * @param memory a pointer to the nupython memory
  * @param program_complete keeps track is we've reached the end of the program graph
  * @param breakpoint_hit keeps track if the current line was previously set a a break point
  * @param breakpoints returns all the current breakpoints set in a program graph as a set
  * @return void
*/

struct STMT* step_execute(struct STMT* cur, 
                          struct RAM* memory, 
                          bool& program_complete, 
                          bool& breakpoint_hit,
                          set<int>& breakpoints)
{
  if (cur == nullptr)
  {
    program_complete = true; // need to update flag here
    cout << "execution has completed" << endl;
    return nullptr;
  }

  // check if the stmt has a breakpoint
  bool has_breakpoint = breakpoints.contains(cur->line) ;

  if (has_breakpoint && !breakpoint_hit)
  {
    cout << "breakpoint hit at line " << cur->line << endl;
    breakpoint_hit = true;
    return cur;
  }

  // 1. Break the program graph
  struct PG_LINKS broken_links = break_pg(cur);

  // 2. consume any lingering keyboard input by calling std::getline( )
  // , just in case the current stmt is a call to input( )
  
  //3. Execute
  // If execution fail, output error and leave the stmt as it is 
  // Execute does all of that
  
  // Move to the next statement, advance cur to next
 struct STMT* next_stmt = cur;

  // 3.a. Execute for straight statements
  if ((cur->stmt_type == STMT_ASSIGNMENT) ||
      (cur->stmt_type == STMT_FUNCTION_CALL) ||
      (cur->stmt_type == STMT_PASS))
      {
        struct ExecuteResult step_result = execute(cur, memory);
        //Check if execution succeded
        if (!step_result.Success)
          {next_stmt = cur;}
        else
        {next_stmt =  broken_links.next;}
      }

  // 3.b. Execute for while loops
  else if (cur->stmt_type == STMT_WHILE_LOOP)
  {
    struct RAM_VALUE* condition_eval = execute_expr(cur, memory, cur->types.while_loop->condition);
    if (condition_eval == nullptr)
    {
      next_stmt  = cur;
    }
    else
    {
      if (condition_eval->types.i != 0)
      {
        next_stmt = broken_links.loop_body;
      }
      else
        next_stmt = broken_links.next;
    }
    if (condition_eval != nullptr)
     {ram_free_value(condition_eval);}
  } 

 // 3.b. Execute for if else statements
  else if (cur->stmt_type == STMT_IF_THEN_ELSE)
  {
    struct RAM_VALUE* condition_eval = execute_expr(cur, memory, cur->types.if_then_else->condition);
    if (condition_eval == nullptr)
    {
      next_stmt  = cur;
    }
    else
    {
      if (condition_eval->types.i != 0)
      {
        next_stmt = broken_links.true_path;
      }
       // False path
      else
        next_stmt = broken_links.false_path;
    }
    if (condition_eval != nullptr)
     {ram_free_value(condition_eval);}
  }
 // 4. Repair and free memory
  repair_pg(cur,broken_links);


  // if the user inputs "s" again, you now execute the current statement and move past the breakpoint.
  if (next_stmt != cur)
  {breakpoint_hit = false; }

  if (next_stmt == nullptr)
    {
      program_complete = true;
      cout << "program completed" << endl;
    }
  return next_stmt; 
}


//11 (r)
void r_command(struct STMT*& cur, struct RAM* mem, bool& program_complete,
                bool& breakpoint_hit, set<int>& breakpoints){
  if(cur == nullptr){
    cout << "execution has completed" << endl;
    return;
  }

  while (true){
    if(cur == nullptr){ //break if nullptr (base case for while loop)
      break;
    }

    struct STMT* prev_cur = cur; //update pointer

    cur = step_execute(cur, mem, program_complete, breakpoint_hit, breakpoints);

    if(breakpoint_hit || cur == prev_cur){ //if flag positive, break (stop at breakpoints) or statement didn't advance (just in case)
      break;
    }

  }

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

  STMT* cur = program;
  bool program_complete = false;
  bool breakpoint_hit = false;
  set<int> breakpoints;

  while (true) {

    cout << endl;
    cout << "Enter a command, h for help >" << endl;
    if(!(cin >> cmd)){//this line was causing it to timeout (didn't check if no more input)
      if(cin.eof()){
        break;
      }
      cin.clear();
      continue;
    }

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
      pg_command(program);

    }
    else if(cmd == "sm"){
      sm_command(memory);
    }
    else if(cmd == "b"){
      b_command(breakpoints);
    }
    else if(cmd == "rb"){
      rb_command(breakpoints);
    }
    else if(cmd == "cb"){
      cb_command(breakpoints);
    }
    else if(cmd == "lb"){
      lb_command(breakpoints);
    }

    // "w" command  -- w: what line will execute next?
    else if(cmd == "w")
    { what_line(cur);}

    // "s" command  -- s: step through and execute
    else if(cmd == "s")
    {
      string keyboard_input ;
      getline(cin,keyboard_input);

      cur = step_execute(cur, 
                        memory, 
                        program_complete, 
                        breakpoint_hit, 
                        breakpoints);
    }

    else if(cmd == "r"){
      string keyboard_input ;
      getline(cin,keyboard_input);
      r_command(cur, memory, program_complete, breakpoint_hit, breakpoints);
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
