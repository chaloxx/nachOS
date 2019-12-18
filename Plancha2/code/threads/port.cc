#include "port.hh"

Port::Port(){
 //
 l = new Lock(NULL);
 // Lista de mensajes
 buzon = new List<Mensaje*>;
 //Condiciones
 esperarAlguien = new Condition(NULL,l);
 esperarMensaje = new Condition(NULL,l);

}

Port::~Port(){
  delete l;
  delete buzon;
  delete esperarAlguien;
  delete esperarMensaje;
}




void Port::Send(int message){
 l -> Acquire(); // Proteger buzon
 Mensaje * m = new Mensaje;
 m -> msg = message; // Guardar mensaje
 m -> enviado = false; // Todavia no fue enviado
 buzon -> Append(m); // Entregar mensaje
 DEBUG('p',"%s envio %d \n",currentThread-> GetName(),message);
 esperarMensaje -> Signal(); // Avisa que llego un mensaje
 while(! m -> enviado){
    esperarAlguien -> Wait(); // Esperar que alguien tome algun mensaje
 }
 l -> Release(); // Liberar buzon
}



void Port::Receive(int* message){
 DEBUG('p',"LLego %s para recibir mensaje\n",currentThread->GetName());
 l -> Acquire(); // Proteger buzon
 DEBUG('p',"%s esperando mensaje \n",currentThread-> GetName());
 while(buzon->IsEmpty()){
   esperarMensaje -> Wait();
 }
 Mensaje * m = buzon -> Pop(); // Toma el mensaje
 *message = m -> msg; // Guardar mensaje
 m -> enviado = true; // Se envío mensaje
 DEBUG('p',"%s recibio %d \n",currentThread-> GetName(),*message);
 esperarAlguien -> Signal(); // Avisar que alguien tomo algún mensaje
 l -> Release(); // Liberar buzon
}
