#include "syscall.h"
#include "string.c"



int main(){
  print("Hola\n");
  ChangeDir("..");
  Create("pepa");
  Halt();
}
