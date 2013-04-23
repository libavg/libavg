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

#include "DAG.h"
#include "Queue.h"
#include "Command.h"
#include "WorkerThread.h"
#include "ObjectCounter.h"
#include "triangulate/Triangulate.h"
#include "GLMHelper.h"
#include "GeomHelper.h"
#include "OSHelper.h"
#include "FileHelper.h"
#include "StringHelper.h"
#include "MathHelper.h"
#include "CubicSpline.h"
#include "BezierCurve.h"
#include "Signal.h"
#include "Backtrace.h"
#include "WideLine.h"
#include "Rect.h"
#include "Triangle.h"
#include "TestSuite.h"
#include "TimeSource.h"
#include "XMLHelper.h"
#include "Logger.h"

#include <boost/thread/thread.hpp>

#include <boost/bind.hpp>

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

using namespace avg;
using namespace std;
using namespace boost;

class DAGTest: public Test
{
public:
    DAGTest()
        : Test("DAGTest", 2)
    {
    }

    void runTests() 
    {
        {
            DAG dag;

            dag.addNode(1, set<long>());
            long outgoing2[] = {1};
            dag.addNode(0, makeOutgoing(1, outgoing2));
            
            long expected[] = {0, 1};
            checkResults(&dag, expected);
        }
        {
            DAG dag;

            dag.addNode(2, set<long>());
            long outgoing2[] = {1};
            dag.addNode(0, makeOutgoing(1, outgoing2));
            long outgoing3[] = {2};
            dag.addNode(1, makeOutgoing(1, outgoing3));

            long expected[] = {0, 1, 2};
            checkResults(&dag, expected);
        }
        {
            DAG dag;

            long outgoing2[] = {1};
            dag.addNode(0, makeOutgoing(1, outgoing2));
            dag.addNode(2, set<long>());
            long outgoing3[] = {2};
            dag.addNode(1, makeOutgoing(1, outgoing3));

            long expected[] = {0, 1, 2};
            checkResults(&dag, expected);
        }
        {
            DAG dag;

            dag.addNode(2, set<long>());
            long outgoing2[] = {1, 2};
            dag.addNode(0, makeOutgoing(1, outgoing2));
            long outgoing3[] = {2};
            dag.addNode(1, makeOutgoing(1, outgoing3));

            long expected[] = {0, 1, 2};
            checkResults(&dag, expected);
        }
        {
            DAG dag;

            long outgoing2[] = {1};
            dag.addNode(0, makeOutgoing(1, outgoing2));
            long outgoing3[] = {0};
            dag.addNode(1, makeOutgoing(1, outgoing3));

            bool bExceptionThrown = false;
            long expected[] = {0, 1, 2};
            try {
                checkResults(&dag, expected);
            } catch (const Exception&) {
                bExceptionThrown = true;
            }
            TEST(bExceptionThrown);
        }
    }

private:
    set<long> makeOutgoing(int n, long ids[])
    {
        set<long> v;
        for (int i=0; i<n; ++i) {
            v.insert(ids[i]);
        }
        return v;
    }

    void checkResults(DAG* pDAG, long expected[])
    {
        vector<long> results;
        pDAG->sort(results);

        for (unsigned i=0; i<results.size(); ++i) {
            QUIET_TEST(results[i] == expected[i]);
        }
    }
};

class QueueTest: public Test
{
public:
    QueueTest()
        : Test("QueueTest", 2)
    {
    }

    void runTests() 
    {
        runSingleThreadTests();
        runMultiThreadTests();
    }

private:
    typedef Queue<int>::QElementPtr ElemPtr;
    
    void runSingleThreadTests()
    {
        Queue<string> q;
        typedef Queue<string>::QElementPtr ElemPtr;
        TEST(q.empty());
        q.push(ElemPtr(new string("1")));
        TEST(q.size() == 1);
        TEST(!q.empty());
        q.push(ElemPtr(new string("2")));
        q.push(ElemPtr(new string("3")));
        TEST(q.size() == 3);
        TEST(*q.pop() == "1");
        TEST(*q.pop() == "2");
        q.push(ElemPtr(new string("4")));
        TEST(*q.pop() == "3");
        TEST(*q.peek() == "4");
        TEST(*q.pop() == "4");
        TEST(q.empty());
        ElemPtr pElem = q.pop(false);
        TEST(!pElem);
    }

