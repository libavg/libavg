//=============================================================================
//
// Original code Copyright (C) 2003, ART+COM AG Berlin
//
// Released under LGPL.
//
//=============================================================================
//
//   $RCSfile$
//   $Author$
//   $Revision$
//   $Date$
//
//=============================================================================


#include "acJSContextPublisher.h"

#include <nsIGenericFactory.h>
#include <nsMemory.h>

JSContext * acJSContextPublisher::_myContext_(0);


NS_IMPL_ISUPPORTS1_CI(acJSContextPublisher, acIJSContextPublisher);

acJSContextPublisher::acJSContextPublisher()
{
}

acJSContextPublisher::~acJSContextPublisher()
{
}

NS_IMETHODIMP acJSContextPublisher::GetContext(PRInt32 *_retval)
{
    *_retval = (PRInt32)_myContext_;
    return NS_OK;
}

NS_IMETHODIMP
acJSContextPublisher::SetContext(PRInt32 theContext) {
    _myContext_ = (JSContext *)theContext;
    return NS_OK;
}


