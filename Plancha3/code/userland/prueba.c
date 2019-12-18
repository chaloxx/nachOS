
#include "../userprog/syscall.h"


int length(char* st){ 
    int i = 0; 
    for(;*(st+i) != '\0';i++); 
    return i; 
 } 


int print(char* msg){
    Write(msg,length(msg),1);
}


int main(){
  int nw = 10, nl = 10, i = 0;
  unsigned iters = nw+nl;
  int ret [iters];
  char* args[iters];
  for(; i < nw; i++){
	  print("Creando un escritor\n");
	  char letter = 97+i;
	  args[i] = &letter;
	  ret[i]  = Exec("../userland/writer",args+i);

  }

  for(; i < iters; i++){
	  print("Creando un lector\n");
          char letter = 97+i;
	  args[i] = &letter;
	  ret[i] = Exec("../userland/reader",args+i);
  }

 for(unsigned k = 0; k < iters;k++){
       print("Prueba hace join\n");
       Join(ret[k]); 
 }
 print("Prueba termino su trabajo\n");
 Halt();


}