    void runMultiThreadTests()
    {
        {
            Queue<int> q(10);
            thread pusher(boost::bind(&pushThread, &q, 100));
            thread popper(boost::bind(&popThread, &q, 100));
            pusher.join();
            popper.join();
            TEST(q.empty());
        }
        {
            Queue<int> q(10);
            thread pusher1(boost::bind(&pushThread, &q, 100));
            thread pusher2(boost::bind(&pushThread, &q, 100));
            thread popper(boost::bind(&popThread, &q, 200));
            pusher1.join();
            pusher2.join();
            popper.join();
            TEST(q.empty());
        }
        {
            Queue<int> q(10);
            thread pusher(boost::bind(&pushClearThread, &q, 100));
            thread popper(boost::bind(&popClearThread, &q));
            pusher.join();
            popper.join();
            TEST(q.empty());
        }
    }

    static void pushThread(Queue<int>* pq, int numPushes)
    {
        for (int i=0; i<numPushes; ++i) {
            pq->push(ElemPtr(new int(i)));
            msleep(1);
        }
    }

    static void popThread(Queue<int>* pq, int numPops)
    {
        for (int i=0; i<numPops; ++i) {
            pq->peek();
            pq->pop();
            msleep(3);
        }
    }

    static void pushClearThread(Queue<int>* pq, int numPushes)
    {
        typedef Queue<int>::QElementPtr ElemPtr;
        for (int i=0; i<numPushes; ++i) {
            pq->push(ElemPtr(new int(i)));
            if (i%7 == 0) {
                pq->clear();
            }
            msleep(1);
        }
        pq->push(ElemPtr(new int(-1)));
    }

    static void popClearThread(Queue<int>* pq)
    {
        ElemPtr pElem;
        do {
            pElem = pq->pop();
        } while (*pElem != -1);
    }
};

class TestWorkerThread: public WorkerThread<TestWorkerThread>
{
public:
    TestWorkerThread(CQueue& cmdQ, int* pNumFuncCalls, int* pIntParam, 
            string* pStringParam)
        : WorkerThread<TestWorkerThread>("Thread1", cmdQ),
          m_pNumFuncCalls(pNumFuncCalls),
          m_pIntParam(pIntParam),
          m_pStringParam(pStringParam)
    {
    }

    bool init() 
    {
        (*m_pNumFuncCalls)++;
        return true;
    }

    bool work()
    {
        (*m_pNumFuncCalls)++;
        waitForCommand();
        return true;
    }

    void deinit()
    {
        (*m_pNumFuncCalls)++;
    }

    void doSomething(int i, std::string s)
    {
        *m_pIntParam = i;
        *m_pStringParam = s;
    }

private:
    int * m_pNumFuncCalls;
    int * m_pIntParam;
    std::string * m_pStringParam;
};


class WorkerThreadTest: public Test
{
public:
    WorkerThreadTest()
        : Test("WorkerThreadTest", 2)
    {
    }

    void runTests() 
    {
        typedef TestWorkerThread::CmdPtr CmdPtr;
        TestWorkerThread::CQueue cmdQ;
        boost::thread* pTestThread;
        int numFuncCalls = 0;
        int intParam = 0;
        std::string stringParam;
        cmdQ.pushCmd(boost::bind(&TestWorkerThread::doSomething, _1, 23, "foo"));
        cmdQ.pushCmd(boost::bind(&TestWorkerThread::stop, _1));
        pTestThread = new boost::thread(TestWorkerThread(cmdQ, &numFuncCalls,
                &intParam, &stringParam));
        pTestThread->join();
        delete pTestThread;
        TEST(numFuncCalls == 3);
        TEST(intParam == 23);
        TEST(stringParam == "foo");
    }
};


class DummyClass
{
public:
    DummyClass()
    {
        ObjectCounter::get()->incRef(&typeid(*this));
    }

