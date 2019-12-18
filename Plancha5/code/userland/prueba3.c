
#include "../userprog/syscall.h"
#include "string.c"


int main(){
  ChangeDir("../..");
  Remove("pepe");
  Halt();


}
