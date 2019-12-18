/// Entry points into the Nachos kernel from user programs.
///
/// There are two kinds of things that can cause control to transfer back to
/// here from user code:
///
/// * System calls: the user code explicitly requests to call a procedure in
///   the Nachos kernel.  Right now, the only function we support is `Halt`.
///
/// * Exceptions: the user code does something that the CPU cannot handle.
///   For instance, accessing memory that does not exist, arithmetic errors,
///   etc.
///
/// Interrupts (which can also cause control to transfer from user code into
/// the Nachos kernel) are handled elsewhere.
///
/// For now, this only handles the `Halt` system call.  Everything else core-
/// dumps.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2019 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "transfer.hh"
#include "syscall.h"
#include "threads/system.hh"
#include <unistd.h>
#include "args.hh"
#include <stdlib.h>


#define FILE_PATH_MAX_LEN 100
#define FILE_MAX_LEN 1000
#define FILE_NAME_MAX_LEN 1000

static void
IncrementPC()
{
    unsigned pc;

    pc = machine->ReadRegister(PC_REG);
    machine->WriteRegister(PREV_PC_REG, pc);
    pc = machine->ReadRegister(NEXT_PC_REG);
    machine->WriteRegister(PC_REG, pc);
    pc += 4;
    machine->WriteRegister(NEXT_PC_REG, pc);
}

/// Do some default behavior for an unexpected exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.






// Libera una pagina determinada por el algoritmo del reloj en el marco físico
// copiando su contenido a swap si debe



static void
DefaultHandler(ExceptionType et)
{
    int exceptionArg = machine->ReadRegister(2);
    switch (et){
	 case PAGE_FAULT_EXCEPTION :{
           // vpn buscada
           unsigned vpn = machine -> ReadRegister(BAD_VADDR_REG) / PAGE_SIZE;
           DEBUG('u',"Se solicita vpn %u soy %p\n",vpn, currentThread);
           AddressSpace* space = currentThread -> space;
           space -> FindVPN(vpn);
           break;
          }

         case READ_ONLY_EXCEPTION :{
           DEBUG('r',"Read only exception\n");
           ASSERT(false);
         }


           default:
             fprintf(stderr, "Unexpected user mode exception: %s, arg %d.\n",
                    ExceptionTypeToString(et), exceptionArg);
             ASSERT(false);
         }


}

/// Handle a system call exception.
///
/// * `et` is the kind of exception.  The list of possible exceptions is in
///   `machine/exception_type.hh`.
///
/// The calling convention is the following:
///
/// * system call identifier in `r2`;
/// * 1st argument in `r4`;
/// * 2nd argument in `r5`;
/// * 3rd argument in `r6`;
/// * 4th argument in `r7`;
/// * the result of the system call, if any, must be put back into `r2`.
///
/// And do not forget to increment the program counter before returning. (Or
/// else you will loop making the same system call forever!)

//Función tonta para fork
void Dummy(void* arg){
  char** argv = (char**) arg;
  currentThread->space->InitRegisters();//Inicia registros para nuevo proceso
  //currentThread->space->RestoreState();//Cambia la tabla de paginación de la maquina
  int argc = 0;
  for(; argv[argc] != 0; argc++);
  DEBUG('u',"Tengo %u argumentos\n",argc);
  int add = WriteArgs(argv); //Escribir argumentos en pila
  machine -> WriteRegister(4,argc);
  machine -> WriteRegister(5,add);
  machine->Run();//Saltar a programa de usuario
  ASSERT(false);
}