    virtual ~DummyClass()
    {
        ObjectCounter::get()->decRef(&typeid(*this));
    }

    int i;
};


class ObjectCounterTest: public Test {
public:
    ObjectCounterTest()
        : Test("ObjectCounterTest", 2)
    {
    }

    void runTests() 
    {
        TEST(ObjectCounter::get()->getCount(&typeid(DummyClass)) == 0);
        {
            DummyClass dummy1;
            DummyClass dummy2;
            TEST(ObjectCounter::get()->getCount(&typeid(dummy1)) == 2);
        }
        TEST(ObjectCounter::get()->getCount(&typeid(DummyClass)) == 0);
    }
};


// The following pragmas avoid a compiler warning (potential division by 0)
#ifdef _MSC_VER
#pragma optimize("", off)
#pragma warning(push)
#pragma warning(disable:4723)
#endif
class GeomTest: public Test
{
public:
    GeomTest()
        : Test("GeomTest", 2)
    {
    }

    void runTests() 
    {
        // TODO: Move to a separate math test once we're done here.
        TEST(almostEqual(invSqrt(1), 1));
        TEST(almostEqual(invSqrt(4), 0.5));

        {
            LineSegment l1(glm::vec2(0,0), glm::vec2(2,2));
            LineSegment l2(glm::vec2(2,0), glm::vec2(0,2));
            TEST(lineSegmentsIntersect(l1, l2));
            TEST(lineSegmentsIntersect(l2, l1));
        }
        {
            LineSegment l1(glm::vec2(0,0), glm::vec2(0,2));
            LineSegment l2(glm::vec2(2,0), glm::vec2(2,2));
            TEST(!lineSegmentsIntersect(l1, l2));
        }
        {
            LineSegment l1(glm::vec2(0,0), glm::vec2(2,0));
            LineSegment l2(glm::vec2(0,2), glm::vec2(2,2));
            TEST(!lineSegmentsIntersect(l1, l2));
        }
        {
            LineSegment l1(glm::vec2(0,0), glm::vec2(2,0));
            TEST(l1.isPointOver(glm::vec2(1,23)));
            TEST(l1.isPointOver(glm::vec2(1.9,-5)));
            TEST(!l1.isPointOver(glm::vec2(-1,1)));
            TEST(!l1.isPointOver(glm::vec2(3,-1)));
        }
        {
            glm::vec2 pt0(glm::vec2(1,1));
            glm::vec2 pt1(glm::vec2(1,3));
            glm::vec2 pt2(glm::vec2(1,-2));
            vector<glm::vec2> poly;
            poly.push_back(glm::vec2(0,0));
            poly.push_back(glm::vec2(2,0));
            poly.push_back(glm::vec2(2,2));
            poly.push_back(glm::vec2(0,2));
            TEST(pointInPolygon(pt0, poly));
            TEST(!pointInPolygon(pt1, poly));
            TEST(!pointInPolygon(pt2, poly));
            poly.push_back(glm::vec2(2,1));
            TEST(!pointInPolygon(pt0, poly));
        }
        {
            glm::vec2 p1(glm::vec2(0,0));
            glm::vec2 v1(glm::vec2(1,1));
            glm::vec2 p2(glm::vec2(2,1));
            glm::vec2 v2(glm::vec2(1,0));
            TEST(getLineLineIntersection(p1, v1, p2, v2) == glm::vec2(1,1));
        }
        TEST(almostEqual(getRotatedPivot(glm::vec2(10,0), M_PI, glm::vec2(15,5)), 
                glm::vec2(20,10)));
        TEST(almostEqual(getRotatedPivot(glm::vec2(10,0), M_PI*0.5, glm::vec2(15,5)),
                glm::vec2(20,0)));
        TEST(almostEqual(getRotatedPivot(glm::vec2(10,0), M_PI*1.5, glm::vec2(15,5)), 
                glm::vec2(10,10)));
        TEST(almostEqual(getRotatedPivot(glm::vec2(10,0), M_PI*2, glm::vec2(15,5)), 
                glm::vec2(10,0)));
        TEST(almostEqual(getRotatedPivot(glm::vec2(23,0), M_PI*0.5), glm::vec2(0,23)));

        {
            // TODO: More tests
            FRect(0,0,10,10);
        }
    }
};
#ifdef _MSC_VER
#pragma warning(pop)
#pragma optimize("", on)
#endif


