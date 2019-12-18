#include "../userprog/syscall.h"
#include "string.c"

   
void main(int argc, char** argv){
  char* letter = argv[0];
  print("Soy lector: ");
  print(letter);
  print("\n");
  unsigned size = 5;
  char buff;
  OpenFileId id = Open("../userland/pepe");
  for(int i = 0;i < size; i++){
          Read(&buff,1,id);
	  print("Lei:");
	  print(&buff);
	  print("\n");
  }
  Close(id);
  print("Lector termino su trabajo\n");
  Exit(1); 
 }

