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

// Crear consolar sincronizada
//
SynchConsole::SynchConsole(){
  console   = new Console(nullptr, nullptr, ReadAvail, WriteDone, 0);
  readAvail = new Semaphore("read avail", 0);
  writeDone = new Semaphore("write done", 0);
  lReader = new Lock(NULL); // A lo sumo puede haber un escritor y un lector
  lWriter = new Lock(NULL);
}


SynchConsole::~SynchConsole(){
  delete console;
  delete readAvail;
  delete writeDone;
  delete lReader;
  delete lWriter;
}


void SynchConsole::WriteConsole(int size,char* outBuffer){
  for(int i = 0; i < size; i++){
    char c = 0;
    c = *outBuffer++;
    lWriter -> Acquire(); //
    console -> PutChar(c);  // Mandar a imprimir
    writeDone -> P();        // Esperar a que se imprima
    lWriter -> Release();
    if( c == '\n') return; // Si se encuentra un salto de linea
                           // terminar escritura
  }

}

void SynchConsole::ReadConsole(int size,char* outBuffer){
  for(int i = 0; i < size;i++){
    char c = 0;
    //lReader -> Acquire();
    readAvail->P();        // Esperar a que llegue un caracter
    c = console->GetChar();  // Almacenar caracter
    //lReader -> Release();
    *outBuffer++ = c;
    if (c == '\n' || c == '\r') return;
  }
}
