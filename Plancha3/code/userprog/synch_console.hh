// Estructura para sincronizar el acceso a consola

#include "../machine/console.hh"
#include "../threads/synch.hh"

class SynchConsole{
 private:
   Console* console;
   Lock* lWriter;
   Lock* lReader;
 public:
   SynchConsole();
   ~SynchConsole();
   void WriteConsole(int,char*);
   void ReadConsole(int,char*);
};