class TriangleTest: public Test
{
public:
    TriangleTest()
        : Test("TriangleTest", 2)
    {
    }

    void runTests()
    {
        Triangle tri(glm::vec2(0,0), glm::vec2(4,4), glm::vec2(4,8));
        TEST(tri.isInside(glm::vec2(3,4)));
        TEST(!tri.isInside(glm::vec2(1,4)));
        TEST(!tri.isInside(glm::vec2(2,1)));
        TEST(!tri.isInside(glm::vec2(-2,5)));
        TEST(!tri.isInside(glm::vec2(5,5)));
        tri = Triangle(glm::vec2(0,0), glm::vec2(4,8), glm::vec2(4,4));
        TEST(tri.isInside(glm::vec2(3,4)));

        glm::vec2 polyArray[] = {glm::vec2(0,0), glm::vec2(8,2), glm::vec2(9,0), glm::vec2(9,3), 
                glm::vec2(1,1), glm::vec2(0,3)}; 

        Vec2Vector poly = vectorFromCArray(6, polyArray);
        vector<unsigned int> triangulation;
        triangulatePolygon(triangulation, poly);

        TEST(triangulation.size() == 4*3);
        unsigned int baselineIndexes[] = {5,0,4, 1,4,0, 4,1,3, 1,2,3};
        TEST(triangulation == vectorFromCArray(12, baselineIndexes));
/*
        for (unsigned int i=0; i<triangulation.size(); i++) {
            cerr << i << ":" << triangulation[i] << endl;
        }
*/
    }

};


class FileTest: public Test
{
public:
    FileTest()
        : Test("FileTest", 2)
    {
    }

    void runTests()
    {
        TEST(getPath("/foo/bar.txt") == "/foo/");
        TEST(getFilenamePart("/foo/bar.txt") == "bar.txt");
    }
};


class OSTest: public Test
{
public:
    OSTest()
        : Test("OSTest", 2)
    {
    }

    void runTests()
    {
        cerr << getAvgLibPath() << endl;
        TEST(getAvgLibPath() != "");
#ifdef __APPLE__
        TEST(getMemoryUsage() != 0);
#endif
    }
};


class StringTest: public Test
{
public:
    StringTest()
        : Test("StringTest", 2)
    {
    }

    void runTests()
    {
        TEST(stringToInt("5") == 5);
        TEST(almostEqual(stringToFloat("5.5"), 5.5f));
        TEST(stringToBool("False") == false);
        bool bExceptionThrown = false;
        try {
            stringToInt("5a");
        } catch (const Exception& e) {
            if (e.getCode() == AVG_ERR_TYPE ) {
                bExceptionThrown = true;
            }
        }
        TEST(bExceptionThrown);
        TEST(stringToVec2(" ( 3.4 , 2.1 ) ") == glm::vec2(3.4f, 2.1f));
        vector<float> v;
        fromString("(1,2,3,4,5)", v);
        TEST(v.size() == 5 && v[0] == 1 && v[4] == 5);
        v.clear();
        fromString("()", v);
        TEST(v.size() == 0);
    }
};


class SplineTest: public Test
{
public:
    SplineTest()
        : Test("SplineTest", 2)
    {
    }

