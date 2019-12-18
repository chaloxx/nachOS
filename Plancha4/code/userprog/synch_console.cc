#include "synch_console.hh"

static Semaphore* readAvail;
static Semaphore* writeDone;


static void ReadAvail(void *arg)
{
    readAvail->V();
}

static void WriteDone(void *arg)
{
    writeDone->V();
}


SynchConsole::SynchConsole(){
  console   = new Console(nullptr, nullptr, ReadAvail, WriteDone, 0);
  readAvail = new Semaphore("read avail", 0);
  writeDone = new Semaphore("write done", 0);
  l = new Lock(NULL); // El acceso a la consola es excluyente

}


SynchConsole::~SynchConsole(){
  delete console;
  delete readAvail;
  delete writeDone;
  delete l;
}


void SynchConsole::WriteConsole(int size,char* outBuffer){
  for(int i = 0; i < size; i++){
    char c = 0;
    c = *outBuffer++;
    l -> Acquire();
    console -> PutChar(c);  // Mandar a imprimir
    writeDone -> P();        // Esperar a que se imprima
    l -> Release();
    if( c == '\n') return; // Contar cuando se encuentra un salto de linea
  }

}

void SynchConsole::ReadConsole(int size,char* outBuffer){
  for(int i = 0; i < size;i++){
    char c = 0;
    l -> Acquire();
    readAvail->P();        // Esperar a que llegue un caracter
    c = console->GetChar();  // Almacenar caracter
    l -> Release();
    *outBuffer++ = c;
    if (c == '\n' || c == '\r') return;
  }
}
