#include "syscall.h"
#include "string.c"



int main(int argc,char** argv){ 
   if (argc < 2){
      print("Cantidad incorrecta de argumentos\n");
      return 1;
   }
   unsigned size = 0;
   for(unsigned i = 1; i < argc ;i++)
	   size+=length(argv[i]);   
   char buff[size+argc+(argc-2)];   
   buff[0] = '\0';
   for(unsigned i = 1; i < argc;i++){
	   cat(buff,argv[i],length(argv[i])+1);
	   cat(buff," ",2);
     }
	   
   print(buff);
   print("\n");
}
