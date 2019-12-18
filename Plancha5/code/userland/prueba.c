
#include "../userprog/syscall.h"
#include "string.c"


int main(){
  int nw = 10, nl = 10, i = 0;
  unsigned iters = nw+nl;
  int ret [iters];
  char* args[iters];

  for(; i < nw; i++){
	  print("Creando un escritor\n");
	  char letter = 97+i;
	  args[i] = &letter;
	  ret[i]  = Exec("writer",args+i);

  }
  
  char a = 'a';
  args[iters] = &a;
  ret[i] = Exec("removedor",args+iters);

  for(; i < iters; i++){
	  print("Creando un lector\n");
          char letter = 97+i;
	  args[i] = &letter;
	  ret[i] = Exec("reader",args+i);
  }
 //Remove("pepe") ;
 for(unsigned k = 0; k < iters;k++){
       print("Prueba hace join\n");
       Join(ret[k]); 
 }

 print("Prueba termino su trabajo\n");
 Halt();


}
