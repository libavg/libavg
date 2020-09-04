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

#include "WrapHelper.h"

#include "../base/GeomHelper.h"
#include "../base/OSHelper.h"
#include "../base/XMLHelper.h"
#include "../base/Logger.h"
#include "../player/MessageID.h"
#include "../player/TestHelper.h"
#include "../player/VideoWriter.h"
#include "../player/SVG.h"

using namespace boost::python;
using namespace avg;
using namespace std;

namespace bp = boost::python;

// [todo] - remove after releasing libavg-v2.0.0
size_t getMemoryUsageDeprecated() 
{
    avgDeprecationWarning("1.9.0", "avg.getMemoryUsage", "player.getMemoryUsage");
    return getMemoryUsage();
}

bool pointInPolygonDepcrecated(const glm::vec2& pt, const std::vector<glm::vec2>& poly) 
{
    avgDeprecationWarning("1.9.0", "avg.pointInPolygon", "Point2D.isInPolygon");
    return pointInPolygon(pt, poly);
}
// end remove

class SeverityScopeHelper{};
class CategoryScopeHelper{};

BOOST_PYTHON_MEMBER_FUNCTION_OVERLOADS(TestHelper_fakeTouchEvent_overloads,
        fakeTouchEvent, 4, 5)

void export_misc()
{
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
    
    class_<TestHelper>("TestHelper", no_init)
        .def("fakeMouseEvent", &TestHelper::fakeMouseEvent)
        .def("fakeMouseWheelEvent", &TestHelper::fakeMouseWheelEvent)
        .def("fakeTouchEvent", &TestHelper::fakeTouchEvent,
                TestHelper_fakeTouchEvent_overloads())
        .def("fakeTangibleEvent", &TestHelper::fakeTangibleEvent)
        .def("fakeKeyEvent", &TestHelper::fakeKeyEvent)
        .def("dumpObjects", &TestHelper::dumpObjects)
        .def("getObjectCount", &TestHelper::getObjectCount)
    ;

    class_<VideoWriter, boost::shared_ptr<VideoWriter>, boost::noncopyable>
            ("VideoWriter", no_init)
        .def(init<CanvasPtr, const std::string&, int, int, int, bool>())
        .def(init<CanvasPtr, const std::string&, int, int, int>())
        .def(init<CanvasPtr, const std::string&, int>())
        .def(init<CanvasPtr, const std::string&>())
        .def("stop", &VideoWriter::stop)
        .def("pause", &VideoWriter::pause)
        .def("play", &VideoWriter::play)
        .add_property("filename", &VideoWriter::getFileName)
        .add_property("framerate", &VideoWriter::getFramerate)
        .add_property("qmin", &VideoWriter::getQMin)
        .add_property("qmax", &VideoWriter::getQMax)
        .add_property("synctoplayback", &VideoWriter::getSyncToPlayback)
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

}

