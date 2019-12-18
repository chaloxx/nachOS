#include "../port.hh"
#include "../thread.hh"
#include <unistd.h>

#define MAX 3
#define NUM_THREADS 6




Port* p;

void enviar(void *arg)
{
  int i = *(int*)arg;
  sleep(random()%MAX);
  p-> Send(i);
}

void recibir(void *arg)
{
  int* i = new int;
  usleep(random()%MAX);
  p-> Receive(i);
}

void ej2Test(){
  DEBUG('p', "Comienza test de ejercicio 2\n");
  p = new Port();
  char * names[NUM_THREADS];
  Thread *threads[NUM_THREADS];
  char v[10];
  int *mjes = new int[NUM_THREADS];

  for(int i = 0;i < NUM_THREADS;i++){
     mjes[i] = i;
  }

  for (int i = 0 ; i < NUM_THREADS; i++){
    sprintf(v,"Hilo %d",i);
    names[i] = new char [64];
    strcpy(names[i],v);
    threads[i] = new Thread(names[i],false,0);
    if (i%2 == 0){
      threads[i]->Fork(enviar,(void *) (mjes+i));
    }
    else{
      DEBUG('p',"Aca entra\n");
      threads[i]->Fork(recibir,(void *) names[i]);
    }
  }
}
