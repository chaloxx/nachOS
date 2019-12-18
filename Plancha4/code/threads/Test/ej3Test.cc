
void iterar(void *arg)
{
  for(int i = 0; i < 10;i++){
    DEBUG('j',"Soy %s, iteracion %d\n",currentThread->GetName(),i);
    currentThread -> Yield();
  }
  currentThread->Finish(0);
}

void ej3Test(){
 DEBUG('j',"Iniciando test de ejercicio 3\n");
 char * name;
 Thread *t;
 int *j = new int;
 *j = 0;
 DEBUG('j',"Soy %s y j vale %d\n",currentThread->GetName(),*j);
 name = new char [64];
 strcpy(name,"Hilo 0");
 t = new Thread(name,true,0);
 t-> Fork(iterar,(void *) name);
 t-> Join(j);
 DEBUG('j',"Soy %s y j vale %d\n",currentThread->GetName(),*j);
}
