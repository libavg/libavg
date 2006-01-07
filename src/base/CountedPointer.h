//
// $Id$
//

#ifndef _CountedPointer_H_
#define _CountedPointer_H_

#include <stdlib.h>

// This is a simple non-threadsafe smart pointer.
// To be replaced by the boost smart pointer again once the unstable
// boost ver 1.31 isn't in general use anymore.

template <class type>
class CountedPointer
{
public:
    explicit CountedPointer(type * pType = 0)
              : pBody(pType), pCount(new size_t(1))
    {
    }
    
    CountedPointer(const CountedPointer & copy)
      : pBody(copy.pBody), pCount(copy.pCount)      
    { 
        incCount(); 
    }
    
    ~CountedPointer()
    { 
        decCount(); 
    }

    
    CountedPointer& operator=(const CountedPointer & rhs);

    type * operator -> () const 
    {
        return pBody; 
    }
    
    type & operator * () const                             
    { 
        return *pBody; 
    }

    type * get() const                                     
    { 
        return pBody; 
    }

    bool operator == (const CountedPointer & rhs) const  
    { 
        return pBody == rhs.pBody; 
    }

    bool operator != (const CountedPointer & rhs) const 
    {
        return pBody != rhs.pBody;
    }

    operator bool () const                                    
    { 
        return pBody != 0; 
    }

private:
    void incCount()    
    { 
        ++*pCount; 
    }

    void decCount();

private:
    type * pBody;
    size_t * pCount;
};

template <class type>
CountedPointer<type>& CountedPointer<type>::operator=
    (const CountedPointer & rhs)
{
    if (pBody != rhs.pBody) {
        decCount();
        pBody = rhs.pBody;
        pCount = rhs.pCount;
        incCount();
    }
    return *this;
}

template <class type>
void CountedPointer<type>::decCount()
{
  if (!--*pCount) {
    delete pBody;
    delete pCount;
  }
}

template <class type>
class CountedArrayPointer
{
public:
    explicit CountedArrayPointer(type * pType = 0)
        : pBody(pType), pCount(new size_t(1))
    {
    }
    
    CountedArrayPointer(const CountedArrayPointer & copy)
        : pBody(copy.pBody), pCount(copy.pCount)
    { 
        incCount(); 
    }
    
    ~CountedArrayPointer()                        
    { 
        decCount(); 
    }

    CountedArrayPointer& operator=(const CountedArrayPointer & rhs);

    type * operator -> () const
    {
        return pBody; 
    }
    
    type & operator * () const
    { 
        return *pBody; 
    }

    type * get() const
    { 
        return pBody; 
    }

    bool operator == (const CountedArrayPointer & rhs) const 
    { 
        return pBody == rhs.pBody; 
    }
    
    bool operator != (const CountedArrayPointer & rhs) const 
    { 
        return pBody != rhs.pBody; 
    }

    type & operator [] (size_t i)
    { 
        return pBody[i]; 
    }
    
    const type & operator [] (size_t i) const
    {
        return pBody[i]; 
    }

    operator bool () const
    { 
        return pBody != 0; 
    }

private:
    void      incCount()
    { 
        ++*pCount; 
    }
    
    void      decCount();

private:
    type *      pBody;
    size_t *    pCount;
};

template <class type>
CountedArrayPointer<type>& CountedArrayPointer<type>::operator=
    (const CountedArrayPointer & rhs)
{
  if (pBody != rhs.pBody) {
    decCount();
    pBody = rhs.pBody;
    pCount = rhs.pCount;
    incCount();
  }
  return *this;
}

template <class type>
void CountedArrayPointer<type>::decCount()
{
  if (!--*pCount) {
    delete [] pBody;
    delete pCount;
  }
}

#endif
