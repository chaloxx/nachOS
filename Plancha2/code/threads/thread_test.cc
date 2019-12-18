/// Simple test case for the threads assignment.
///
/// Create several threads, and have them context switch back and forth
/// between themselves by calling `Thread::Yield`, to illustrate the inner
/// workings of the thread system.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2007-2009 Universidad de Las Palmas de Gran Canaria.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "system.hh"
#include "synch.hh"
#include "port.hh"
#include <stdio.h>
#include <unistd.h>
#include "Test/ej1Test.cc"
#include "Test/ej2Test.cc"
#include "Test/ej3Test.cc"
#include "Test/ej4Test.cc"

void ThreadTest(){
  ej1Test();// Correr con -d c
  ///ej2Test(); // Correr con -d p
  //ej3Test(); // Correr con -d j
  //ej4ATest(); // Correr con -d g
  //ej4BTest();// Correr con -d s
}
