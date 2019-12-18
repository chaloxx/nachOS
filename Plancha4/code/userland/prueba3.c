#include "../userprog/syscall.h"
#include "string.c"

int main(){
  unsigned iters = 10;
  int ret [iters]; 
  char* arg = "Hola"; 
  for(int i = 0; i < iters; i++)
	  ret[i]  = Exec("../userland/sort",&arg);
  

  for(unsigned k = 0; k < iters;k++){
       print("Prueba hace join\n");
       Join(ret[k]); 
 }
 print("Prueba termino su trabajo\n");
 Halt();


}
