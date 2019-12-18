#include "syscall.h"



int length(char* st){ 
    int i = 0; 
    for(;*(st+i) != '\0';i++); 
    return i; 
 } 


int print(char* msg){
    Write(msg,length(msg),1);
}



//Suponemos que str1 tiene sufieciente memoria para almacenar str2
int cpy(char* str1,const char* str2,unsigned numBytes){
	unsigned i = 0;
	for(;i < numBytes;i++)
		str1[i] = str2[i];
	return i;
}

int cat(char* str1,const  char* str2,unsigned numBytes){
    for(;str1[0] != '\0';++str1);
    unsigned i = 0;
    for(; i < numBytes;i++)
	    str1[i] = str2[i];
    return i;
}
