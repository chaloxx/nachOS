/// Routines to manage a directory of file names.
///
/// The directory is a table of fixed length entries; each entry represents a
/// single file, and contains the file name, and the location of the file
/// header on disk.  The fixed size of each directory entry means that we
/// have the restriction of a fixed maximum size for file names.
///
/// The constructor initializes an empty directory of a certain size; we use
/// ReadFrom/WriteBack to fetch the contents of the directory from disk, and
/// to write back any modifications back to disk.
///
/// Also, this implementation has the restriction that the size of the
/// directory cannot expand.  In other words, once all the entries in the
/// directory are used, no more files can be created.  Fixing this is one of
/// the parts to the assignment.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2018 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "directory.hh"
#include "directory_entry.hh"
#include "file_header.hh"
#include "lib/utility.hh"
#include "file_system.hh"
#include "threads/system.hh"



extern FileSystem* fileSystem;


//Mover ruta al siguiente '/'
const char* UpdatePath(const char* path){
    for(;path[0] != '/';path++);
    ++path;
    DEBUG('f',"Se actualizo la ruta %s\n",path);
    return path;
}

//Sector donde se ubica el padre
void Directory::SetDirectoryFather(unsigned fd){
    raw.dirFather = fd;
}



DirectoryEntry CreateDirectoryEntry(const char* path){
   ASSERT(path != nullptr);
   DirectoryEntry entry;
   DEBUG('x',"Creando dirEntry para %s\n",path);
   unsigned i = 0; 
   for(; path[i] != '/' && path[i] != '\0';i++);
   entry.inUse = true;
   strncpy(entry.name,path,i);
   entry.name[i] ='\0';
   if (path[i] == '/')
     entry.isDirectory = true;
   else 
     entry.isDirectory = false;
   return entry;   
}



/// Initialize a directory; initially, the directory is completely empty.  If
/// the disk is being formatted, an empty directory is all we need, but
/// otherwise, we need to call FetchFrom in order to initialize it from disk.
///
/// * `size` is the number of entries in the directory.
// df es el sector donde se almacena el directorio padre
Directory::Directory(unsigned size,unsigned where,int df)
{


    ASSERT(size > 0);
    Iam =  where;
    raw.tableSize = size;
    raw.dirFather = df;
    DEBUG('u',"El tamaño del dir es %u\n",size*sizeof(DirectoryEntry));
    for (unsigned i = 0; i < raw.tableSize; i++)
        raw.table[i].inUse = false;
}

/// De-allocate directory data structure.
Directory::~Directory()
{
}

/// Read the contents of the directory from disk.
///
/// * `file` is file containing the directory contents.
void
Directory::FetchFrom(OpenFile *file)
{
    ASSERT(file != nullptr);
    file->ReadAt((char *) &raw,DIRECTORY_FILE_SIZE, 0);
}

/// Write any modifications to the directory back to disk.
///
/// * `file` is a file to contain the new directory contents.
void
Directory::WriteBack(OpenFile *file)
{
    ASSERT(file != nullptr);
    file->WriteAt((char *) &raw,DIRECTORY_FILE_SIZE, 0);
}

/// Look up file name in directory, and return its location in the table of
/// directory entries.  Return -1 if the name is not in the directory.
///
/// * `name` is the file name to look up.
int
Directory::FindIndex(const char *name)
{
    ASSERT(name != nullptr);
    //for()

    for (unsigned i = 0; i < raw.tableSize; i++)
            if (raw.table[i].inUse
              && !strncmp(raw.table[i].name, name, FILE_NAME_MAX_LEN))
                  return i;           
    
       return -1;  // name not in directory
}



        

// Agregar un archivo al directorio, si ya se encuentra se retorna false sino true
// Si no existen los subdirectorios en path, entonces se crean
bool
Directory::Add(const char *name, int newSector,Bitmap* freeMap)
{
    ASSERT(name != nullptr);
    DEBUG('r',"path: %s sec:%u\n",name,Iam);
           
    DirectoryEntry entry = CreateDirectoryEntry(name);
    int res = FindIndex(entry.name); 
    //Si es un archivo y ya existe error
    if (!entry.isDirectory && res != -1)
        return false;

    if (entry.isDirectory){
            //Caso especial: me pasan "..", se agrega en el padre
	    Directory* dir;
	    OpenFile* dirFile;
	    if (!strcmp(entry.name,"..")){
		    //Si estoy en el root, no puedo subir más
		    if (raw.dirFather == -1)
			    return false;
		    dir = new Directory(10,raw.dirFather,0);
		    dirFile = new OpenFile(raw.dirFather);
		    dir -> FetchFrom(dirFile);
		    name = UpdatePath(name);
		    //Agregar al padre
		    bool success = dir -> Add(name,newSector,freeMap);
		    delete dirFile;
		    delete dir;
		    return success;

	    }
               //Si es un directorio y no existe hay que crearlo
      	    if (res == -1){
                //Reservar sector para el directorio
                entry.sector = freeMap -> Find();
                //Crear directorio		 
                dirFile = fileSystem -> CreateDirectory(entry.sector,freeMap,Iam);	   
	        DEBUG('r',"El padre de %s es %d\n",entry.name,Iam);
            }
	    else{
                entry.sector = raw.table[res].sector;
	        dirFile = new OpenFile(entry.sector);
	    }
            dir = new Directory(10,entry.sector,Iam);
	    dir -> FetchFrom(dirFile);
	    //Actualizar ruta
	    name = UpdatePath(name);	 	    
	    //Agregar al subdirectorio	    	    
	    DEBUG('f',"nm:%s\n",name);           
	    if(!dir -> Add(name,newSector,freeMap))
			    return false;
            delete dirFile;
            delete dir;	    

	    //Si el subdirectorio ya estaba guardado, no hay nada más para hacer
	    if (res != -1)
		    return true;
    }
    else
	entry.sector = newSector;

    for (unsigned i = 0; i < raw.tableSize; i++)
        if (!raw.table[i].inUse) {
	    DEBUG('r',"entry.name:%s\n",entry.name);
	    raw.table[i] = entry;	    
	    //Cada directorio guarda sus propios cambios	    
	    OpenFile* myDirFile = new OpenFile(Iam);
	    WriteBack(myDirFile);
	    delete myDirFile;
            return true;
        }
    
    return false;  // no space.  Fix when we have extensible files.
}

