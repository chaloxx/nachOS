#include "../userprog/syscall.h"
#include "string.c"


int main(){
    print("Comienza test\n");
   Create("pepe2");
   //Las cosas pueden fallar cuando size es relativamente grande
   unsigned size = 500;
   OpenFileId id = Open("pepe2");
   for(unsigned i = 0; i < size;i++)
   	   Write("a",1,id);
   Close(id);
   id = Open("pepe2");
   char buf[size];
   print("Se escribiooooo:\n");
   for(unsigned i = 0;i < size;i++)
           Read(buf+i,1,id);
   
   //print("????????????\n");
   //Read(buf,size,id);
   print(buf);
   print("\n");
   print("Termino\n"); 
   print("Seguro\n");
   Close(id);
   //Remove("pepe2");
   Halt();
}
