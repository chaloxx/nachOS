#include "../userprog/syscall.h"

int length(char* st){
   int i = 0;
   for(;*(st+i) != '\0';i++);
   return i;
}

int print(char* msg){
  Write(msg,length(msg),1);
}


void main(int argc,char** argv){
  print("Soy escritor: ");
  print(argv[0]);
  print("\n");
  OpenFileId id = Open("../userland/pepe");
  Write(argv[0],1,id);		   
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

