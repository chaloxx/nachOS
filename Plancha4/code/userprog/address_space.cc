/// Routines to manage address spaces (executing user programs).
///
/// In order to run a user program, you must:
///
/// 1. Link with the `-N -T 0` option.
/// 2. Run `coff2noff` to convert the object file to Nachos format (Nachos
///    object code format is essentially just a simpler version of the UNIX
///    executable object code format).
/// 3. Load the NOFF file into the Nachos file system (if you have not
///    implemented the file system yet, you do not need to do this last
///    step).
///
/// Copyright (c) 1992-1993 The Regents of the University of California.
///               2016-2017 Docentes de la Universidad Nacional de Rosario.
/// All rights reserved.  See `copyright.h` for copyright notice and
/// limitation of liability and disclaimer of warranty provisions.


#include "address_space.hh"
#include "bin/noff.h"
#include "machine/endianness.hh"
#include "threads/system.hh"

//Calcula un coeficiente de acuerdo a la prioridad de reemplazo :
// use dirty    prioridad
// false false     0 (más alto)
// false true      1
// true  false     2
// true  true      3 (más bajo)

#ifdef VMEM
int
CalculatePriority(unsigned now){
  Thread* t = coremap[now].process;
  ASSERT(t != nullptr);
  unsigned vpn = coremap[now].vpn;
  TranslationEntry* page = t -> space -> GetPage(vpn);
  DEBUG('y',"Calculando coef para %u con use:%u dirty:%u\n",vpn,page->use,page->dirty);
  return (page -> dirty * 1) + (page -> use * 2);
}
//Rutina para apagar el bit de use de la page vpn del thread t
void GetOffUseBit(Thread* t,unsigned vpn){
    ASSERT(t != nullptr);
    TranslationEntry* page = t -> space -> GetPage(vpn);
    page -> use = false;
}

void
PrintCoreMap(){
 for(unsigned i = 0; i < NUM_PHYS_PAGES;i++){
      unsigned vpn = coremap[i].vpn;
      TranslationEntry* page = coremap[i].process -> space -> GetPage(vpn);
      printf("page:%u\n",i);
      printf("vpn:%u\n",vpn);
      printf("dirty:%u\n",page -> dirty);
      printf("use:%u\n",page -> use);
      printf("---------\n");
 }

}


 void
ImprovedClockAlgorithm(){
  DEBUG('c',"Se entra al algoritmo con %u\n",mmIndex);
  unsigned coef = 0;
 //Quedarse con el que tenga coeficiente 0 o con el menor posible
  unsigned now = mmIndex; 
  unsigned lower = 4;
  //  PrintCoreMap();
  for(unsigned i = 0;i < NUM_PHYS_PAGES;i++){
     coef = CalculatePriority(now);
     if (coef == 0){
          DEBUG('c',"Se sale del algoritmo con %u\n",mmIndex);
          mmIndex = now;
	  return;     
     }
     //Si se encuentra una página con menor coeficiente, actualizar
     if (coef < lower){
	     lower = coef;
	     mmIndex = now;          
     }
     if(coef == 2 || coef == 3)
        GetOffUseBit(coremap[mmIndex].process,coremap[mmIndex].vpn);

     now = (now+1)%NUM_PHYS_PAGES;
  } 
  DEBUG('c',"Se llego al final y se sale del algoritmo con %u\n",mmIndex);
}


void ClockAlgorithm(){
  DEBUG('c',"Se entra al algoritmo con %u\n",mmIndex);
  bool found = false;
  // Iterar hasta encontrar una página adecuada
  while(!found){
    // Chequear si el marco se usa por algún proceso  
       Thread* t = coremap[mmIndex].process;
       ASSERT(t != nullptr);
       unsigned vpn = coremap[mmIndex].vpn;
       TranslationEntry* page = t -> space -> GetPage(vpn);
       //Si fue usada recientemente le damos otra oportunidad
       if (page -> use){
         page -> use = false;
         mmIndex = (mmIndex + 1) % NUM_PHYS_PAGES;
       }
       else
         found = true;
  }             
  DEBUG('c',"Se sale con %u\n",mmIndex);
  return;
}


#endif


uint32_t AddressTranslation(uint32_t virtualAddr,TranslationEntry* pageTable){
   DEBUG('z',"direc virtual %u\n",virtualAddr);
   uint32_t offset = virtualAddr % PAGE_SIZE;
   unsigned virtualPage =  virtualAddr / PAGE_SIZE;
   DEBUG('z',"pagina %u y offset %u\n",virtualPage,offset);
   uint32_t physicalAddr = (pageTable[virtualPage].physicalPage*PAGE_SIZE) + offset;
   DEBUG('z',"direc real %u\n",physicalAddr);
   return physicalAddr;

}




