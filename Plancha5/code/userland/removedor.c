#include "syscall.h"
#include "string.c"


int main(){
  char* str = "test/pepe";
  print("Voy a borrar\n");
  Remove("test/pepe");
  print("Ya borre\n");
  Exit(1);
}
