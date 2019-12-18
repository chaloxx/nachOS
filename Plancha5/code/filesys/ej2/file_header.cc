/// Routines for managing the disk file header (in UNIX, this would be called
/// the i-node).
///
/// The file header is used to locate where on disk the file's data is
/// stored.  We implement this as a fixed size table of pointers -- each
/// entry in the table points to the disk sector containing that portion of
/// the file data (in other words, there are no indirect or doubly indirect
/// blocks). The table size is chosen so that the file header will be just
/// big enough to fit in one disk sector,
///
/// Unlike in a real system, we do not keep track of file permissions,
/// ownership, last modification date, etc., in the file header.
///
/// A file header can be initialized in two ways:
///
/// * for a new file, by modifying the in-memory data structure to point to
///   the newly allocated data blocks;
/// * for a file already on disk, by reading the file header from disk.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "file_header.hh"

/// Initialize a fresh file header for a newly created file.  Allocate data
/// blocks for the file out of the map of free disk blocks.  Return false if
/// there are not enough free blocks to accomodate the new file.
///
/// * `freeMap` is the bit map of free disk sectors.
/// * `fileSize` is the bit map of free disk sectors.

bool
FileHeader::Allocate(Bitmap *freeMap, unsigned newBytes)
{
    ASSERT(freeMap != nullptr);
    ASSERT(newBytes < MAX_FILE_SIZE);
    numBytes = newBytes; // Tamaño del archivo
    numDataSectors = DivRoundUp(newBytes, SECTOR_SIZE);//Sectores para datos
    DEBUG('f',"Se necesitan %u sectores para datos\n",numDataSectors);
    unsigned numMetaSectors = DivRoundUp(numDataSectors,META_DATA_SIZE); //Sectores para metadatos
    DEBUG('f',"Se necesitan %u sectores para metadatos\n",numMetaSectors);
    if (freeMap->CountClear() < numDataSectors+numMetaSectors)
        return false;  // Not enough space.


    ASSERT(numMetaSectors <= NUM_DIRECT);//No podemos tener más sectores para metadatos que lo que nos sobra en el file header
    DEBUG('f',"Tamaño de metaDataSectors %u \n",NUM_DIRECT*sizeof(unsigned));
    unsigned iters = 0;
    for (unsigned i = 0; i < numMetaSectors; i++){
           metaDataSectors[i] = freeMap->Find();	   
	   if(i < numMetaSectors-1){
		   iters = META_DATA_SIZE;
	   }
	   else{
	      iters = numDataSectors % META_DATA_SIZE;
	   }
	   DEBUG('f',"Se van a hacer %u iteraciones\n",iters);
	   unsigned* dataSectors = new unsigned[META_DATA_SIZE];
	   for(unsigned k = 0;k < iters;k++){
		  dataSectors[k] = freeMap -> Find();//Reservar cada sector para datos
		  DEBUG('f',"Se reserva el sector %u para datos\n",dataSectors[k]);
	   }

	   synchDisk->WriteSector(metaDataSectors[i],(char*)dataSectors);
	   delete [] dataSectors;//Esta memoria no se usa más
       }
     return true;
}

/// De-allocate all the space allocated for data blocks for this file.
///
/// * `freeMap` is the bit map of free disk sectors.
void
FileHeader::Deallocate(Bitmap *freeMap)
{
    ASSERT(freeMap != nullptr);
    unsigned numMetaDataSectors = DivRoundUp(numDataSectors,META_DATA_SIZE);
    unsigned* dataSectors, iters = 0;
    for (unsigned i = 0; i < numMetaDataSectors; i++) {
	dataSectors = new unsigned[META_DATA_SIZE];
	synchDisk -> ReadSector(metaDataSectors[i],(char*)dataSectors);
        if (i < numMetaDataSectors-1){
	  iters = META_DATA_SIZE;
	}
	else{
	  iters = numDataSectors % META_DATA_SIZE;
	}
	for(unsigned k = 0; k < iters;k++){
	   ASSERT(freeMap -> Test(dataSectors[k]));
	   freeMap ->Clear(dataSectors[k]);
	}
		
        ASSERT(freeMap->Test(metaDataSectors[i]));  // ought to be marked!
        freeMap->Clear(metaDataSectors[i]);
    }
}

/// Fetch contents of file header from disk.
///
/// * `sector` is the disk sector containing the file header.
void
FileHeader::FetchFrom(unsigned sector)
{
    synchDisk->ReadSector(sector, (char *) this);
}

/// Write the modified contents of the file header back to disk.
///
/// * `sector` is the disk sector to contain the file header.
void
FileHeader::WriteBack(unsigned sector)
{
    synchDisk->WriteSector(sector, (char *) this);
}

/// Return which disk sector is storing a particular byte within the file.
/// This is essentially a translation from a virtual address (the offset in
/// the file) to a physical address (the sector where the data at the offset
/// is stored).
///
/// * `offset` is the location within the file of the byte in question.
unsigned
FileHeader::ByteToSector(unsigned offset)
{
    DEBUG('u',"offset: %u\n",offset);
    unsigned dataSector = DivRoundDown(offset,SECTOR_SIZE);// Calcular sector de datos
    unsigned metaDataSector = DivRoundDown(dataSector,META_DATA_SIZE); //Calcular sector de metadatos
    unsigned* dataSectors = new unsigned[META_DATA_SIZE];
    synchDisk->ReadSector(metaDataSectors[metaDataSector],(char*)dataSectors);//Leer sector de datos
    unsigned realSector = dataSectors[dataSector%META_DATA_SIZE];//Obtener sector
    delete [] dataSectors;//Esta memoria no se usa más
    DEBUG('u',"real sector: %u\n",realSector);
    return realSector;
}

/// Return the number of bytes in the file.
unsigned
FileHeader::FileLength() const
{
    return numBytes;
}

/// Print the contents of the file header, and the contents of all the data
/// blocks pointed to by the file header.
void
FileHeader::Print()
{
    //char *data = new char [SECTOR_SIZE];

    printf("FileHeader contents.\n"
           "    Size: %u bytes\n"
           "    Block numbers: ",
           numBytes);
    /*for (unsigned i = 0; i < raw.numSectors; i++)
        printf("%u ", raw.dataSectors[i]);
    printf("\n    Contents:\n");
    for (unsigned i = 0, k = 0; i < raw.numSectors; i++) {
        synchDisk->ReadSector(raw.dataSectors[i], data);
        for (unsigned j = 0; j < SECTOR_SIZE && k < raw.numBytes; j++, k++) {
            if ('\040' <= data[j] && data[j] <= '\176')  // isprint(data[j])
                printf("%c", data[j]);
            else
                printf("\\%X", (unsigned char) data[j]);
        }
        printf("\n");
    }
    delete [] data;*/
}

RawFileHeader*
FileHeader::GetRaw() const
{
    RawFileHeader* raw = new RawFileHeader;
    raw -> numBytes = numBytes;
    raw -> numSectors = numDataSectors;
    //raw -> metaDataSectors = (unsigned*)metaDataSectors; 
    return raw;
}
