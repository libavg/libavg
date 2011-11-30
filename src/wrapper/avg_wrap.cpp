//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2011 Ulrich von Zadow
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
void export_node();
void export_event();
#ifndef WIN32
void export_devices();
#endif
void export_anim();

#include "WrapHelper.h"

#include "../base/Logger.h"
#include "../base/OSHelper.h"
#include "../base/GeomHelper.h"
#include "../player/Player.h"
#include "../player/AVGNode.h"
#include "../player/DivNode.h"
#include "../player/TrackerInputDevice.h"
#include "../player/TouchEvent.h"
#include "../player/MouseEvent.h"
#include "../player/TestHelper.h"
#include "../player/Canvas.h"
#include "../player/OffscreenCanvas.h"
#include "../player/VideoWriter.h"
#include "../player/SVG.h"

#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/raw_function.hpp>

using namespace boost::python;
using namespace avg;
using namespace std;


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(TestHelper_fakeTouchEvent_overloads,
        fakeTouchEvent, 4, 5)

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


BOOST_PYTHON_MODULE(avg)
{
    docstring_options doc_options(true, false);

    export_base();

    register_ptr_to_python<DivNodePtr>();
    register_ptr_to_python<CanvasNodePtr>();
    register_ptr_to_python<AVGNodePtr>();
    register_ptr_to_python<EventPtr>();
    register_ptr_to_python<MouseEventPtr>();
    register_ptr_to_python<TouchEventPtr>();

    def("getMemoryUsage", getMemoryUsage);

    def("pointInPolygon", pointInPolygon);

    class_<Logger>("Logger", no_init)
        .def("get", &Logger::get, 
                return_value_policy<reference_existing_object>())
        .staticmethod("get")
        .def("setCategories", &Logger::setCategories)
        .def("pushCategories", &Logger::pushCategories)
        .def("popCategories", &Logger::popCategories)
        .def("trace", &Logger::trace)
        .def_readonly("NONE", &Logger::NONE)
        .def_readonly("BLTS", &Logger::BLTS)
        .def_readonly("PROFILE", &Logger::PROFILE)
        .def_readonly("PROFILE_LATEFRAMES", &Logger::PROFILE_LATEFRAMES)
        .def_readonly("PROFILE_VIDEO", &Logger::PROFILE_VIDEO)
        .def_readonly("EVENTS", &Logger::EVENTS)
        .def_readonly("EVENTS2", &Logger::EVENTS2)
        .def_readonly("CONFIG", &Logger::CONFIG)
        .def_readonly("WARNING", &Logger::WARNING)
        .def_readonly("ERROR", &Logger::ERROR)
        .def_readonly("MEMORY", &Logger::MEMORY)
        .def_readonly("APP", &Logger::APP)
        .def_readonly("PLUGIN", &Logger::PLUGIN)
        .def_readonly("PLAYER", &Logger::PLAYER)
    ;

#ifndef WIN32
    export_devices();
#endif
    export_event();
    export_node();
    export_anim();

    class_<TestHelper>("TestHelper", no_init)
        .def("fakeMouseEvent", &TestHelper::fakeMouseEvent)
        .def("fakeTouchEvent", &TestHelper::fakeTouchEvent,
                TestHelper_fakeTouchEvent_overloads())
        .def("fakeKeyEvent", &TestHelper::fakeKeyEvent)
        .def("dumpObjects", &TestHelper::dumpObjects)
    ;

    class_<Player>("Player") 
        .def("get", &Player::get, 
                return_value_policy<reference_existing_object>())
        .staticmethod("get")
        .def("setResolution", &Player::setResolution)
        .def("isFullscreen", &Player::isFullscreen)
        .def("setWindowFrame", &Player::setWindowFrame)
        .def("setWindowPos", &Player::setWindowPos)
        .def("setOGLOptions", &Player::setOGLOptions)
        .def("setMultiSampleSamples", &Player::setMultiSampleSamples)
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
        .def("getTestHelper", &Player::getTestHelper,
                return_value_policy<reference_existing_object>())
        .def("setFakeFPS", &Player::setFakeFPS)
        .def("getFrameTime", &Player::getFrameTime)
        .def("getFrameDuration", &Player::getFrameDuration)
        .def("createNode", &Player::createNodeFromXmlString)
        .def("createNode", &Player::createNode)
        .def("addTracker", &Player::addTracker,
                return_value_policy<reference_existing_object>())
        .def("enableMultitouch", &Player::enableMultitouch)
        .def("isMultitouchAvailable", &Player::isMultitouchAvailable)
        .def("getTracker", &Player::getTracker,
                return_value_policy<reference_existing_object>())
        .def("setInterval", &Player::setInterval)
        .def("setTimeout", &Player::setTimeout)
        .def("setOnFrameHandler", &Player::setOnFrameHandler)
        .def("clearInterval", &Player::clearInterval)
        .def("addInputDevice", &Player::addInputDevice)
        .def("getMouseState", &Player::getMouseState)
        .def("getKeyModifierState", &Player::getKeyModifierState)
        .def("screenshot", &Player::screenshot)
        .def("stopOnEscape", &Player::setStopOnEscape)
        .def("showCursor", &Player::showCursor)
        .def("setCursor", &Player::setCursor)
        .def("getElementByID", &Player::getElementByID)
        .def("getRootNode", &Player::getRootNode)
        .def("getFramerate", &Player::getFramerate)
        .def("getVideoRefreshRate", &Player::getVideoRefreshRate)
        .def("isUsingShaders", &Player::isUsingShaders)
        .def("setGamma", &Player::setGamma)
        .def("setMousePos", &Player::setMousePos)
        .def("loadPlugin", &Player::loadPlugin)
        .def("setEventHook", &Player::setEventHook)
        .def("getEventHook", &Player::getEventHook)
        .add_property("pluginPath", &Player::getPluginPath, &Player::setPluginPath)
        .add_property("volume", &Player::getVolume, &Player::setVolume)
    ;

    class_<Canvas, boost::shared_ptr<Canvas>, boost::noncopyable>("Canvas", no_init)
        .def(self == self)
        .def(self != self)
        .def("__hash__", &Canvas::getHash)
        .def("getRootNode", &Canvas::getRootNode)
        .def("getElementByID", &Canvas::getElementByID)
        .def("screenshot", &Canvas::screenshot)
    ;

    class_<OffscreenCanvas, boost::shared_ptr<OffscreenCanvas>, bases<Canvas>,
            boost::noncopyable>("OffscreenCanvas", no_init)
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

    class_<VideoWriter, boost::shared_ptr<VideoWriter>, boost::noncopyable>
            ("VideoWriter", no_init)
        .def(init<CanvasPtr, const std::string&, int, int, int, bool>())
        .def(init<CanvasPtr, const std::string&, int, int, int>())
        .def(init<CanvasPtr, const std::string&, int>())
        .def("stop", &VideoWriter::stop)
        .def("pause", &VideoWriter::pause)
        .def("play", &VideoWriter::play)
        .add_property("filename", &VideoWriter::getFileName)
        .add_property("framerate", &VideoWriter::getFramerate)
        .add_property("qmin", &VideoWriter::getQMin)
        .add_property("qmax", &VideoWriter::getQMax)
    ;

    BitmapPtr (SVG::*renderElement1)(const UTF8String&) = &SVG::renderElement;
    BitmapPtr (SVG::*renderElement2)(const UTF8String&, const glm::vec2&) = 
            &SVG::renderElement;
    BitmapPtr (SVG::*renderElement3)(const UTF8String&, float) = 
            &SVG::renderElement;
    NodePtr (SVG::*createImageNode1)(const UTF8String&, const dict&) = 
            &SVG::createImageNode;
    NodePtr (SVG::*createImageNode2)(const UTF8String&, const dict&, const glm::vec2&) = 
            &SVG::createImageNode;
    NodePtr (SVG::*createImageNode3)(const UTF8String&, const dict&, float) = 
            &SVG::createImageNode;

    class_<SVG, boost::noncopyable>("SVG", init<const UTF8String&, bool>())
        .def("renderElement", renderElement1)
        .def("renderElement", renderElement2)
        .def("renderElement", renderElement3)
        .def("createImageNode", createImageNode1)
        .def("createImageNode", createImageNode2)
        .def("createImageNode", createImageNode3)
        .def("getElementSize", &SVG::getElementSize)
        ;

}
