#include "transfer.hh"
#include "lib/utility.hh"
#include "threads/system.hh"


bool ReadStringFromUser(int userAddress, char *outString,
                        unsigned maxByteCount)
{
    ASSERT(userAddress != 0);
    ASSERT(outString != nullptr);
    ASSERT(maxByteCount != 0);
    bool b = false;
    unsigned count = 0;
    do {
        int temp;
        count++;
        int i = 0 ;
        for(; i < 4 && !b; i++ ){
          b = machine -> ReadMem(userAddress,1,&temp);
        }
        if ( i == 4) ASSERT(false);
       b = false;
       userAddress++;
        *outString = (unsigned char) temp;
    } while (*outString++ != '\0' && count < maxByteCount);

    return *(outString - 1) == '\0';
}

// TODO: complete...

void ReadBufferFromUser(int userAddress, char *outBuffer,unsigned byteCount){
  ASSERT(userAddress != 0);
  ASSERT(outBuffer != nullptr);
  int temp;
  bool b = false;
  do{
    int i = 0 ;
    for(;i < 4 && !b; i++ ){
      b = machine -> ReadMem(userAddress,1,&temp);
    }
    if ( i == 4) ASSERT(false);
  b = false;
   userAddress++;
   *outBuffer = (unsigned char) temp;
   byteCount--;
 }while(byteCount > 0 && *outBuffer++ != '\0');
}


void WriteStringToUser (const char *string , int userAddress ){
  ASSERT(userAddress != 0);
  ASSERT(string != 0);
  int temp;
  bool b = false;
  do{
    temp = *string++;
    int i = 0 ;
    for(; i < 4 && !b; i++ ){
      b = machine -> WriteMem(userAddress,1,temp);
    }
    if ( i == 4) ASSERT(false);
   b = false;
   userAddress++;
  }while(*string != '\0');
  temp = '\0';
  bool val;
  do{
   val = machine -> WriteMem(userAddress,1,temp);
 }while(!val);
}

void WriteBufferToUser (const char *buffer , int userAddress ,unsigned byteCount ){
  DEBUG('e',"Aca entra");
  DEBUG('e',"TEngo %s\n",buffer);
  ASSERT(buffer != 0);
  ASSERT(userAddress != 0);
  int temp;
  bool b = false;
  do{
    temp = *buffer++;
    int i = 0 ;
    for( ;i < 4 && !b; i++ ){
      b = machine -> WriteMem(userAddress,1,temp);
    }
    if ( i == 4) ASSERT(false);
    b = false;
    userAddress++;
    byteCount--;
  }while(*buffer != '\0' && byteCount > 0);
  temp = '\0';
  b = false;
  while(!b){
    b = machine -> WriteMem(userAddress,1,temp);
  }
}
