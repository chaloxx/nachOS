
#include "../userprog/syscall.h"
#include "string.c"


int main(){
  int nw = 10, nl = 10 , i = 0;
  unsigned iters = nw+nl;
  int ret [iters];
  char letter = 'a';
  char* arg = &letter;
  for(; i < nw; i++){
	  print("Creando un escritor\n");
	  ret[i]  = Exec("../userland/writer",&arg);

  }

  for(; i < iters; i++){
	  print("Creando un lector\n");
	  ret[i] = Exec("../userland/reader",&arg);
  }

 for(unsigned k = 0; k < iters;k++){
       print("Prueba hace join\n");
       Join(ret[k]); 
 }
 print("Prueba termino su trabajo\n");
 Halt();


}
