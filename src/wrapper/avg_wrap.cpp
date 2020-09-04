//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2020 Ulrich von Zadow
//
//  This library is free software; you can redistribute it and/or
//  modify it under the terms of the GNU Lesser General Public
//  License as published by the Free Software Foundation; either
//  version 2 of the License, or (at your option) any later version.
//
//  This library is distributed in the hope that it will be useful,
//  but WITHOUT ANY WARRANTY; without even the implied warranty of
//  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
//  Lesser General Public License for more details.
//
//  You should have received a copy of the GNU Lesser General Public
//  License along with this library; if not, write to the Free Software
//  Foundation, Inc., 59 Temple Place, Suite 330, Boston, MA  02111-1307  USA
//
//  Current versions can be found at www.libavg.de
//

void export_base();
void export_misc();
void export_node();
void export_event();
void export_anim();

#include "WrapHelper.h"
#include "raw_constructor.hpp"

#include "../base/OSHelper.h"
#include "../graphics/ImageCache.h"
#include "../player/Player.h"
#include "../player/AVGNode.h"
#include "../player/CameraNode.h"
#include "../player/DivNode.h"
#include "../player/TouchEvent.h"
#include "../player/MouseEvent.h"
#include "../player/Canvas.h"
#include "../player/Contact.h"
#include "../player/OffscreenCanvas.h"
#include "../player/VersionInfo.h"
#include "../player/ExportedObject.h"
#include "../player/TestHelper.h"
#include "../anim/Anim.h"

#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/raw_function.hpp>

using namespace boost::python;
using namespace avg;
using namespace std;

namespace bp = boost::python;

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(Player_createNode_overloads,
        createNode, 2, 3)

OffscreenCanvasPtr createCanvas(const boost::python::tuple &args,
                const boost::python::dict& params)
{
    checkEmptyArgs(args, 1);
    return extract<Player&>(args[0])().createCanvas(params);
}

CanvasPtr createMainCanvas(const boost::python::tuple &args,
                const boost::python::dict& params)
{
    checkEmptyArgs(args, 1);
    return extract<Player&>(args[0])().createMainCanvas(params);
}

boost::function<size_t (const bp::tuple& args, const bp::dict& kwargs )>
        playerGetMemoryUsage = boost::bind(getMemoryUsage);

