#include "syscall.h"
#include "string.c"



int main(){
   print("Hola\n");
   Create("prueba/pepe");
   OpenFileId id = Open("prueba/pepe");
   char* str = "hola soy pepe";
   Write(str,length(str),id);
   Close(id);
   unsigned size = 50;
   char buff[size];
   id = Open("prueba/pepe");
   Read(buff,size,id);
   print("Esto es lo que lei:");
   print(buff);
   print("\n");
   Halt();
}