/// Remove a file name from the directory.   Return true if successful;
/// return false if the file is not in the directory.
///
/// * `name` is the file name to be removed.
// Modificado para poder borrar archivos o subdirectorios
bool
Directory::Remove(const char *name)
{
    ASSERT(name != nullptr);
    DirectoryEntry entry = CreateDirectoryEntry(name);
    int i = FindIndex(entry.name);
    DEBUG('r',"rm: %s\n",name);
    DEBUG('r',"en: %s\n",entry.name);
    
    if (i == -1 && strcmp(entry.name,".."))
        return false;  // name not in directory
    if (entry.isDirectory){	    
	    int sector;
	    //Borrar en el padre
	    if (!strcmp(entry.name,"..")){
		if (raw.dirFather == -1)
                      return false;							
	        sector = raw.dirFather;
		DEBUG('r',"1sec:%u\n",sector);
	    }
	    else{
		sector = raw.table[i].sector;
	    }
	    Directory* dir = new Directory(10,sector,Iam);
	    OpenFile* dirFile = new OpenFile(dir -> Iam);
	    dir -> FetchFrom(dirFile);
	    name = UpdatePath(name);	    
	    bool res = dir -> Remove(name);
	    delete dir;
	    delete dirFile;    
	    return res;
    }
    ASSERT(raw.table[i].inUse == true);
    raw.table[i].inUse = false;              	      
    OpenFile* myDirFile = new OpenFile(Iam);
    //Guardad cambios
    WriteBack(myDirFile);
    return true;
}

//Borrar los contenidos de un directorio usando el fs
void
Directory::RemoveContents(const char* path){
    ASSERT(path != nullptr);
    DEBUG('r',"Borrando %s\n",path);
    for(unsigned k = 0; k < raw.tableSize;k++)
      	   if (raw.table[k].inUse){			   
         	   char* newPath = AbsolutePath(path,raw.table[k].name);
		   DEBUG('r',"Se borra %s\n",newPath);
                   fileSystem -> Remove(newPath);
     		   delete [] newPath;
	   }
}



/// List all the file names in the directory.
void
Directory::List() const
{
    for (unsigned i = 0; i < raw.tableSize; i++)
        if (raw.table[i].inUse){
	    if (raw.table[i].isDirectory)		    
                 printf("/%s\n", raw.table[i].name);
	    else 		    
                 printf("%s\n", raw.table[i].name);
	}

}

/// List all the file names in the directory, their `FileHeader` locations,
/// and the contents of each file.  For debugging.
void
Directory::Print(const char* name,const char* sep) const
{
    FileHeader *hdr = new FileHeader;
    printf("%sPrinting directory %s contents:\n",sep,name);
    for (unsigned i = 0; i < raw.tableSize; i++)
        if (raw.table[i].inUse) {	     
            printf("%s    Name: %s\n"
                   "%s   Sector: %u\n",
                   sep,raw.table[i].name,sep, raw.table[i].sector);
	    if (raw.table[i].isDirectory){
	      char* newSep = Concatenar(sep,"   ");
	      Directory* dir = new Directory(10,raw.table[i].sector,Iam);
	      OpenFile* dirFile = new OpenFile(dir -> Iam);
	      dir -> FetchFrom(dirFile);
	      dir -> Print(raw.table[i].name,newSep);
	      delete [] newSep;
	      delete dir;
	      delete dirFile;
	    }
        }
    printf("\n%sEnd\n",sep);
    delete hdr;
}

const RawDirectory *
Directory::GetRaw() const
{
    return &raw;
}

//Encontrar el sector de un directorio anidado o padre
// -1 identifica al padre del directorio root (no existe)
// Usamos un bool para determinar si estamos buscando un archivo
// o un directorio
int
Directory::Find(const char* path,bool isDir){
      ASSERT(path != nullptr);
      DirectoryEntry entry = CreateDirectoryEntry(path);
      if (entry.isDirectory){
	      int sector;
              if (! strcmp(entry.name,"..")){ // Buscar en el padre
	             
                      //Si estamos en el direc root no podemos subir
		      if (raw.dirFather == -1) 
     			      return -1;
		      //El sector es del padre de este directorio     
	              sector = raw.dirFather;
	       }      
               else{//Buscar en los directorios anidados
		      int index = FindIndex(entry.name);
		      //Si no es un directorio, error
		      if (!raw.table[index].isDirectory)
			      return -1;
		      sector = raw.table[index].sector;	       
	       }	      
               Directory* dir = new Directory(10,sector,Iam);
	       OpenFile* dirFile = new OpenFile(sector);
	       dir -> FetchFrom(dirFile);
	       path = UpdatePath(path); 
               int res = dir -> Find(path,isDir);
	       delete dir;
	       delete dirFile;
	       return res;	            
      }

      else if (!strcmp(entry.name,"..")){
	      return raw.dirFather;     
              }

      else{
      int index = FindIndex(entry.name);
      //Chequear que el elemento sea lo que buscamos (directorio o archivo)
      if (index != -1 && isDir == raw.table[index].isDirectory)
	      return raw.table[index].sector;                 
      return -1;
      }
  }