/// Do little endian to big endian conversion on the bytes in the object file
/// header, in case the file was generated on a little endian machine, and we
/// are re now running on a big endian machine.
static void
SwapHeader(noffHeader *noffH)
{
    ASSERT(noffH != nullptr);

    noffH->noffMagic              = WordToHost(noffH->noffMagic);
    noffH->code.size              = WordToHost(noffH->code.size);
    noffH->code.virtualAddr       = WordToHost(noffH->code.virtualAddr);
    noffH->code.inFileAddr        = WordToHost(noffH->code.inFileAddr);
    noffH->initData.size          = WordToHost(noffH->initData.size);
    noffH->initData.virtualAddr   = WordToHost(noffH->initData.virtualAddr);
    noffH->initData.inFileAddr    = WordToHost(noffH->initData.inFileAddr);
    noffH->uninitData.size        = WordToHost(noffH->uninitData.size);
    noffH->uninitData.virtualAddr =
      WordToHost(noffH->uninitData.virtualAddr);
    noffH->uninitData.inFileAddr  = WordToHost(noffH->uninitData.inFileAddr);
}

/// Create an address space to run a user program.
///
/// Load the program from a file `executable`, and set everything up so that
/// we can start executing user instructions.
///
/// Assumes that the object code file is in NOFF format.
///
/// First, set up the translation from program memory to physical memory.
/// For now, this is really simple (1:1), since we are only uniprogramming,
/// and we have a single unsegmented page table.
///
/// * `executable` is the file containing the object code to load into
///   memory.
//






AddressSpace::AddressSpace(OpenFile *executable, unsigned id)
{
    ASSERT(executable != nullptr);
    exe = executable;
    exe -> ReadAt((char *) &noffH, sizeof noffH, 0);
    if (noffH.noffMagic != NOFF_MAGIC &&
          WordToHost(noffH.noffMagic) == NOFF_MAGIC)
        SwapHeader(&noffH);
    ASSERT(noffH.noffMagic == NOFF_MAGIC);
    DEBUG('u',"Primer direc virtual de codigo %u\n",noffH.code.virtualAddr);
   DEBUG('u',"Ultima direc virtual de codigo %u\n",noffH.code.virtualAddr+noffH.code.size);
    DEBUG('u',"Primer direc virtual de datos %u\n",noffH.initData.virtualAddr);


    // How big is address space?

    unsigned size = noffH.code.size + noffH.initData.size
                    + noffH.uninitData.size + USER_STACK_SIZE;
      // We need to increase the size to leave room for the stack.
    numPages = DivRoundUp(size, PAGE_SIZE);
    size = numPages * PAGE_SIZE;

    DEBUG('y', "Initializing address space of %s, num pages %u, size %u\n", currentThread -> GetName(),
          numPages, size);

    DEBUG('y', "Tamaño total de cod y datos %u\n",
                noffH.code.size + noffH.initData.size);


    pageTable = new TranslationEntry[numPages];
    for (unsigned i = 0; i < numPages; i++) {
        pageTable[i].virtualPage  = i;
        pageTable[i].physicalPage = NUM_PHYS_PAGES;
        pageTable[i].valid        = false;
        pageTable[i].use          = false;
        pageTable[i].dirty        = false;
        pageTable[i].readOnly     = false;
          // If the code segment was entirely on a separate page, we could
          // set its pages to be read-only.
    }

    ShadowPageTable = new Bitmap(numPages);
    VpInMemory = new Bitmap(numPages);

    char* s = "swap/asid%u";
    char* swapName = new char[strlen(s)];
    sprintf(swapName,s,id);
    fileSystem -> Create(swapName,size);
    DEBUG('y',"Se creo %s\n",swapName);
    swap = fileSystem -> Open(swapName);
    ASSERT(swap != nullptr);
    tlb = machine -> GetMMU() -> tlb;
    delete [] swapName;
}

/// Deallocate an address space.
///
/// Nothing for now!
AddressSpace::~AddressSpace()
{
    delete [] pageTable;
    delete exe;
    delete swap;
    delete ShadowPageTable;
    delete VpInMemory;
}

