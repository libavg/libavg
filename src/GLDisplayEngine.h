//
// $Id$
//

#ifndef _GLDisplayEngine_H_
#define _GLDisplayEngine_H_

#include "IDisplayEngine.h"

class GLDisplayEngine : 	
	public IDisplayEngine
{
	private:
		GLDisplayEngine ();
		GLDisplayEngine (const GLDisplayEngine &);
		GLDisplayEngine & operator = (const GLDisplayEngine &);
	
	public:
		virtual ~GLDisplayEngine ();
};

#endif //_GLDisplayEngine_H_

