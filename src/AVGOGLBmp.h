//
// $Id$
//

#ifndef _AVGOGLBmp_H_
#define _AVGOGLBmp_H_

#include <paintlib/planybmp.h>

class AVGOGLBmp : public PLAnyBmp
{

public:
    AVGOGLBmp();
    virtual ~AVGOGLBmp();
    AVGOGLBmp(const PLBmp &Orig);
    AVGOGLBmp(const AVGOGLBmp &Orig);
    AVGOGLBmp &operator=(const PLBmp &Orig);
    AVGOGLBmp &operator=(const AVGOGLBmp &Orig);

    void bind();
    void unbind();
    int getTexID();

protected:
    virtual void freeMembers();
    
    unsigned int m_TexID;
    bool m_bBound;
};

inline AVGOGLBmp::AVGOGLBmp(const AVGOGLBmp &Orig)
    : PLAnyBmp ()
{
  internalCopy (Orig);
}

inline AVGOGLBmp::AVGOGLBmp(const PLBmp &Orig)
    : PLAnyBmp ()
{
  internalCopy (Orig);
}

inline AVGOGLBmp & AVGOGLBmp::operator=( const PLBmp &Orig)
{
  PLBmp::operator=(Orig);
  return *this;
}

inline AVGOGLBmp & AVGOGLBmp::operator=( const AVGOGLBmp &Orig)
{
  PLBmp::operator=(Orig);
  return *this;
}
#endif