BOOST_PYTHON_MODULE(avg)
{
    try {
        docstring_options doc_options(true, false);

        Player::get();
        PyEval_InitThreads();
        export_base();
        export_misc();

        register_ptr_to_python<DivNodePtr>();
        register_ptr_to_python<CanvasNodePtr>();
        register_ptr_to_python<NodePtr>();
        register_ptr_to_python<BitmapPtr>();
        register_ptr_to_python<AnimPtr>();
        register_ptr_to_python<ContactPtr>();
        register_ptr_to_python<CanvasPtr>();
        register_ptr_to_python<CursorEventPtr>();
        register_ptr_to_python<OffscreenCanvasPtr>();
        register_ptr_to_python<AVGNodePtr>();
        register_ptr_to_python<EventPtr>();
        register_ptr_to_python<MouseEventPtr>();
        register_ptr_to_python<TouchEventPtr>();

        class_<ExportedObject, boost::shared_ptr<ExportedObject>, boost::noncopyable>
                ("ExportedObject", no_init)
            .def(self == self)
            .def(self != self)
            .def("__hash__", &ExportedObject::getHash)
        ;

        class_<Publisher, bases<ExportedObject>, boost::noncopyable>("Publisher")
            .def("subscribe", &Publisher::subscribe)
            .def("unsubscribe", &Publisher::unsubscribeCallable)
            .def("unsubscribe", &Publisher::unsubscribe)
            .def("unsubscribe", &Publisher::unsubscribe1)
            .def("isSubscribed", &Publisher::isSubscribedCallable)
            .def("isSubscribed", &Publisher::isSubscribed)
            .def("getNumSubscribers", &Publisher::getNumSubscribers)
            .def("publish", &Publisher::publish)
            .def("notifySubscribers", &Publisher::notifySubscribersPy)
            .def("genMessageID", &Publisher::genMessageID)
            .staticmethod("genMessageID")
        ;

        export_event();
        export_node();
        export_anim();

        enum_<GLConfig::ShaderUsage>("ShaderUsage")
            .value("SHADERUSAGE_FULL", GLConfig::FULL)
            .value("SHADERUSAGE_MINIMAL", GLConfig::MINIMAL)
            .value("SHADERUSAGE_AUTO", GLConfig::AUTO)
            .export_values()
        ;

        object playerClass = class_<Player, bases<Publisher>, boost::noncopyable>
                ("Player") 
            .def("get", &Player::get, 
                    return_value_policy<reference_existing_object>())
            .staticmethod("get")
            .def("setResolution", &Player::setResolution)
            .def("isFullscreen", &Player::isFullscreen)
            .def("setWindowFrame", &Player::setWindowFrame)
            .def("setWindowPos", &Player::setWindowPos)
            .def("setWindowTitle", &Player::setWindowTitle)
            .def("setWindowConfig", &Player::setWindowConfig)
            .def("useGLES", &Player::useGLES)
            .def("setOGLOptions", &Player::setOGLOptions)
            .def("setMultiSampleSamples", &Player::setMultiSampleSamples)
            .def("enableGLErrorChecks", &Player::enableGLErrorChecks)
            .def("getScreenResolution", &Player::getScreenResolution)
            .def("getPixelsPerMM", &Player::getPixelsPerMM)
            .def("getPhysicalScreenDimensions", &Player::getPhysicalScreenDimensions)
            .def("assumePixelsPerMM", &Player::assumePixelsPerMM)
            .def("loadFile", &Player::loadFile)
            .def("loadString", &Player::loadString)
            .def("loadCanvasFile", &Player::loadCanvasFile)
            .def("loadCanvasString", &Player::loadCanvasString)
            .def("createMainCanvas", raw_function(createMainCanvas))
            .def("createCanvas", raw_function(createCanvas))
            .def("deleteCanvas", &Player::deleteCanvas)
            .def("getMainCanvas", &Player::getMainCanvas)
            .def("getCanvas", &Player::getCanvas)
            .def("play", &Player::play)
            .def("stop", &Player::stop)
            .def("isPlaying", &Player::isPlaying)
            .def("setFramerate", &Player::setFramerate)
            .def("setVBlankFramerate", &Player::setVBlankFramerate)
            .def("getEffectiveFramerate", &Player::getEffectiveFramerate)
            .def("getMemoryUsage", raw_function(playerGetMemoryUsage))
            .def("getTestHelper", &Player::getTestHelper,
                    return_value_policy<reference_existing_object>())
            .def("setFakeFPS", &Player::setFakeFPS)
            .def("getFrameTime", &Player::getFrameTime)
            .def("getFrameDuration", &Player::getFrameDuration)
            .def("createNode", &Player::createNodeFromXmlString)
            .def("createNode", &Player::createNode, Player_createNode_overloads())
            .def("getTouchUserBmp", &Player::getTouchUserBmp)
            .def("enableMouse", &Player::enableMouse)
            .def("setInterval", &Player::setInterval)
            .def("setTimeout", &Player::setTimeout)
            .def("callFromThread", &Player::callFromThread)
            .def("setOnFrameHandler", &Player::setOnFrameHandler)
            .def("clearInterval", &Player::clearInterval)
            .def("addInputDevice", &Player::addInputDevice)
            .def("getMouseState", &Player::getMouseState)
            .def("getCurrentEvent", &Player::getCurrentEvent)
            .def("getKeyModifierState", &Player::getKeyModifierState)
            .def("screenshot", &Player::screenshot)
            .def("keepWindowOpen", &Player::keepWindowOpen)
            .def("stopOnEscape", &Player::setStopOnEscape)
            .def("showCursor", &Player::showCursor)
            .def("isCursorShown", &Player::isCursorShown)
            .def("getElementByID", &Player::getElementByID)
            .def("getRootNode", &Player::getRootNode)
            .def("getFramerate", &Player::getFramerate)
            .def("getVideoRefreshRate", &Player::getVideoRefreshRate)
            .def("getVideoMemInstalled", &Player::getVideoMemInstalled)
            .def("getVideoMemUsed", &Player::getVideoMemUsed)
            .def("setGamma", &Player::setGamma)
            .def("setMousePos", &Player::setMousePos)
            .def("loadPlugin", &Player::loadPlugin)
            .def("setEventHook", &Player::setEventHook)
            .def("getEventHook", &Player::getEventHook)
            .def("getConfigOption", &Player::getConfigOption)
            .def("isUsingGLES", &Player::isUsingGLES)
            .def("areFullShadersSupported", &Player::areFullShadersSupported)
            .add_property("pluginPath", &Player::getPluginPath, &Player::setPluginPath)
            .add_property("volume", &Player::getVolume, &Player::setVolume)
            .add_property("imageCache", make_function(&Player::getImageCache,
                    return_value_policy<reference_existing_object>()))
        ;
        exportMessages(playerClass, "Player");
        
        class_<Canvas, bases<ExportedObject>, boost::noncopyable>("Canvas", no_init)
            .def("getRootNode", &Canvas::getRootNode)
            .def("getElementByID", &Canvas::getElementByID)
            .def("screenshot", &Canvas::screenshot)
        ;

        class_<OffscreenCanvas, bases<Canvas>, boost::noncopyable>
                ("OffscreenCanvas", no_init)
            .def("getID", &OffscreenCanvas::getID)
            .def("render", &OffscreenCanvas::manualRender)
            .def("registerCameraNode", &OffscreenCanvas::registerCameraNode)
            .def("unregisterCameraNode", &OffscreenCanvas::unregisterCameraNode)
            .add_property("handleevents", &OffscreenCanvas::getHandleEvents)
            .add_property("multisamplesamples", &OffscreenCanvas::getMultiSampleSamples)
            .add_property("mipmap", &OffscreenCanvas::getMipmap)
            .add_property("autorender", &OffscreenCanvas::getAutoRender,
                    &OffscreenCanvas::setAutoRender)
            .def("getNumDependentCanvases", &OffscreenCanvas::getNumDependentCanvases)
            .def("isSupported", &OffscreenCanvas::isSupported)
            .staticmethod("isSupported")
            .def("isMultisampleSupported", &OffscreenCanvas::isMultisampleSupported)
            .staticmethod("isMultisampleSupported")
        ;

        class_<VersionInfo>("VersionInfo")
            .add_property("full", &VersionInfo::getFull)
            .add_property("release", &VersionInfo::getRelease)
            .add_property("major", &VersionInfo::getMajor)
            .add_property("minor", &VersionInfo::getMinor)
            .add_property("micro", &VersionInfo::getMicro)
            .add_property("revision", &VersionInfo::getRevision)
            .add_property("builder", &VersionInfo::getBuilder)
            .add_property("buildtime", &VersionInfo::getBuildTime)
            ;
    } catch (const exception& e) {
        PyErr_SetString(PyExc_RuntimeError, e.what());
        throw error_already_set();
    }
}
