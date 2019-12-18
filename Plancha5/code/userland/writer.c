#include "syscall.h"
#include "string.c"

void main(int argc,char** argv){
  print("Soy escritor: ");
  print(argv[0]);
  print("\n");
  OpenFileId id = Open("test/pepe");
  if (id < 0){
    print("Escritor no pudo abrir el archivo\n");
    Exit(1);
  }

  print("Escritor abrio el archivo\n");
  int size = 5;
  for(int i = 0;i < size;i++){	
	  Write(argv[0],1,id);
          print ("Escribi:");
	  print(argv[0]);
	  print("\n");
  }

  Close(id);
  print("Escritor termino su trabajo\n");
  Exit(1); 
 }

