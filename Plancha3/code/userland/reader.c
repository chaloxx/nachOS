#include "../userprog/syscall.h"

int length(char* st){
   int i = 0;
   for(;*(st+i) != '\0';i++);
   return i;
}

int print(char* msg){
  Write(msg,length(msg),1);
}


void main(int argc, char** argv){
  char* letter = argv[0];
  print("Soy lector: ");
  print(letter);
  print("\n");
  unsigned size = 5;
  char buff;
  OpenFileId id = Open("../userland/pepe");
  print("Abri el archivo\n");
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

