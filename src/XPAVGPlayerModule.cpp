//
// $Id$
//

#include <nsIGenericFactory.h>

#include "AVGPlayer.h"
#include "AVGEvent.h"
#include "AVGImage.h"
#include "AVGVideo.h"
#include "AVGWords.h"
#include "AVGAVGNode.h"
#include "AVGExcl.h"
#include "AVGConradRelais.h"
#include "acJSContextPublisher.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(AVGPlayer)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGEvent)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGImage)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGVideo)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGWords)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGAVGNode)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGExcl)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGConradRelais)
NS_GENERIC_FACTORY_CONSTRUCTOR(acJSContextPublisher)

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
NS_DECL_CLASSINFO(AVGImage)
NS_DECL_CLASSINFO(AVGVideo)
NS_DECL_CLASSINFO(AVGWords)
NS_DECL_CLASSINFO(AVGAVGNode)
NS_DECL_CLASSINFO(AVGExcl)
NS_DECL_CLASSINFO(AVGConradRelais)
NS_DECL_CLASSINFO(acJSContextPublisher)
  
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
  },
  { "c-base avgimage Component", 
    AVGIMAGE_CID, 
    AVGIMAGE_CONTRACTID, 
    AVGImageConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGImage),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGImage)
  },
  { "c-base avgvideo Component", 
    AVGVIDEO_CID, 
    AVGVIDEO_CONTRACTID, 
    AVGVideoConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGVideo),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGVideo)
  },
  { "c-base avgwords Component", 
    AVGWORDS_CID, 
    AVGWORDS_CONTRACTID, 
    AVGWordsConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGWords),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGWords)
  },
  { "c-base avgavgnode Component", 
    AVGAVGNODE_CID, 
    AVGAVGNODE_CONTRACTID, 
    AVGAVGNodeConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGAVGNode),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGAVGNode)
  },
  { "c-base avgexcl Component", 
    AVGEXCL_CID, 
    AVGEXCL_CONTRACTID, 
    AVGExclConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGExcl),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGExcl)
  },
  { "c-base avgconradrelais Component", 
    AVGCONRADRELAIS_CID, 
    AVGCONRADRELAIS_CONTRACTID, 
    AVGConradRelaisConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGConradRelais),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGConradRelais)
  },
  { "acJSContextPublisher Component", 
    ACJSCONTEXTPUBLISHER_CID, 
    ACJSCONTEXTPUBLISHER_CONTRACTID, 
    acJSContextPublisherConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(acJSContextPublisher),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(acJSContextPublisher)
  }

};

NS_IMPL_NSGETMODULE(AVGPlayerModule, components)

//==============================================================================
//
// $Log$
// Revision 1.10  2004/02/13 15:51:33  uzadow
// Added untested Conrad relais card support.
//
// Revision 1.9  2003/10/24 11:52:33  uzadow
// Added auto width/height sensing for videos and text, added interface flattening.
//
// Revision 1.8  2003/09/19 17:08:57  uzadow
// Massive memory leak fixes.
//
// Revision 1.7  2003/08/28 09:49:29  uzadow
// Fixed bugs in video support. Now opacity doesnt work :-(.
//
// Revision 1.6  2003/08/20 22:16:21  uzadow
// Added words node incl. alpha channel support and clipping
//
// Revision 1.5  2003/07/27 14:45:45  uzadow
// lotsastuff
//
// Revision 1.4  2003/03/22 20:35:12  uzadow
// Added excl node support.
//
// Revision 1.3  2003/03/22 17:10:27  uzadow
// Fixed AVGAVGNode refcount handling.
//
// Revision 1.2  2003/03/21 15:13:01  uzadow
// Event object now accessible from Javascript.
//
// Revision 1.1  2003/03/18 17:03:46  uzadow
// Forgotten files
//
//
//==============================================================================
