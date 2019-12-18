#include "utility.hh"	

#ifndef _TABLET_
#define _TABLET_

template <class T>
class TableT{
 public:
   
   TableT(unsigned);

   bool Add(unsigned,T*);

   T* Get(unsigned i);

   bool HasVal(unsigned i);

   T* Remove(unsigned);

   bool IsEmpty();
 private:
   unsigned size;
   T** data;

};


template <class T>
TableT<T>::TableT(unsigned s)
{
    size = s;
    data = (T**) malloc(sizeof(T*)*size);
    for(int i = 0; i < size;i++)
	    data[i] = nullptr;     
    
}



/*template <class T>
TableT<T>::~TableT(){
   delete [] data;
}*/

template <class T>
bool 
TableT<T>::Add(unsigned i,T* item)
{
	if (data[i] == nullptr){
	    data[i] = item;
	    return true;
	}
	return false;
}




template <class T>
T*
TableT<T>::Get(unsigned i)
{
     ASSERT(i >= 0 && i <= size);
     return data[i];
}

template <class T>
bool
TableT<T>::HasVal(unsigned i) 
{
    ASSERT(i >= 0 && i<= size);
    return data[i] != nullptr;
}

template <class T>
bool
TableT<T>::IsEmpty() 
{
    for(int i = 0;i < size;i++){
	    if (data[i] != nullptr){
	       return false;
	    }
    }
    return true;
}

template <class T>
T*
TableT<T>::Remove(unsigned i)
{
    ASSERT(i >= 0 && i <= size);
    T* v = data[i];
    data[i] = nullptr;
    return v;  
}

#endif
