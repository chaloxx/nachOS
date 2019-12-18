#include "../userprog/syscall.h"
#include "string.c"





void main(int argc,char** argv){
  print("Soy escritor: ");
  print(argv[0]);
  print("\n");
  OpenFileId id = Open("../userland/pepe");
  int size = 5;
  for(int i = 0;i < size;i++){	
          print ("Escribi:");
	  print(argv[0]);
	  print("\n");
  }

  Close(id);
  print("Escritor termino su trabajo\n");
  Exit(1); 
 }

