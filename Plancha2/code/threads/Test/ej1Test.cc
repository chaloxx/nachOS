#include "../synch.hh"
#include "../thread.hh"
#define N_VISITANTES 100 // Visitantes por molinete
#define MOLINETES 5 // Cantidad de molinetes
extern Thread* currentThread;


//Test para Lock


Lock *g;
int visitantes = 0;


void molinete(void * name_)
{
  int n_visitantes =  100;
  int num_threads = 10;
  int i,j;
  char *name  = (char *) name_;
  for (i=0;i < N_VISITANTES;i++){
    g-> Acquire();
    currentThread -> Yield(); // Forzamos un cambio de contexto
    j = visitantes;
    currentThread -> Yield();
    visitantes = j + 1;
    DEBUG('s',"Visitantes : %d soy %s\n",visitantes,name);
    g-> Release();
    }
}




void ej1ATest(){
  g = new Lock(NULL);
  char **names = new  char* [MOLINETES];
  Thread *threads[MOLINETES];
  char v[10];

  for (int i = 0; i < MOLINETES; i++){
    sprintf(v,"hilo::%d",i);
    names[i] = new char [64];
    strcpy(names[i],v);
    threads[i] = new Thread(names[i],false,0);
    threads[i]->Fork(molinete,(void *) names[i]);

  }
}






/////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Test para conditions

#define MAX_BUFFER 1024 // Tamaño del buffer
#define DATOS_A_PRODUCIR 10000// Datos que se desean producir
// Con un valor mayor a 2 se rompe, no se porque
#define PEOPLE 6 // Cantidad de personas, de las cuales la mitad es productor y la otra mitad consumidor
int n_elementos = 0; //Numero de elementos en el buffer
int buffer[MAX_BUFFER]; // Buffer comun

Lock* r;
Condition* no_lleno;
Condition* no_vacio;




void productor(void*) { /* codigo del productor */
 int dato,i ,pos = 0;
 for(i=0; i < DATOS_A_PRODUCIR; i++ ) {
  dato = i; /* producir dato */
  DEBUG('c',"%s produce %d \n",currentThread-> GetName(),dato); /* produce dato */
  r-> Acquire(); /* acceder al buffer */
  while (n_elementos == MAX_BUFFER){ /* si buffer lleno */
   no_lleno-> Wait(); /* se bloquea */
  }
  buffer[pos] = i;
  pos = (pos + 1) % MAX_BUFFER;/*calculamos la posicion del siguiente elemento en el buffer*/
  n_elementos ++;
  no_vacio-> Signal(); /* si hemos creado un dato el buffer no estará vacio*/
  r-> Release();/*liberamos el mutex*/
 }
}

void consumidor(void*){
/* codigo del sonsumidor */

 int dato, i ,pos = 0;
 for(i=0; i < DATOS_A_PRODUCIR; i++ ) {
  r-> Acquire(); /* acceder al buffer */
  while (n_elementos == 0){ /* si buffer vacio */
   no_vacio-> Wait(); /* se bloquea */
  }
  dato = buffer[pos];
  DEBUG('c',"%s consume %d \n",currentThread-> GetName(), dato); /* consume dato */
  pos = (pos + 1) % MAX_BUFFER;/*calculamos la posicion del siguiente elemento en el buffer*/
  n_elementos --;
  no_lleno-> Signal(); /* buffer no lleno */
  r-> Release();
 }
}






void ej1BTest(){
  DEBUG('c',"Iniciando test para Conditions\n");
  char * nameC1 = new char [64];
  char * nameC2 = new char [64];
  strcpy(nameC1,"buffer no lleno");
  strcpy(nameC2,"buffer no vacio");
  r = new Lock(NULL);
  no_lleno = new Condition(nameC1,r);
  no_vacio = new Condition(nameC2,r);
  char *names[PEOPLE];
  Thread *threads[PEOPLE];
  char v[10];

  for (int i = 0; i < PEOPLE ; i++){
    sprintf(v,"hilo::%d",i);
    names[i] = new char [64];
    strcpy(names[i],v);
    threads[i] = new Thread(names[i],false,0);
    if (i % 2 == 0){
      threads[i]->Fork(productor,(void *) names[i]);
    }
    else{
      threads[i]->Fork(consumidor,(void *) names[i]);
    }


  }
}



