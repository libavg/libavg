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
#include "acJSContextPublisher.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(AVGPlayer)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGEvent)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGImage)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGVideo)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGWords)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGAVGNode)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGExcl)
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
NS_DECL_CLASSINFO(acJSContextPublisher)
  
static const nsModuleComponentInfo components[] =
{
  { "c-base avgplayer Component", 
    AVGPLAYER_CID, 
    AVGPLAYER_CONTRACTID, 
    AVGPlayerConstructor
  },
  { "c-base avgevent Component", 
    AVGEVENT_CID, 
    AVGEVENT_CONTRACTID, 
    AVGEventConstructor
  },
  { "c-base avgimage Component", 
    AVGIMAGE_CID, 
    AVGIMAGE_CONTRACTID, 
    AVGImageConstructor
  },
  { "c-base avgvideo Component", 
    AVGVIDEO_CID, 
    AVGVIDEO_CONTRACTID, 
    AVGVideoConstructor
  },
  { "c-base avgwords Component", 
    AVGWORDS_CID, 
    AVGWORDS_CONTRACTID, 
    AVGWordsConstructor
  },
  { "c-base avgavgnode Component", 
    AVGAVGNODE_CID, 
    AVGAVGNODE_CONTRACTID, 
    AVGAVGNodeConstructor
  },
  { "c-base avgexcl Component", 
    AVGEXCL_CID, 
    AVGEXCL_CONTRACTID, 
    AVGExclConstructor,
    AVGPlayerRegistrationProc
  },
  { "acJSContextPublisher Component", 
    ACJSCONTEXTPUBLISHER_CID, 
    ACJSCONTEXTPUBLISHER_CONTRACTID, 
    acJSContextPublisherConstructor
  }

};

NS_IMPL_NSGETMODULE(AVGPlayerModule, components)

//==============================================================================
//
// $Log$
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