    void runTests()
    {
        {
            float xd[] = {0,1,2,3};
            vector<float> x = vectorFromCArray(4, xd);
            float yd[] = {3,2,1,0};
            vector<float> y = vectorFromCArray(4, yd);
            CubicSpline spline(x, y);
            TEST(almostEqual(spline.interpolate(-1), 4));
            TEST(almostEqual(spline.interpolate(0), 3));
            TEST(almostEqual(spline.interpolate(0.5), 2.5));
            TEST(almostEqual(spline.interpolate(3), 0));
            TEST(almostEqual(spline.interpolate(3.5), -0.5));
        }
        {
            float xd[] = {2,4,6,8};
            vector<float> x = vectorFromCArray(4, xd);
            float yd[] = {0,1,3,6};
            vector<float> y = vectorFromCArray(4, yd);
            CubicSpline spline(x, y);
            TEST(almostEqual(spline.interpolate(0), -1));
            TEST(almostEqual(spline.interpolate(2), 0));
            TEST(spline.interpolate(3) < 0.5);
            TEST(spline.interpolate(3) > 0);
            TEST(spline.interpolate(7) > 4);
            TEST(spline.interpolate(7) < 5);
            TEST(almostEqual(spline.interpolate(8), 6));
            TEST(almostEqual(spline.interpolate(10), 9));
        }
        {
            float xd[] = {0,1,1};
            vector<float> x = vectorFromCArray(3, xd);
            float yd[] = {1,2,1};
            vector<float> y = vectorFromCArray(3, yd);
            bool bExceptionThrown = false;
            try {
                CubicSpline spline(x, y);
            } catch (const Exception&) {
                bExceptionThrown = true;
            }
            TEST(bExceptionThrown);
        }
/*
        {
            float xd[] = {0,1,2};
            vector<float> x = vectorFromCArray(3, xd);
            float yd[] = {1,2,1};
            vector<float> y = vectorFromCArray(3, yd);
            CubicSpline spline(x, y, true);
            TEST(almostEqual(spline.interpolate(0), 1));
            TEST(almostEqual(spline.interpolate(0.5), 1.5));
            TEST(almostEqual(spline.interpolate(2), 1));
            TEST(almostEqual(spline.interpolate(3), 2));
        }
*/
    }
};


class BezierCurveTest: public Test
{
public:
    BezierCurveTest()
        : Test("BezierCurveTest", 2)
    {
    }

    void runTests()
    {
        BezierCurve curve(glm::vec2(0,0), glm::vec2(1,0), glm::vec2(1,1), glm::vec2(0,1));
        TEST(almostEqual(curve.interpolate(0), glm::vec2(0,0)));
        TEST(almostEqual(curve.getDeriv(0), glm::vec2(3, 0)));
        TEST(almostEqual(curve.interpolate(1), glm::vec2(0,1)));
        TEST(almostEqual(curve.getDeriv(1), glm::vec2(-3, 0)));
        TEST(almostEqual(curve.interpolate(0.5), glm::vec2(0.75,0.5)));
    }
};


class WideLineTest: public Test
{
public:
    WideLineTest()
        : Test("WideLineTest", 2)
    {
    }

    void runTests()
    {
        WideLine line(glm::vec2(0,0), glm::vec2(4,3), 2);
        TEST(almostEqual(line.getLen(), 5));
    }
};


class Listener
{
public:
    Listener(Signal<Listener>& signal)
        : m_Signal(signal),
          m_bFuncCalled(false)
    {
    }

    virtual ~Listener()
    {}

    virtual void func()
    {
        m_bFuncCalled = true;
    }

    bool funcCalled() const
    {
        return m_bFuncCalled;
    }

    void reset()
    {
        m_bFuncCalled = false;
    }

protected:
    Signal<Listener>& m_Signal;

private:
    bool m_bFuncCalled;
};


class DisconnectingSelfListener: public Listener
{
public:
    DisconnectingSelfListener(Signal<Listener>& signal)
        : Listener(signal)
    {
    }

    virtual void func()
    {
        Listener::func();
        m_Signal.disconnect(this);
    }
};


class DisconnectingOtherListener: public Listener
{
public:
    DisconnectingOtherListener(Signal<Listener>& signal, Listener* pOther)
        : Listener(signal),
          m_pOther(pOther)
    {
    }

    virtual void func()
    {
        Listener::func();
        if (m_pOther) {
            m_Signal.disconnect(m_pOther);
            m_pOther = 0;
        }
    }

private:
    Listener* m_pOther;
};


class ConnectingOtherListener: public Listener
{
public:
    ConnectingOtherListener(Signal<Listener>& signal, Listener* pOther)
        : Listener(signal),
          m_pOther(pOther)
    {
    }