static void
SyscallHandler(ExceptionType _et)
{
    int scid = machine->ReadRegister(2);
    DEBUG('a',"Syscall : %d\n",scid);
    switch (scid) {

        case SC_HALT:
            DEBUG('a', "Shutdown, initiated by user program.\n");
            interrupt->Halt();
            break;

        case SC_CREATE: {
            int filenameAddr = machine->ReadRegister(4);
            if (filenameAddr == 0){
              DEBUG('a', "Error: address to filename string is null.\n");
              break;
            }


            char fileName[FILE_NAME_MAX_LEN];
            if (!ReadStringFromUser(filenameAddr, fileName, FILE_NAME_MAX_LEN)){
              DEBUG('a', "Error: filename string too long (maximum is %u bytes).\n",FILE_NAME_MAX_LEN);
              break;
            }

            fileSystem -> Create(fileName,FILE_MAX_LEN);
            break;
        }

        case SC_OPEN: {
          // Leer registro v0
          int userBuffer = machine -> ReadRegister(4);
          //Si el puntero es nulo tirar error
          if (userBuffer == 0){
            DEBUG('a',"Ruta inválida\n");
            break;
          }
          //Almacenar ruta
          char path [FILE_PATH_MAX_LEN];
          ReadBufferFromUser(userBuffer,path,FILE_PATH_MAX_LEN);
          //Abrir archivo
          DEBUG('b',"Se intenta abrir el archivo %s soy %p \n",path,currentThread);
          OpenFile* of = fileSystem -> Open(path);
	  ASSERT(of != nullptr);
	  DEBUG('b',"Tengo of:%p soy %p\n",of,currentThread);
          OpenFileId id = currentThread -> AddToTable(of);
	  DEBUG('a',"Se obtuvo el fd %u\n",id);
          //Retornar id
          machine -> WriteRegister(2,id);
          break;
        }

        case SC_READ: {
           DEBUG('u',"Llega a read\n");
           int userBuffer = machine -> ReadRegister(4);
	   DEBUG('u',"1\n");
           if (userBuffer == 0){
             DEBUG('a',"Puntero nulo\n");
             break;
           }

           int size = machine -> ReadRegister(5);
	   DEBUG('u',"2\n");
           if (size <= 0){
             DEBUG('a',"El tamaño debe ser mayor a 0\n");
             break;
           }

           OpenFileId id = machine -> ReadRegister(6);
	   DEBUG('u',"3\n");
           if (id < 0){
             DEBUG('a',"file descriptor inválido\n");
             break;
           }

           char buffer[size];
	   DEBUG('u',"4\n");
           // Leer por consola
           if (id == CONSOLE_INPUT || id == CONSOLE_OUTPUT){
             DEBUG('a',"Leyendo por consola\n");
             synchConsole -> ReadConsole(size,buffer);
           }
           // Leer desde un archivo
           else{
	     DEBUG('u',"5\n");
             DEBUG('a',"Se va a leer desde %d\n",id);
             OpenFile* of = currentThread -> GetOpenFile(id);
	     DEBUG('b',"of:%p soy %p\n",of,currentThread);
             of -> Read(buffer,size);
           }
           //Escribir a espacio de usuario
           DEBUG('a',"2\n");
           WriteBufferToUser(buffer,userBuffer,size);
           DEBUG('a',"Leyó %s",buffer);
          break;
        }

          case SC_WRITE : {
            int userBuffer = machine -> ReadRegister(4);
            if (userBuffer == 0){
              DEBUG('y',"Buffer nulo");
              break;
            }

            int size = machine -> ReadRegister(5);
            if (size <= 0){
              DEBUG('y',"Tamaño inválido");
              break;
            }

            char buffer[size];
            // Leer desde espacio de usuario
	        ReadBufferFromUser(userBuffer,buffer,size);
            OpenFileId id = machine -> ReadRegister(6);

            //Escribir por consola
            if (id == CONSOLE_OUTPUT || id == CONSOLE_INPUT){
              DEBUG('a',"Escribiendo por consola\n");
              synchConsole -> WriteConsole(size,buffer);
            }
            //Escribir en un archivo
            else{
              DEBUG('a',"Se intenta escribir en %d\n",id);
              OpenFile* of = currentThread -> GetOpenFile(id);
              of -> Write(buffer,size);
            }
            break;
          }

          case SC_CLOSE: {
              int id = machine->ReadRegister(4);
              if (id < 0){
                DEBUG('a',"File descriptor inválido\n");
                break;
              }

              currentThread -> CloseOpenFile(id);
              DEBUG('a', "Se cierra id %u.\n", id);
              break;
          }

        case SC_EXIT: {
          int value = machine -> ReadRegister(4);
          //Finalizar thread enviando valor
          currentThread -> Finish(value);
          break;
        }

        case SC_EXEC: {
          DEBUG('a',"Por aca pasa\n");
          int userBuffer = machine -> ReadRegister(4);
          //Copiar argumentos a espacio de kernel
	  char** argv = SaveArgs(machine -> ReadRegister(5));
          if (userBuffer == 0){
            DEBUG('a',"Puntero nulo\n");
            break;
          }

          //Almacenar ruta
          char* path = new char [FILE_PATH_MAX_LEN+1];
          //Crear nuevo hilo
          char* name =  new char[60];
          name = "Pedrito";
          Thread* child = new Thread(name,true,currentThread -> GetPriority());
          //Copiar ruta a espacio de kernel
          ReadBufferFromUser(userBuffer,path,FILE_PATH_MAX_LEN);
          //Agregar a tabla de procesos
          SpaceId id = processTable -> Add(child);
          //Cargar el archivo que se va a ejecutar
          OpenFile *executable = fileSystem -> Open(path);
          if (executable == nullptr) {
              DEBUG('a',"No pudo abrir %s\n", path);
              break;
          }


          //Crear espacio de direcciones
          AddressSpace *space = new AddressSpace(executable,id);

          child -> space = space;
          //Forkear
          child -> Fork(Dummy,(void*)argv);
          //Devolver id de proceso hijo
          machine -> WriteRegister(2,id);
          break;
        }

        case SC_JOIN: {
          DEBUG('u',"2\n");
          SpaceId id = machine -> ReadRegister(4);
          if (id < 0){
            DEBUG('a',"Space id inválido\n");
            break;
          }

          //Chequear que el id existe en la tabla de procesos
          if(!processTable -> HasKey(id)){
            DEBUG('a',"No hay procesos con id %d\n",id);
            break;
          }
          Thread* t = processTable -> Get(id);
          int* i = new int;
          t -> Join(i); // Bloquer hasta que el proceso termine
          processTable -> Remove(id); // Quitar de tabla de procesos
          machine -> WriteRegister(2,*i);
          delete i;
          break;
        }

        default:
            fprintf(stderr, "Unexpected system call: id %d.\n", scid);
            ASSERT(false);

    }

    IncrementPC();
    
}


/// By default, only system calls have their own handler.  All other
/// exception types are assigned the default handler.
void
SetExceptionHandlers()
{
    machine->SetHandler(NO_EXCEPTION,            &DefaultHandler);
    machine->SetHandler(SYSCALL_EXCEPTION,       &SyscallHandler);
    machine->SetHandler(PAGE_FAULT_EXCEPTION,    &DefaultHandler);
    machine->SetHandler(READ_ONLY_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(BUS_ERROR_EXCEPTION,     &DefaultHandler);
    machine->SetHandler(ADDRESS_ERROR_EXCEPTION, &DefaultHandler);
    machine->SetHandler(OVERFLOW_EXCEPTION,      &DefaultHandler);
    machine->SetHandler(ILLEGAL_INSTR_EXCEPTION, &DefaultHandler);
}