/// Set the initial values for the user-level register set.
///
/// We write these directly into the “machine” registers, so that we can
/// immediately jump to user code.  Note that these will be saved/restored
/// into the `currentThread->userRegisters` when this thread is context
/// switched out.
void
AddressSpace::InitRegisters()
{
    for (unsigned i = 0; i < NUM_TOTAL_REGS; i++)
        machine->WriteRegister(i, 0);

    // Initial program counter -- must be location of `Start`.
    machine->WriteRegister(PC_REG, 0);

    // Need to also tell MIPS where next instruction is, because of branch
    // delay possibility.
    machine->WriteRegister(NEXT_PC_REG, 4);

    // Set the stack register to the end of the address space, where we
    // allocated the stack; but subtract off a bit, to make sure we do not
    // accidentally reference off the end!
    machine->WriteRegister(STACK_REG, numPages * PAGE_SIZE - 16);
    DEBUG('a', "Initializing stack register to %u\n",
          numPages * PAGE_SIZE - 16);
}

/// On a context switch, save any machine state, specific to this address
/// space, that needs saving.
///
/// For now, nothing!
/*void
AddressSpace::SaveState()
{}*/

/// On a context switch, restore the machine state so that this address space
/// can run.
///
/// For now, tell the machine where to find the page table.
/*void
AddressSpace::RestoreState()
{
  DEBUG('u',"Restaurando tlb soy %p\n",currentThread);
  // Invalidar todas las traducciones
  for(unsigned i = 0; i < TLB_SIZE;i++){
      tlb[i].valid = false;
    }
  }
*/
unsigned
AddressSpace::GetNumPages(){
  return numPages;
}

void
AddressSpace::UpdPageTable(){
    ASSERT(tlb != nullptr);
    for(unsigned i = 0; i < TLB_SIZE;i++){
    if(tlb[i].valid){
      unsigned vpn = tlb[i].virtualPage;
      DEBUG('y',"Actualizando %u\n",vpn);
      ASSERT(vpn < numPages);
      pageTable[vpn].use = tlb[i].use;
      pageTable[vpn].dirty = tlb[i].dirty;
      DEBUG('y',"Use: %d\n",pageTable[vpn].use);
      DEBUG('y',"Dirty: %d\n",pageTable[vpn].dirty);
    }
  }
  DEBUG('u',"Soy %p y ya actualize mi page table\n", currentThread);
}

void 
AddressSpace::NullTLB(){
	ASSERT(tlb != nullptr);
	for(unsigned i = 0;i < TLB_SIZE;i++)
		tlb[i].valid = false;

}




TranslationEntry*
AddressSpace::GetPage(int vpn){
  ASSERT(vpn < numPages);
  DEBUG('y',"Se solicita page %u y se tienen %u pages\n",vpn,numPages);
  return  &pageTable[vpn];
}

noffHeader
AddressSpace::GetNoffH(){
  return noffH;
}

OpenFile*
AddressSpace::GetExecutable(){
  return exe;
}

bool
AddressSpace::CopyToSwap(unsigned vpn){
  //ASSERT(pageTable[vpn].dirty == true);
  unsigned vAddr = vpn * PAGE_SIZE;
  unsigned physAddr = pageTable[vpn].physicalPage * PAGE_SIZE;
  char* mainMemory = machine -> GetMMU() -> mainMemory;
  int numBytes = swap -> WriteAt(&mainMemory[physAddr],PAGE_SIZE,vAddr);
  DEBUG('y',"Se copio %d bytes desde %u fisica a %u swap\n",numBytes,physAddr,vAddr);
  ShadowPageTable -> Mark(vpn);
  VpInMemory -> Clear(vpn);
  //Si descomento esto se rompe
  //pageTable[vpn].dirty = false;
  return numBytes == PAGE_SIZE;
}

bool
AddressSpace::CopyFromSwap(unsigned vpn){
  //La página debe estar limpia
  //ASSERT(pageTable[vpn].dirty == false);  
  unsigned vAddr = vpn * PAGE_SIZE;
  unsigned physAddr = pageTable[vpn].physicalPage * PAGE_SIZE;
  char* mainMemory = machine -> GetMMU() -> mainMemory;
  memset(&mainMemory[physAddr],0,PAGE_SIZE);
  int numBytes = swap -> ReadAt(&mainMemory[physAddr],PAGE_SIZE,vAddr);
  DEBUG('y',"Se escribio %d bytes desde %u swap a %u fisica\n",numBytes,vAddr,physAddr);
  VpInMemory -> Mark(vpn);  
  ShadowPageTable -> Clear(vpn);
  return numBytes >= 0;
}


