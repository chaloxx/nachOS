#include "../thread.hh"
extern Thread* currentThread;

#define NUM_THREADS 1

void iterar(void *arg)
{
  for(int i = 0; i < 10;i++){
    DEBUG('j',"Soy %s, iteracion %d\n",currentThread->GetName(),i);
    currentThread -> Yield();
  }
  currentThread->Finish();
}

void ej3Test(){
  DEBUG('j', "Comienza test de ejercicio 3\n");
  char * names[NUM_THREADS];
  Thread *threads[NUM_THREADS];
  int res;
  for (int i = 0 ; i < NUM_THREADS; i++){
    names[i] = new char [10];
    sprintf(names[i],"hilo::%d",i);
    threads[i] = new Thread(names[i],true,0);
    threads[i]->Fork(iterar,(void *) names[i]);
    }
  for(unsigned i = 0;i < NUM_THREADS;i++){
	  DEBUG('j',"iter:%u\n",i);
	  threads[i] -> Join(&res);	 
  }
  /*for (unsigned i = 0;i < NUM_THREADS;i++){	  
         delete threads[i];
         DEBUG('j',"Aca llego\n");
  }*/

}

