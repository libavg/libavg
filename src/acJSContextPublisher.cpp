//=============================================================================
// Copyright (C) 2003, ART+COM AG Berlin
//
// These coded instructions, statements, and computer programs contain
// unpublished proprietary information of ART+COM AG Berlin, and
// are copy protected by law. They may not be disclosed to third parties
// or copied or duplicated in any form, in whole or in part, without the
// specific, prior written permission of ART+COM AG Berlin.
//=============================================================================
//
//   $RCSfile$
//   $Author$
//   $Revision$
//   $Date$
//
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

/* long get (); */
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


