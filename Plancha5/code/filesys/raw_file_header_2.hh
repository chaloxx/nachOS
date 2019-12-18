/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_FILESYS_RAWFILEHEADER__HH
#define NACHOS_FILESYS_RAWFILEHEADER__HH


#include "machine/disk.hh"

static const unsigned NUM_DIRECT
  = (SECTOR_SIZE - 2 * sizeof (int)) / sizeof (int);
const unsigned MAX_FILE_SIZE = NUM_DIRECT * SECTOR_SIZE;
const unsigned RAW_SIZE = SECTOR_SIZE / sizeof(int);
/*Modificamos la estructura para poder implementar indirección*/
/*El file header sera quien guarde el tamaño del archivo y la cantidad de sectores ocupados*/
class RawFileHeader {
   // unsigned numBytes;  ///< Number of bytes in the file.
   // unsigned numSectors;  ///< Number of data sectors in the file.
	public:
          unsigned* dataSectors; 

	  RawFileHeader(unsigned);
	  
	  ~RawFileHeader();

	  void FetchFrom(unsigned);

	  void WriteBack(unsigned);


};





#endif
