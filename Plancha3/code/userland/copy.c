#include "syscall.h"
#include "string.c"


#define MAX_LEN 100



int main(int argc, char** argv){
 if (argc < 3 ){
	 print("Cantidad incorrecta de argumentos");
	 return 1;
 }
 char buffer[MAX_LEN];
 print("1\n");
 OpenFileId id1 = Open(argv[1]);
 Read(buffer,MAX_LEN,id1);
 Close(id1);
 print("Leyo: ");
 print(buffer);
 print("\n");
 Create(argv[2]);
 OpenFileId id2 = Open(argv[2]);
 Write(buffer,length(buffer),id2);
 Close(id2);
 print("2\n");
 Exit(1);
 return 1;
}