    virtual void func()
    {
        Listener::func();
        if (m_pOther) {
            m_Signal.connect(m_pOther);
            m_pOther = 0;
        }
    }

private:
    Listener* m_pOther;
};


class SignalTest: public Test
{
public:
    SignalTest()
        : Test("SignalTest", 2)
    {
    }

    void runTests()
    {
        Signal<Listener> s(&Listener::func);
        Listener l1(s);
        Listener l2(s);
        s.connect(&l1);
        s.connect(&l2);
        s.emit();
        TEST(l1.funcCalled() && l2.funcCalled());
        l1.reset();
        l2.reset();

        s.disconnect(&l1);
        s.emit();
        TEST(!(l1.funcCalled()) && l2.funcCalled());
        l2.reset();

        {
            DisconnectingSelfListener disconnecter(s);
            s.connect(&disconnecter);
            s.emit();
            TEST(l2.funcCalled() && disconnecter.funcCalled());
            TEST(s.getNumListeners() == 1);
            l2.reset();
            disconnecter.reset();

            s.emit();
            TEST(l2.funcCalled() && !(disconnecter.funcCalled()));
            l2.reset();
        }
        {
            DisconnectingOtherListener disconnecter(s, &l2);
            s.connect(&disconnecter);
            s.emit();
            TEST(l2.funcCalled() && disconnecter.funcCalled());
            TEST(s.getNumListeners() == 1);
            l2.reset();
            disconnecter.reset();

            s.emit();
            TEST(!(l2.funcCalled()) && disconnecter.funcCalled());
            s.disconnect(&disconnecter);
        }
        {
            ConnectingOtherListener connecter(s, &l2);
            s.connect(&connecter);
            s.emit();
            TEST(l2.funcCalled() && connecter.funcCalled());
            TEST(s.getNumListeners() == 2);
            l2.reset();
            connecter.reset();

            s.emit();
            TEST(l2.funcCalled() && connecter.funcCalled());
        }
    }
};


class BacktraceTest: public Test
{
public:
    BacktraceTest()
        : Test("BacktraceTest", 2)
    {
    }

    void runTests()
    {
        vector<string> sFuncs;
        getBacktrace(sFuncs);
#ifndef _WIN32
        TEST(sFuncs[0].find("runTests") != string::npos);
#endif
    }
};


class PolygonTest: public Test
{
public:
    PolygonTest()
        : Test("PolygonTest", 2)
    {
    }

    void runTests()
    {
        glm::vec2 polyArray[] = {glm::vec2(30,0), glm::vec2(40,20), glm::vec2(60,30),
                glm::vec2(40,40), glm::vec2(30,60), glm::vec2(20,40), glm::vec2(0,30),
                glm::vec2(20,20)}; 

        Vec2Vector poly = vectorFromCArray(8, polyArray);
        vector<unsigned int> triangulation;
        triangulatePolygon(triangulation, poly);

        TEST(triangulation.size() == 6*3);
        unsigned int baselineIndexes[] = {6,7,5, 5,7,1, 7,0,1, 5,1,3, 3,1,2, 4,5,3};
        TEST(triangulation == vectorFromCArray(18, baselineIndexes));
/*     
        for (unsigned int i=0; i<triangulation.size(); i++) {
            cerr << i << ":" << triangulation[i] << endl;
        }/
*/
    }

};


class XmlParserTest: public Test
{
public:
    XmlParserTest()
      : Test("XmlParserTest", 2)
    {
    }

    void runTests()
    {
        string sXmlString = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
            "<shiporder orderid=\"889923\">"
            "  <orderperson>John Smith</orderperson>"
            "</shiporder>";

        {
            string sSchema = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>"
                "<xs:schema xmlns:xs=\"http://www.w3.org/2001/XMLSchema\">"
                "<xs:element name=\"shiporder\">"
                "  <xs:complexType>"
                "    <xs:sequence>"
                "      <xs:element name=\"orderperson\" type=\"xs:string\"/>"
                "    </xs:sequence>"
                "    <xs:attribute name=\"orderid\" type=\"xs:string\" use=\"required\"/>"
                "  </xs:complexType>"
                "</xs:element>"
                "</xs:schema>";

            XMLParser parser;
            parser.setSchema(sSchema, "shiporder.xsd");
            parser.parse(sXmlString, "shiporder.xml");
        }
        {
            string sDTD =
                "<!ELEMENT shiporder (orderperson)* >"
                "<!ATTLIST shiporder"
                "    orderid CDATA #IMPLIED>"
                "<!ELEMENT orderperson (#PCDATA) >";
            XMLParser parser;
            parser.setDTD(sDTD, "shiporder.dtd");
            parser.parse(sXmlString, "shiporder.xml");
        }
    }
};