void   
AddressSpace::SaveState(){
  DEBUG('u',"Soy %p y actualizo mi page table\n", currentThread);
  //Actualizar page table antes del cambio de contexto
  UpdPageTable();
  // Anular TLB
  NullTLB();
}


void
AddressSpace::FindVPN(unsigned vpn){ 
#ifdef VMEM
  if(numPages <= vpn){
   DEBUG('y',"vpn inválida %u solo tengo %u\n",vpn,numPages);
   //currentThread -> Finish(-1);
   ASSERT(false);
  }
  TranslationEntry* page = GetPage(vpn);
   // Chequear si la página esta en memoria
   if (VpInMemory -> Test(vpn)){ 
     unsigned physPage = page ->physicalPage;
     //Comprobar que la page está en el marco físico que dice estar
     ASSERT(coremap[physPage].process == currentThread && coremap[physPage].vpn == vpn);
     DEBUG('y',"vpn %u está en memoria\n",vpn);
     // La página está siendo copiada a swap y no puede usarse
     if (coremap[physPage].inSwap)
	     return;
     AddToTLB(page);
     return;
   }


  // Si no hay más ram hay que desalojar alguna página
  int p = -1;
  if(paginationTable -> CountClear() == 0){
     DEBUG('y',"No hay mas memoria\n");
     p = FindPageToClean();
   }  
   page -> physicalPage = paginationTable -> Find();
   //Corroborar que se eligió la página desalojada
   if (p != -1)
	   ASSERT(p == page -> physicalPage);

   // Asignar esta pagina al correspondiente thread
   coremap[page -> physicalPage].process = currentThread;
   coremap[page -> physicalPage].vpn = page -> virtualPage;
   //Ya descartamos que este en memoria, entonces hay que copiarla desde swap o desde el disco

   //Chequear si la página esta en swap
   if(ShadowPageTable -> Test(vpn)){
     DEBUG('y',"vpn %u está en swap\n",vpn);
     if(!CopyFromSwap(vpn)){
       DEBUG('y',"Error al escribir de swap a memoria la page %u",vpn);
       ASSERT(false);
     }
    }
    else{//sino está en el disco
       DEBUG('y',"vpn %u está en disco\n",vpn);
      if(!CopyFromDisk(vpn,page)){
        DEBUG('y',"Error al traer la página desde el disco\n");
        ASSERT(false);
      }
    }
    DEBUG('y',"A la vpn %u se le asigna %u \n",vpn,page -> physicalPage);
    //Agregar a TLB
    AddToTLB(page);
#endif     
}

bool
AddressSpace::CopyFromDisk(unsigned vpn,TranslationEntry* page){
  //Desde donde hay que copiar
  unsigned vAddr = vpn*PAGE_SIZE;
  //Adonde hay que copiar
  unsigned physAddr = page -> physicalPage*PAGE_SIZE;
  char *mainMemory = machine->GetMMU()->mainMemory;
  // Setear memoria
  memset(&mainMemory[physAddr],0,PAGE_SIZE);
  // Copiar lo que corresponda del segmento de código o del segmento de datos
  unsigned numBytes = 0;
  unsigned pos = 0;
  unsigned offset = 0;

  unsigned codeEnd = noffH.code.virtualAddr + noffH.code.size;
  unsigned pageEnd = vAddr + PAGE_SIZE;
  unsigned dataEnd = noffH.initData.virtualAddr + noffH.initData.size;
  DEBUG('u',"La virtual address es %u\n",vAddr);
  DEBUG('u',"codigo comienza en  %u\n",noffH.code.virtualAddr);
  DEBUG('u',"codigo termina en  %u\n",codeEnd);
  DEBUG('u',"datos comienza en%u\n",noffH.initData.virtualAddr);
  DEBUG('u',"datos termina en%u\n",noffH.initData.virtualAddr+noffH.initData.size);
  unsigned ctos = 0;
  if (noffH.code.virtualAddr <= vAddr && vAddr < codeEnd){
      offset = vAddr-noffH.code.virtualAddr;
      pos = noffH.code.inFileAddr+offset;
      if (pageEnd <= codeEnd)
       ctos = PAGE_SIZE;      
      else
        ctos = codeEnd-vAddr;    
      DEBUG('u',"En codigoSe van a leer %u bytes\n",ctos); 
      numBytes += exe -> ReadAt(&mainMemory[physAddr],ctos,pos);
      vAddr += ctos;
      physAddr += ctos;
      DEBUG('u',"En codigo se leyeron %u bytes\n",numBytes); 
  }

  if (noffH.initData.virtualAddr <= vAddr && vAddr < dataEnd && numBytes < PAGE_SIZE) {
      offset = vAddr-noffH.initData.virtualAddr;
      pos = noffH.initData.inFileAddr+offset;
      ctos = PAGE_SIZE - numBytes;
      DEBUG('u',"En datos se van a leer %u bytes\n",ctos);
      numBytes += exe -> ReadAt(&(mainMemory[physAddr]),ctos, pos);
      DEBUG('u',"En datos se leyeron %u bytes\n",numBytes);
  }
  VpInMemory -> Mark(vpn);
  return numBytes >= 0;
  }

