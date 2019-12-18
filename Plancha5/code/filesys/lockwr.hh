#include "../threads/synch.hh"
// Lock para sincronización de lectores y escritores dentro de un archivo abierto

enum RW {Reader,Writer};

class LockWR{

   public:
	   LockWR(const char*);
	   ~LockWR();	   
	   void WriterLock();
	   void ReaderLock();
	   void Closer();
	   bool Opener();
	   void WriterUnlock();
	   void ReaderUnlock();
	   void Remover();
         	   
  private:
           //Número de lectores dentro del archivo
           int readerin;
           //Número de escritores dentro del archivo
           int writerin;
	   // Número de hilos que tienen abierto el archivo
	   int in;
	   // Locks y conditions para sincronización
           Lock* sem,*lockIn;
           Condition *qr, *qw, *cl;
	   // La bandera close nos permite detectar si alguien va a borrar el archivo
	   // Si es así, no permitimos que nadie más lo abra
	   bool close;
	   //Métodos internos
	   void LockFile(RW);
           void UnlockFile(RW);
	   //Nombre del archivo
	   char name [100];

};