class StandardLoggerTest: public Test
{
public:
    StandardLoggerTest()
      : Test("StandardLoggerTest", 2)
    {
    }

    void runTests()
    {
        std::stringstream buffer;
        std::streambuf *sbuf = std::cerr.rdbuf();
        Logger *logger = Logger::get();
        {
            std::cerr.rdbuf(buffer.rdbuf());
                string msg("Test log message");
                AVG_TRACE(Logger::category::NONE, Logger::severity::WARNING, msg);
            std::cerr.rdbuf(sbuf);
                TEST(buffer.str().find(msg) != string::npos);
            buffer.str(string());

            std::cerr.rdbuf(buffer.rdbuf());
                AVG_TRACE(Logger::category::NONE, Logger::severity::DEBUG, msg);
            std::cerr.rdbuf(sbuf);
            std::cout << buffer.str();
                TEST(buffer.str().find(msg) == string::npos);
            buffer.str(string());
        }
        {
            std::cerr.rdbuf(buffer.rdbuf());
                category_t CUSTOM_CAT = logger->configureCategory("CUSTOM_CAT 1");
                string msg("CUSTOM_CAT LOG");
                AVG_TRACE(CUSTOM_CAT, Logger::severity::WARNING, msg);
            std::cerr.rdbuf(sbuf);
                TEST(buffer.str().find(msg) != string::npos);
            buffer.str(string());
        }
        {
            std::cerr.rdbuf(buffer.rdbuf());
                category_t CUSTOM_CAT = logger->configureCategory("CUSTOM_CAT 1",
                        Logger::severity::CRITICAL);
                string msg_info("CUSTOM_CAT LOG INFO");
                AVG_TRACE(CUSTOM_CAT, Logger::severity::WARNING, msg_info);
            std::cerr.rdbuf(sbuf);
                TEST(buffer.str().find(msg_info) == string::npos);
            buffer.str(string());

            std::cerr.rdbuf(buffer.rdbuf());
                string msg_critical("CUSTOM_CAT LOG CRITICAL");
                AVG_TRACE(CUSTOM_CAT, Logger::severity::CRITICAL, msg_critical);
            std::cerr.rdbuf(sbuf);
                TEST(buffer.str().find(msg_critical) != string::npos);
            buffer.str(string());
        }
    }
};

class BaseTestSuite: public TestSuite
{
public:
    BaseTestSuite() 
        : TestSuite("BaseTestSuite")
    {
        addTest(TestPtr(new DAGTest));
        addTest(TestPtr(new QueueTest));
        addTest(TestPtr(new WorkerThreadTest));
        addTest(TestPtr(new ObjectCounterTest));
        addTest(TestPtr(new GeomTest));
        addTest(TestPtr(new TriangleTest));
        addTest(TestPtr(new FileTest));
        addTest(TestPtr(new OSTest));
        addTest(TestPtr(new StringTest));
        addTest(TestPtr(new SplineTest));
        addTest(TestPtr(new BezierCurveTest));
        addTest(TestPtr(new SignalTest));
        addTest(TestPtr(new BacktraceTest));
        addTest(TestPtr(new PolygonTest));
        addTest(TestPtr(new XmlParserTest));
        addTest(TestPtr(new StandardLoggerTest));
    }
};


int main(int nargs, char** args)
{
    BaseTestSuite suite;
    suite.runTests();
    bool bOK = suite.isOk();

    if (bOK) {
        return 0;
    } else {
        return 1;
    }
}

