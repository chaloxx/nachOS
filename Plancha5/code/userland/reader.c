#include "../userprog/syscall.h"
#include "string.c"


void main(int argc, char** argv){
  char* letter = argv[0];
  print("Soy lector: ");
  print(letter);
  print("\n");
  unsigned size = 5;
  char buff;
  OpenFileId id = Open("test/pepe");
  if (id < 0){
    print("Lector no pudo abrir el archivo\n");
    Exit(1);
  }
  print("Lector abrio el archivo\n");
  for(int i = 0;i < size; i++){
          print("Voy a leer\n");
	  Read(&buff,1,id);
	  print("Lei:");
	  print(&buff);
	  print("\n");
  }
  Close(id);
  print("Lector termino su trabajo\n");
  Exit(1); 
 }

