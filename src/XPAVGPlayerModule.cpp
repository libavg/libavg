//
// $Id$
//

#include <nsIGenericFactory.h>

#include "AVGPlayer.h"
#include "AVGEvent.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(AVGPlayer)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGEvent)

static NS_METHOD AVGPlayerRegistrationProc(nsIComponentManager *aCompMgr,
                                           nsIFile *aPath,
                                           const char *registryLocation,
                                           const char *componentType,
                                           const nsModuleComponentInfo *info)
{
    return NS_OK;
}

static NS_METHOD AVGPlayerUnregistrationProc(nsIComponentManager *aCompMgr,
                                             nsIFile *aPath,
                                             const char *registryLocation,
                                             const nsModuleComponentInfo *info)
{
    return NS_OK;
}

NS_DECL_CLASSINFO(AVGPlayer)
NS_DECL_CLASSINFO(AVGEvent)

static const nsModuleComponentInfo components[] =
{
  { "c-base avgplayer Component", 
    AVGPLAYER_CID, 
    AVGPLAYER_CONTRACTID, 
    AVGPlayerConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGPlayer),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGPlayer)
  },
  { "c-base avgevent Component", 
    AVGEVENT_CID, 
    AVGEVENT_CONTRACTID, 
    AVGEventConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGEvent),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGEvent)
  }
};

NS_IMPL_NSGETMODULE(AVGPlayerModule, components)

//==============================================================================
//
// $Log$
// Revision 1.2  2003/03/21 15:13:01  uzadow
// Event object now accessible from Javascript.
//
// Revision 1.1  2003/03/18 17:03:46  uzadow
// Forgotten files
//
// Revision 1.1  2003/02/21 15:06:45  uzadow
// Printer errors are handled correctly.
//
//
//==============================================================================