void
AddressSpace::AddToTLB(TranslationEntry* page){
#ifdef VMEM
  // Agregar a tlb
  DEBUG('y',"Agregando %u a la tlb se encuentra en %u, se reemplaza %u en la tlb\n", page -> virtualPage, page -> physicalPage,tlbIndex);
  TranslationEntry oldPage = tlb[tlbIndex];
  // Si la traducción en la página que se va a reemplazar en la TLB era válida hay que copiar
  // los bits que puede haber modificado la TLB.
  // Dado que la traducción es válida solo para el actual thread, la página actualizada
  // está en la pageTable del actual thread
  if (oldPage.valid){
      pageTable[oldPage.virtualPage].dirty = oldPage.dirty;
      pageTable[oldPage.virtualPage].use = oldPage.use;
  }
  ASSERT(page -> virtualPage < numPages);
  tlb[tlbIndex].virtualPage = page -> virtualPage;
  tlb[tlbIndex].physicalPage = page -> physicalPage;
  tlb[tlbIndex].valid = true;
  tlb[tlbIndex].use = page -> use;
  tlb[tlbIndex].dirty = page -> dirty;
  tlb[tlbIndex].readOnly = page -> readOnly;
  // Elegir la próxima página a descartar en la TLB de forma lineal
  tlbIndex = (tlbIndex+1) % TLB_SIZE;
#endif
}



unsigned
AddressSpace::FindPageToClean(){
#ifdef VMEM	
  //Actualizar page table
  UpdPageTable();
  // Usar algoritmo del reloj para determinar que pagina reemplazar
  ClockAlgorithm();
  //ImprovedClockAlgorithm();
  DEBUG('y',"Se va a reemplazar %u \n",mmIndex);
  Thread* proc  = coremap[mmIndex].process;
  //Dado que la ram está llena esto debería ser null
  ASSERT(proc != nullptr);
  unsigned vpn = coremap[mmIndex].vpn;
  //Sino está en el disco está solicitando una página en ram para escribir
  AddressSpace* space = proc -> space;
  TranslationEntry* page = space -> GetPage(vpn);
  // Copiar a swap si está sucia
  if(page -> dirty){
      //Mientras la bandera de swap está encendida nadie puede acceder a este página      
      coremap[mmIndex].inSwap = true;
      DEBUG('h',"%d esta sucia, copiando en swap\n",vpn);
      ASSERT(space -> CopyToSwap(vpn));
      //La página ya no está sucia     
      coremap[mmIndex].inSwap = false;
  }
  // La pagina virtual ya no se encuentra en memoria
  space -> VpInMemory -> Clear(vpn);
  DEBUG('u',"%u ya no está en memoria\n",vpn);
  if (proc == currentThread) 
	  UpdVPFromTLB(vpn);
  coremap[mmIndex].process = nullptr;  
  // Desmarcar página en tabla de paginacion
  paginationTable -> Clear(mmIndex);
  int res = mmIndex;
  //mmIndex = (mmIndex+1) % NUM_PHYS_PAGES;
  return res;
 #endif
}



void
AddressSpace::UpdVPFromTLB(unsigned vpn){
  DEBUG('y',"Se invalida %u en la tlb\n",vpn);
  TranslationEntry* tlb = machine -> GetMMU() -> tlb;
  // Invalidar traducción en tlb
  for(unsigned i = 0;i < TLB_SIZE;i++){ 
     // Actualiza la información pertinente
    if (tlb[i].virtualPage == vpn  && tlb[i].valid){
          TranslationEntry* page = GetPage(vpn);
          //page -> dirty = tlb[i].dirty;
          page -> use = tlb[i].use;
          tlb[i].valid = false;
    }
  }
}






