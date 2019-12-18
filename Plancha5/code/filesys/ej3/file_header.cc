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



unsigned RestaNaturales(unsigned a, unsigned b){
    if (a > b)
	return a-b;
    return a;
}



bool
FileHeader::Allocate(Bitmap *freeMap, unsigned newBytes)
{
    ASSERT(freeMap != nullptr);
    DEBUG('f',"Tengo q allocar %u bytes\n",newBytes);
    DEBUG('f',"Actualmente tengo %u bytes\n",numBytes);
    if(numBytes+newBytes > MAX_FILE_SIZE){
      DEBUG('f',"El tamaño del archivo es muy grande\n");
      return false;
    }
    unsigned freeBytes = 0;
    unsigned dataOffset = numBytes % SECTOR_SIZE;
    if (dataOffset)
	    freeBytes = SECTOR_SIZE - dataOffset; // Bytes libres en el último sector ocupado
     
    DEBUG('f',"FreeBytes = %u\n",freeBytes);
    if(newBytes <= freeBytes){
	  numBytes += newBytes; 
	  return true;// Nada más para hacer
    }  
    
    //Restar y sumar lo que ya tenemos reservado  
    newBytes -= freeBytes;
    numBytes += freeBytes; 
    DEBUG('f',"newBytes = %u\n",newBytes);

    unsigned faltaDataSec = DivRoundUp(newBytes,SECTOR_SIZE);// Cantidad de sectores para datos que se necesita
    unsigned tengoDataSec = DivRoundUp(numBytes,SECTOR_SIZE);// Cantidad de sectores para datos que ya habíamos reservado
    unsigned tengoMetaSec = DivRoundUp(tengoDataSec,META_DATA_SIZE);// Cantidad de sectores para metadatos que ya hemos reservado
    unsigned metaOffset =  tengoDataSec % META_DATA_SIZE;// Bytes ocupados en el último sector de metadatos
    unsigned faltaMetaSec =  DivRoundUp(tengoDataSec+faltaDataSec,META_DATA_SIZE)-tengoMetaSec;//Cantidada de sectores para metadatos que faltan reservar

    DEBUG('f',"Faltan %u sectores para datos\n",faltaDataSec);
    DEBUG('f',"Faltan %u sectores para metadatos\n",faltaMetaSec);
    if (freeMap->CountClear() < faltaDataSec+faltaMetaSec){ 
	DEBUG('f',"No hay suficiente espacio\n");
        return false;  
    }
    if (faltaMetaSec > NUM_DIRECT-tengoMetaSec){
        DEBUG('f',"No hay suficiente espacio en la cabecera de archivo para los metadatos\n");
	return false;
    }

   //Completar último sector de metadatos si tiene espacio para más sectores de datos
    CompleteLastMetaDataSector(metaOffset,tengoMetaSec-1,&faltaDataSec,freeMap);
      //Reservar los sectores de datos que faltan y los sectores a metadatos
    for (unsigned i = tengoMetaSec; i < tengoMetaSec+faltaMetaSec; i++){
           metaDataSectors[i] = freeMap->Find();	   	   
	   DEBUG('f',"Se reserva el sector %u para metadatos\n",metaDataSectors[i]);
	   unsigned dataSectors[META_DATA_SIZE] ;//Aca se guardan los sectores a datos	  
	   for(unsigned k = 0;k < META_DATA_SIZE && faltaDataSec > 0;k++){
		  dataSectors[k] = freeMap -> Find();//Reservar cada sector para datos
		  faltaDataSec--; //Restamos cada vez que reservamos un sector para datos
		  DEBUG('f',"Se reserva el sector %u para datossss\n",dataSectors[k]);
	   }
           //Guardar cambios
	   synchDisk->WriteSector(metaDataSectors[i],(char*)dataSectors);
       }
     numBytes += newBytes;
     return true;
}


void FileHeader::CompleteLastMetaDataSector(unsigned offset,unsigned last,unsigned* faltaDataSec,Bitmap* freeMap){
    DEBUG('f',"El offset es %u \n",offset);
    DEBUG('f',"Last:%u\n",last);
    if (offset){//El último sector de metadatos tiene espacio para guardar más sectores de datos
        unsigned dataSectors[META_DATA_SIZE];
        synchDisk->ReadSector(metaDataSectors[last],(char*)dataSectors);
        for(unsigned i = offset;*faltaDataSec > 0 && i < META_DATA_SIZE;i++){
               dataSectors[i] = freeMap -> Find();
               DEBUG('f',"Se reserva el sector %u para datosp\n",dataSectors[i]);
	       --*faltaDataSec;
	       DEBUG('f',"Falta:%u\n",*faltaDataSec);
	}
	synchDisk->WriteSector(metaDataSectors[last],(char*)dataSectors);
    }
}

/// De-allocate all the space allocated for data blocks for this file.
///
/// * `freeMap` is the bit map of free disk sectors.
void
FileHeader::Deallocate(Bitmap *freeMap)
{
    ASSERT(freeMap != nullptr);
    unsigned tengoDataSec = DivRoundUp(numBytes,SECTOR_SIZE);
    unsigned tengoMetaSec = DivRoundUp(tengoDataSec,META_DATA_SIZE);
    unsigned dataSectors[META_DATA_SIZE], iters = 0;
    for (unsigned i = 0; i < tengoMetaSec; i++) {
	synchDisk -> ReadSector(metaDataSectors[i],(char*)dataSectors);
        if (i < tengoMetaSec-1){
	  iters = META_DATA_SIZE;
	}
	else{
	  iters = tengoDataSec % META_DATA_SIZE;
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
    DEBUG('f',"offset: %u\n",offset);
    unsigned dataSec = DivRoundDown(offset,SECTOR_SIZE);// Calcular  sector de datos
    DEBUG('f',"sector: %u\n",dataSec);
    unsigned metaDataSec = DivRoundDown(dataSec,META_DATA_SIZE); //Calcular en que sector de metadatos está 
    DEBUG('f',"metadata: %u\n",metaDataSec);

    unsigned dataSectors[META_DATA_SIZE];
    synchDisk->ReadSector(metaDataSectors[metaDataSec],(char*)dataSectors);//Leer sector de datos
    DEBUG('f',"El sector buscado es: %u\n",dataSectors[dataSec%META_DATA_SIZE]);
    unsigned realSector = dataSectors[dataSec%META_DATA_SIZE];//Obtener sector
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
    raw -> numSectors = GetNumSectors();
    //raw -> metaDataSectors = (unsigned*)metaDataSectors; 
    return raw;
}


unsigned
FileHeader::GetNumSectors() const{
	return 0;
}

unsigned FileHeader::MissingBytes(unsigned position, unsigned bytes){
    unsigned ctos = (position + bytes) - numBytes;
    if (ctos > 0)
	    return ctos;
    return 0;
}
