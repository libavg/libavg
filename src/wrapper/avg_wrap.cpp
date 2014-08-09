//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2014 Ulrich von Zadow
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
#include "../base/XMLHelper.h"
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
#include "../player/VersionInfo.h"
#include "../player/ExportedObject.h"

#include <boost/version.hpp>
#include <boost/shared_ptr.hpp>
#include <boost/python/raw_function.hpp>

using namespace boost::python;
using namespace avg;
using namespace std;

namespace bp = boost::python;

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

class SeverityScopeHelper{};
class CategoryScopeHelper{};


boost::function<size_t (const bp::tuple& args, const bp::dict& kwargs )>
        playerGetMemoryUsage = boost::bind(getMemoryUsage);

// [todo] - remove after releasing libavg-v2.0.0
size_t getMemoryUsageDeprecated() {
    avgDeprecationWarning("1.9.0", "avg.getMemoryUsage", "player.getMemoryUsage");
    return getMemoryUsage();
}

bool pointInPolygonDepcrecated(const glm::vec2& pt, const std::vector<glm::vec2>& poly) {
    avgDeprecationWarning("1.9.0", "avg.pointInPolygon", "Point2D.isInPolygon");
    return pointInPolygon(pt, poly);
}
// end remove


BOOST_PYTHON_MODULE(avg)
{
    try {
        docstring_options doc_options(true, false);

        Player::get();
        PyEval_InitThreads();
        export_base();

        register_ptr_to_python<DivNodePtr>();
        register_ptr_to_python<CanvasNodePtr>();
        register_ptr_to_python<AVGNodePtr>();
        register_ptr_to_python<EventPtr>();
        register_ptr_to_python<MouseEventPtr>();
        register_ptr_to_python<TouchEventPtr>();

        // [todo] - remove after releasing libavg-v2.0.0
        def("getMemoryUsage", getMemoryUsageDeprecated);
        def("pointInPolygon", pointInPolygonDepcrecated);
        // end remove

        def("validateXml", validateXml);

        class_<MessageID>("MessageID", no_init)
            .def("__repr__", &MessageID::getRepr)
        ;

        {
           scope loggerScope = class_<Logger, boost::noncopyable>("Logger", no_init)
                .def("addSink", addPythonLogger)
                .def("removeSink", removePythonLogger)
                .def("removeStdLogSink", &Logger::removeStdLogSink)
                .def("configureCategory", &Logger::configureCategory,
                        (bp::arg("severity")=Logger::severity::NONE))
                .def("getCategories", &Logger::getCategories)
                .def("trace", pytrace,
                        (bp::arg("severity")=Logger::severity::INFO))
                .def("debug", &Logger::logDebug,
                        (bp::arg("category")=Logger::category::APP))
                .def("info", &Logger::logInfo,
                        (bp::arg("category")=Logger::category::APP))
                .def("warning", &Logger::logWarning,
                        (bp::arg("category")=Logger::category::APP))
                .def("error", &Logger::logError,
                        (bp::arg("category")=Logger::category::APP))
                .def("critical", &Logger::logCritical,
                        (bp::arg("category")=Logger::category::APP))
                .def("log", &Logger::log,
                        (bp::arg("category")=Logger::category::APP,
                         bp::arg("severity")=Logger::severity::INFO))
           ;
            {
                scope severityScope = class_<SeverityScopeHelper>("Severity");
                severityScope.attr("CRIT") = Logger::severity::CRITICAL;
                severityScope.attr("ERR") = Logger::severity::ERROR;
                severityScope.attr("WARN") = Logger::severity::WARNING;
                severityScope.attr("INFO") = Logger::severity::INFO;
                severityScope.attr("DBG") = Logger::severity::DEBUG;
                severityScope.attr("NONE") = Logger::severity::NONE;
            }
            {
                scope categoryScope = class_<CategoryScopeHelper>("Category");
                categoryScope.attr("APP") = Logger::category::APP;
                categoryScope.attr("CONFIG") = Logger::category::CONFIG;
                categoryScope.attr("DEPREC") = Logger::category::DEPRECATION;
                categoryScope.attr("EVENTS") = Logger::category::EVENTS;
                categoryScope.attr("MEMORY") = Logger::category::MEMORY;
                categoryScope.attr("NONE") = Logger::category::NONE;
                categoryScope.attr("PROFILE") = Logger::category::PROFILE;
                categoryScope.attr("PROFILE_V") = Logger::category::PROFILE_VIDEO;
                categoryScope.attr("PLUGIN") = Logger::category::PLUGIN;
                categoryScope.attr("PLAYER") = Logger::category::PLAYER;
                categoryScope.attr("SHADER") = Logger::category::SHADER;
            }
        }

        scope().attr("logger") = boost::python::ptr(Logger::get());

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

        class_<TestHelper>("TestHelper", no_init)
            .def("fakeMouseEvent", &TestHelper::fakeMouseEvent)
            .def("fakeTouchEvent", &TestHelper::fakeTouchEvent,
                    TestHelper_fakeTouchEvent_overloads())
            .def("fakeTangibleEvent", &TestHelper::fakeTangibleEvent)
            .def("fakeKeyEvent", &TestHelper::fakeKeyEvent)
            .def("dumpObjects", &TestHelper::dumpObjects)
            .def("getObjectCount", &TestHelper::getObjectCount)
        ;

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
            .def("enableMultitouch", &Player::enableMultitouch)
            .def("enableMouse", &Player::enableMouse)
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
            .def("isUsingGLES", &Player::isUsingGLES)
            .def("areFullShadersSupported", &Player::areFullShadersSupported)
            .add_property("pluginPath", &Player::getPluginPath, &Player::setPluginPath)
            .add_property("volume", &Player::getVolume, &Player::setVolume)
        ;
        exportMessages(playerClass, "Player");
        
        class_<Canvas, boost::shared_ptr<Canvas>, bases<ExportedObject>, 
                boost::noncopyable>("Canvas", no_init)
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
            .def("getElementPos", &SVG::getElementPos)
            .def("getElementSize", &SVG::getElementSize)
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
