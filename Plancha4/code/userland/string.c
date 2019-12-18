#include "syscall.h"



int length(char* st){ 
    int i = 0; 
    for(;*(st+i) != '\0';i++); 
    return i; 
 } 


int print(char* msg){
    Write(msg,length(msg),1);
}



