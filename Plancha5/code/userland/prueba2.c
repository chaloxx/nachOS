
#include "../userprog/syscall.h"
#include "string.c"


int main(){
  int nw = 10, nl = 10, i = 0;
  unsigned iters = nw+nl;
  int ret [iters];
  char* args[iters];
  ChangeDir("../..");
  Create("pepe");

  Halt();


}
