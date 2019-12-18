#include "../userprog/syscall.h"
#include "string.c"

int main(){
  unsigned iters = 10;
  int ret [iters]; 
  char* arg = "Hola"; 
  for(int i = 0; i < iters; i++){
	  print("Creando matmult\n");
	  //char letter = 97+i;
	  //arg[i] = &letter;
	  ret[i]  = Exec("../userland/matmult",&arg);
  }

  for(unsigned k = 0; k < iters;k++){
       print("Prueba hace join\n");
       Join(ret[k]); 
 }
 print("Prueba termino su trabajo\n");
 Halt();


}
