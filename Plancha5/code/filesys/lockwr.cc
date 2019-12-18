#include "lockwr.hh"
#include "threads/thread.hh"
#include "lib/debug.hh"
extern Thread* currentThread;

LockWR::LockWR(const char* str){    
    in = 0;
    readerin = writerin = 0;
    sem = new Lock(nullptr);
    lockIn = new Lock(nullptr);
    cl = new Condition(nullptr,lockIn);
    qr = new Condition(nullptr,sem);
    qw = new Condition(nullptr,sem);
    strncpy(name,str,strlen(str)+1);
}



LockWR::~LockWR(){
	delete sem;
        delete lockIn;
        delete cl;
        delete qr;
        delete qw;	
}

void estado(unsigned a, unsigned b){
   DEBUG('c',"Hay %u lectores y %u escritores\n",b,a);
}

//Bloquear el archivo como lector o escritor
void LockWR::LockFile(RW q){
  sem -> Acquire();
  switch(q){
	  case Reader:		  
 		  while(writerin) // No puede leer si hay un escritor
		   qr -> Wait();
		  readerin++;		 
		  DEBUG('c',"lector entra\n");		 
		  break;
          case Writer:		  
		  while(writerin || readerin)// No puede escribir si hay un lector u otro escritor
		    qw -> Wait();
		  writerin++;
		  DEBUG('c',"escritor entra\n");
		  break;
  }
  estado(writerin,readerin);
  sem -> Release();
}

// Desbloquear el archivo  
void LockWR::UnlockFile(RW q){
  sem -> Acquire();
  switch(q){
	  case Reader:
		  readerin--;
		  DEBUG('c',"lector sale de %s\n",name);
		  qw -> Signal();
		  break;
          case Writer:
		  writerin--;
		  DEBUG('c',"escritor sale de %s\n",name);
		  qr -> Broadcast();
		  qw -> Signal();
		  break;
  }
  estado(writerin,readerin);
  sem -> Release();
}


void LockWR::ReaderLock() {
	LockFile(Reader);
}	


void LockWR::ReaderUnlock(){
	UnlockFile(Reader);
}


void LockWR::WriterLock(){
	LockFile(Writer);
}

void LockWR::WriterUnlock(){
	UnlockFile(Writer);
}


bool 
LockWR::Opener(){
	lockIn -> Acquire();
	if(close){
	  lockIn -> Release();
          return false;
	 }
         in+=1;
	 DEBUG('c',"Entre a %s in:%u\n",name,in);
         lockIn -> Release();
	 return true;
}

void LockWR::Closer(){
	lockIn -> Acquire();
	in-=1;
	DEBUG('c',"Me voy de %s in:%u\n",name,in);
	//Avisar que alguien cerró el archivo
	cl -> Signal();
	lockIn -> Release();
}
// No se puede borrar 2 veces un archivo
void LockWR::Remover(){
       ASSERT(!close);
       lockIn -> Acquire();
       // close = true para que no se pueda abrir el archivo
       close = true; 
       DEBUG('c',"Recien llego para borrar %s, in:%u\n",name,in);	       
       while(in){
               DEBUG('c',"Esperando para borrar %s , in:%u\n",name,in);	       
	       cl -> Wait();
       }

       lockIn -> Release();
}


