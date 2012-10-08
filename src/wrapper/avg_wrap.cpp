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
void export_anim();

#include "WrapHelper.h"
#include "raw_constructor.hpp"

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
#include "../player/Style.h"
#include "../player/VersionInfo.h"

#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/raw_function.hpp>

using namespace boost::python;
using namespace avg;
using namespace std;


BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(TestHelper_fakeTouchEvent_overloads,
        fakeTouchEvent, 4, 5)
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

avg::StylePtr createStyle(const boost::python::tuple &args,
        const boost::python::dict &attrs)
{
    checkEmptyArgs(args);
    return StylePtr(new avg::Style(attrs));
}


BOOST_PYTHON_MODULE(avg)
{
    docstring_options doc_options(true, false);

    Player::get();
    export_base();

    register_ptr_to_python<DivNodePtr>();
    register_ptr_to_python<CanvasNodePtr>();
    register_ptr_to_python<AVGNodePtr>();
    register_ptr_to_python<EventPtr>();
    register_ptr_to_python<MouseEventPtr>();
    register_ptr_to_python<TouchEventPtr>();

    def("getMemoryUsage", getMemoryUsage);

    def("pointInPolygon", pointInPolygon);

    class_<MessageID>("MessageID", no_init);

    class_<Logger>("Logger", no_init)
        .def("get", &Logger::get, 
                return_value_policy<reference_existing_object>())
        .staticmethod("get")
        .def("setCategories", &Logger::setCategories)
        .def("pushCategories", &Logger::pushCategories)
        .def("popCategories", &Logger::popCategories)
        .def("trace", &Logger::trace)
        .def_readonly("NONE", &Logger::NONE)
        .def_readonly("PROFILE", &Logger::PROFILE)
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

    class_<Publisher, boost::noncopyable>("Publisher")
        .def("subscribe", &Publisher::subscribe)
        .def("unsubscribe", &Publisher::unsubscribeCallable)
        .def("unsubscribe", &Publisher::unsubscribe)
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

    class_<TestHelper>("TestHelper", no_init)
        .def("fakeMouseEvent", &TestHelper::fakeMouseEvent)
        .def("fakeTouchEvent", &TestHelper::fakeTouchEvent,
                TestHelper_fakeTouchEvent_overloads())
        .def("fakeKeyEvent", &TestHelper::fakeKeyEvent)
        .def("dumpObjects", &TestHelper::dumpObjects)
    ;

    enum_<GLConfig::ShaderUsage>("ShaderUsage")
        .value("SHADERUSAGE_FULL", GLConfig::FULL)
        .value("SHADERUSAGE_MINIMAL", GLConfig::MINIMAL)
        .value("SHADERUSAGE_AUTO", GLConfig::AUTO)
        .export_values()
    ;

    object playerClass = class_<Player, bases<Publisher>, boost::noncopyable>("Player") 
        .def("get", &Player::get, 
                return_value_policy<reference_existing_object>())
        .staticmethod("get")
        .def("setResolution", &Player::setResolution)
        .def("isFullscreen", &Player::isFullscreen)
        .def("setWindowFrame", &Player::setWindowFrame)
        .def("setWindowPos", &Player::setWindowPos)
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
        .def("getTestHelper", &Player::getTestHelper,
                return_value_policy<reference_existing_object>())
        .def("setFakeFPS", &Player::setFakeFPS)
        .def("getFrameTime", &Player::getFrameTime)
        .def("getFrameDuration", &Player::getFrameDuration)
        .def("createNode", &Player::createNodeFromXmlString)
        .def("createNode", &Player::createNode, Player_createNode_overloads())
        .def("enableMultitouch", &Player::enableMultitouch)
        .def("isMultitouchAvailable", &Player::isMultitouchAvailable)
        .def("getTracker", &Player::getTracker,
                return_value_policy<reference_existing_object>())
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
        .def("setCursor", &Player::setCursor)
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
        .add_property("pluginPath", &Player::getPluginPath, &Player::setPluginPath)
        .add_property("volume", &Player::getVolume, &Player::setVolume)
    ;
    exportMessages(playerClass, "Player");
    
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

    class_<SVG, boost::noncopyable>("SVG", no_init)
        .def(init<const UTF8String&>())
        .def(init<const UTF8String&, bool>())
        .def("renderElement", renderElement1)
        .def("renderElement", renderElement2)
        .def("renderElement", renderElement3)
        .def("createImageNode", createImageNode1)
        .def("createImageNode", createImageNode2)
        .def("createImageNode", createImageNode3)
        .def("getElementSize", &SVG::getElementSize)
        ;

    class_<Style, StylePtr, boost::noncopyable>("Style", no_init)
        .def("__init__", raw_constructor(createStyle))
        .def("__getitem__", &Style::__getitem__)
        .def("__contains__", &Style::__contains__)
        .def("has_key", &Style::__contains__)
        .def("keys", &Style::keys)
        .def("values", &Style::values)
        .def("items", &Style::items)
        .def("__len__", &Style::__len__)
        .def("__iter__", &Style::__iter__)
        .def("iteritems", &Style::iteritems)
        .def("iterkeys", &Style::iterkeys)
        .def("itervalues", &Style::itervalues)
        .def("__repr__", &Style::__repr__)
        ;

    class_<VersionInfo>("VersionInfo")
        .add_property("full", &VersionInfo::getFull)
        .add_property("release", &VersionInfo::getRelease)
        .add_property("major", &VersionInfo::getMajor)
        .add_property("minor", &VersionInfo::getMinor)
        .add_property("micro", &VersionInfo::getMicro)
        .add_property("revision", &VersionInfo::getRevision)
        .add_property("branchurl", &VersionInfo::getBranchUrl)
        .add_property("builder", &VersionInfo::getBuilder)
        .add_property("buildtime", &VersionInfo::getBuildTime)
        ;
}
