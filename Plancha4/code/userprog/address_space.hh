/// Data structures to keep track of executing user programs (address
/// spaces).
///
/// For now, we do not keep any information about address spaces.  The user
/// level CPU state is saved and restored in the thread executing the user
/// program (see `thread.hh`).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.

#ifndef NACHOS_USERPROG_ADDRESSSPACE__HH
#define NACHOS_USERPROG_ADDRESSSPACE__HH


#include "filesys/file_system.hh"
#include "../bin/noff.h"
#include "lib/bitmap.hh"
class TranslationEntry;
class Entry;



const unsigned USER_STACK_SIZE = 1024;  ///< Increase this as necessary!


class AddressSpace {
public:

    /// Create an address space, initializing it with the program stored in
    /// the file `executable`.
    ///
    /// * `executable` is the open file that corresponds to the program.
    AddressSpace(OpenFile *executable, unsigned id);

    /// De-allocate an address space.
    ~AddressSpace();

    /// Initialize user-level CPU registers, before jumping to user code.
    void InitRegisters();

    /// Save/restore address space-specific info on a context switch.

    void SaveState();
    void RestoreState();
    unsigned GetNumPages();
    TranslationEntry* GetPage(int);
    noffHeader GetNoffH();

    OpenFile* GetExecutable();

    bool CopyToSwap(unsigned);

    bool CopyFromSwap(unsigned);

    void RestoreTLB();

    void FindVPN(unsigned);

private:

    bool CopyFromDisk(unsigned,TranslationEntry*);
    void AddToTLB(TranslationEntry*);
    unsigned FindPageToClean();
    void UpdVPFromTLB(unsigned);
    TranslationEntry* tlb;
    void UpdPageTable();
    void NullTLB();

    /// Assume linear page table translation for now!
    TranslationEntry *pageTable;

    /// Number of pages in the virtual address space.
    unsigned numPages;

    noffHeader noffH;

    OpenFile* exe;
    OpenFile* swap;
    Bitmap* ShadowPageTable;// Páginas en swap
    Bitmap* VpInMemory;//Páginas en memoriaunsigned vAddr = vpn*PAGE_SIZE;

    int userRegisters[3];

};


#endif
