//
// $Id$
//

#include <nsIGenericFactory.h>

#include <avgconfig.h>

#include "AVGPlayer.h"
#include "AVGEvent.h"
#include "AVGKeyEvent.h"
#include "AVGMouseEvent.h"
#include "AVGWindowEvent.h"
#include "AVGImage.h"
#include "AVGPanoImage.h"
#include "AVGVideoBase.h"
#include "AVGVideo.h"
#ifdef AVG_ENABLE_1394
#include "AVGCamera.h"
#endif
#include "AVGWords.h"
#include "AVGAVGNode.h"
#include "AVGDivNode.h"
#include "AVGExcl.h"
#include "AVGConradRelais.h"
#include "AVGJSPoint.h"
#include "acJSContextPublisher.h"

NS_GENERIC_FACTORY_CONSTRUCTOR(AVGPlayer)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGEvent)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGMouseEvent)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGKeyEvent)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGWindowEvent)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGImage)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGPanoImage)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGVideo)
#ifdef AVG_ENABLE_1394
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGCamera)
#endif
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGWords)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGAVGNode)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGDivNode)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGExcl)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGConradRelais)
NS_GENERIC_FACTORY_CONSTRUCTOR(AVGJSPoint)
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
NS_DECL_CLASSINFO(AVGMouseEvent)
NS_DECL_CLASSINFO(AVGKeyEvent)
NS_DECL_CLASSINFO(AVGWindowEvent)
NS_DECL_CLASSINFO(AVGPanoImage)
NS_DECL_CLASSINFO(AVGImage)
NS_DECL_CLASSINFO(AVGVideo)
#ifdef AVG_ENABLE_1394
NS_DECL_CLASSINFO(AVGCamera)
#endif
NS_DECL_CLASSINFO(AVGWords)
NS_DECL_CLASSINFO(AVGAVGNode)
NS_DECL_CLASSINFO(AVGDivNode)
NS_DECL_CLASSINFO(AVGExcl)
NS_DECL_CLASSINFO(AVGConradRelais)
NS_DECL_CLASSINFO(AVGJSPoint)
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
  { "c-base avgmouseevent Component", 
    AVGMOUSEEVENT_CID, 
    AVGMOUSEEVENT_CONTRACTID, 
    AVGMouseEventConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGMouseEvent),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGMouseEvent)
  },
  { "c-base avgkeyevent Component", 
    AVGKEYEVENT_CID, 
    AVGKEYEVENT_CONTRACTID, 
    AVGKeyEventConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGKeyEvent),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGKeyEvent)
  },
  { "c-base avgevent Component", 
    AVGWINDOWEVENT_CID, 
    AVGWINDOWEVENT_CONTRACTID, 
    AVGWindowEventConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGWindowEvent),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGWindowEvent)
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
  { "c-base avgpanoimage Component", 
    AVGPANOIMAGE_CID, 
    AVGPANOIMAGE_CONTRACTID, 
    AVGPanoImageConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGPanoImage),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGPanoImage)
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
#ifdef AVG_ENABLE_1394
  { "c-base avgcamera Component", 
    AVGCAMERA_CID, 
    AVGCAMERA_CONTRACTID, 
    AVGCameraConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGCamera),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGCamera)
  },
#endif
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
  { "c-base avgdivnode Component", 
    AVGDIVNODE_CID, 
    AVGDIVNODE_CONTRACTID, 
    AVGDivNodeConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGDivNode),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGDivNode)
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
  { "c-base avgjspoint Component", 
    AVGJSPOINT_CID, 
    AVGJSPOINT_CONTRACTID, 
    AVGJSPointConstructor,
    AVGPlayerRegistrationProc,
    AVGPlayerUnregistrationProc,
    NULL /* no factory destructor */,
    NS_CI_INTERFACE_GETTER_NAME(AVGJSPoint),
    NULL /* no language helper */,
    &NS_CLASSINFO_NAME(AVGJSPoint)
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
// Revision 1.18  2004/08/12 08:51:46  uzadow
// Added image warp support
//
// Revision 1.17  2004/07/30 15:48:29  uzadow
// Initial incomplete version of panorama renderer.
//
// Revision 1.16  2004/07/05 16:01:33  uzadow
// Separated div and avg nodes, added keyboard event handling.
//
// Revision 1.15  2004/06/01 10:36:37  artcom
// Added OGL 15 and 24 bpp modes, updates to camera code.
//
// Revision 1.14  2004/05/11 12:03:21  uzadow
// Added configure options for DFB, OGL and camera support.
//
// Revision 1.13  2004/05/05 11:20:25  uzadow
// Initial untested camera node.
//
// Revision 1.12  2004/04/30 10:43:50  uzadow
// Split AVGVideo into AVGVideo and AVGVideoBase to prepare for AVGCamera.
//
// Revision 1.11  2004/02/16 23:12:19  uzadow
// Major refactoring of event handling.
//
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
