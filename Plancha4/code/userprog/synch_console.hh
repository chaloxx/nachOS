// Estructura para sincronizar el acceso a consola

#include "../machine/console.hh"
#include "../threads/synch.hh"

class SynchConsole{
 private:
   Console* console;
   Lock* l;
 public:
   SynchConsole();
   ~SynchConsole();
   void WriteConsole(int,char*);
   void ReadConsole(int,char*);
};
