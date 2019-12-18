/// Data structures to manage a UNIX-like directory of file names.
///
/// A directory is a table of pairs: *<file name, sector #>*, giving the name
/// of each file in the directory, and where to find its file header (the
/// data structure describing where to find the file's data blocks) on disk.
///
/// We assume mutual exclusion is provided by the caller.
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_FILESYS_DIRECTORY__HH
#define NACHOS_FILESYS_DIRECTORY__HH


#include "raw_directory.hh"
#include "open_file.hh"
#include "directory_entry.hh"
const unsigned NUM_DIR_ENTRIES = 10;
const unsigned DIRECTORY_FILE_SIZE = sizeof (DirectoryEntry)* NUM_DIR_ENTRIES + 2*sizeof(int);


/// The following class defines a UNIX-like “directory”.  Each entry in the
/// directory describes a file, and where to find it on disk.
///
/// The directory data structure can be stored in memory, or on disk.  When
/// it is on disk, it is stored as a regular Nachos file.
///
/// The constructor initializes a directory structure in memory; the
/// `FetchFrom`/`WriteBack` operations shuffle the directory information
/// from/to disk.
class Bitmap;
class Directory {
public:

    /// Initialize an empty directory with space for `size` files.
    //Agregue en que sector se ubica y el sector de quien lo creo
    Directory(unsigned size,unsigned where,int df);

    /// De-allocate the directory.
    ~Directory();

    /// Initialize directory contents from disk.
    void FetchFrom(OpenFile *file);

    /// Write modifications to directory contents back to disk.
    void WriteBack(OpenFile *file);

    /// Encontrar el sector de la cabecera de  `name`
    // -1 identifica que no se encontro
    // isDir determina si estamos buscando un directorio o un archivo
    int Find(const char *name,bool isDir);

    /// Add a file name into the directory.
    bool Add(const char *name,int newSector,Bitmap* freeMap);

    /// Remove a file from the directory.
    bool Remove(const char *name);

    /// Print the names of all the files in the directory.
    void List() const;

    /// Verbose print of the contents of the directory -- all the file names
    /// and their contents.
    void Print(const char* name,const char* sep) const;

    /// Get the raw directory structure.
    ///
    /// NOTE: this should only be used by routines that operating on the file
    /// system at a low level.
    const RawDirectory *GetRaw() const;

    int FindDirectory(const char*);

    //Debería usarse solo por quien crea el directorio
    void SetDirectoryFather(unsigned);

    //Usa el fs para remover su contenido
    void RemoveContents(const char* path);

private:
    RawDirectory raw;

    /// Find the index into the directory table corresponding to `name`.
    int FindIndex(const char *name);

    //Sector en que se ubica este directorio
    unsigned Iam;
};


#endif
