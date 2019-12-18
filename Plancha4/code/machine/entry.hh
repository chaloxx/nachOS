class Thread;
//Estructura para el coremap
struct Entry{
 //Quién ocupa la página?
 Thread* process;
 // Qué página virtual es?
 unsigned vpn;
 // Está siendo copiada a swap por otro proceso?
 bool inSwap;
  };

