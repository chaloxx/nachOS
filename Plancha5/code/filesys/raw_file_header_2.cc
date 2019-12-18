#include "raw_file_header_2.hh"
#include "synch_disk.hh"
extern SynchDisk* synchDisk;


RawFileHeader::RawFileHeader(unsigned size){
 dataSectors = new unsigned[size];
}


RawFileHeader::~RawFileHeader(){
 delete [] dataSectors;
}

void
RawFileHeader::FetchFrom(unsigned sector){
 synchDisk -> ReadSector(sector,(char*)this);
}

void
RawFileHeader::WriteBack(unsigned sector){
 synchDisk -> WriteSector(sector,(char*)this);
}


