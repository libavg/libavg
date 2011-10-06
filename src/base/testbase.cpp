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

#include "Queue.h"
#include "Command.h"
#include "WorkerThread.h"
#include "ObjectCounter.h"
#include "Point.h"
#include "Matrix3x4.h"
#include "Triangulate.h"
#include "GeomHelper.h"
#include "OSHelper.h"
#include "FileHelper.h"
#include "StringHelper.h"
#include "MathHelper.h"
#include "CubicSpline.h"
#include "BicubicSpline.h"
#include "BezierCurve.h"
#include "Signal.h"
#include "Backtrace.h"

#include "TestSuite.h"
#include "TimeSource.h"

#include <boost/thread/thread.hpp>

#include <boost/bind.hpp>

#include <iostream>
#include <sstream>
#include <stdio.h>
#include <stdlib.h>

using namespace avg;
using namespace std;
using namespace boost;

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
            Queue<string> q(10);
            thread pusher(boost::bind(&pushThread, &q, 100));
            thread popper(boost::bind(&popThread, &q, 100));
            pusher.join();
            popper.join();
            TEST(q.empty());
        }
        {
            Queue<string> q(10);
            thread pusher1(boost::bind(&pushThread, &q, 100));
            thread pusher2(boost::bind(&pushThread, &q, 100));
            thread popper(boost::bind(&popThread, &q, 200));
            pusher1.join();
            pusher2.join();
            popper.join();
            TEST(q.empty());
        }
    }

    static void pushThread(Queue<string>* pq, int numPushes)
    {
        typedef Queue<string>::QElementPtr ElemPtr;
        for (int i=0; i<numPushes; ++i) {
            stringstream ss;
            ss << i;
            string s = ss.str();
            pq->push(ElemPtr(new string(s)));
            msleep(1);
        }
    }

    static void popThread(Queue<string>* pq, int numPops)
    {
        for (int i=0; i<numPops; ++i) {
            pq->peek();
            pq->pop();
            msleep(3);
        }
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
class PointTest: public Test
{
public:
    PointTest()
        : Test("PointTest", 2)
    {
    }

    void runTests() 
    {
        double one = 1;
        double zero = 0;

        TEST(isinf(-one/zero) != 0);
        TEST(isinf(one/zero) != 0);
        TEST(isinf(one) == 0);
        TEST(isnan(sqrt(-one)) != 0);
        TEST(isnan(sqrt(one+one)) == 0);

        // TODO: Move to a separate math test once we're done here.
        TEST(almostEqual(invSqrt(1), 1));
        TEST(almostEqual(invSqrt(4), 0.5));

        // TODO: The point tests aren't complete!
        DPoint pt1(0,0);
        DPoint pt2(3,4);
        TEST(calcDist(pt1, pt2)-5 < 0.0001);
        TEST(!almostEqual(pt1, pt2));
        TEST(almostEqual(pt1, pt1));
        std::vector<double> v;
        v.push_back(3);
        v.push_back(4);
        DPoint pt3(v);
        TEST(almostEqual(pt2, pt3));
        TEST(almostEqual(pt3.getNorm(), 5));
        DPoint pt4 = pt3.getNormalized();
        TEST(almostEqual(pt4.getNorm(), 1, 0.0001));
        {
            DLineSegment l1(DPoint(0,0), DPoint(2,2));
            DLineSegment l2(DPoint(2,0), DPoint(0,2));
            TEST(lineSegmentsIntersect(l1, l2));
            TEST(lineSegmentsIntersect(l2, l1));
        }
        {
            DLineSegment l1(DPoint(0,0), DPoint(0,2));
            DLineSegment l2(DPoint(2,0), DPoint(2,2));
            TEST(!lineSegmentsIntersect(l1, l2));
        }
        {
            DLineSegment l1(DPoint(0,0), DPoint(2,0));
            DLineSegment l2(DPoint(0,2), DPoint(2,2));
            TEST(!lineSegmentsIntersect(l1, l2));
        }
        {
            DLineSegment l1(DPoint(0,0), DPoint(2,0));
            TEST(l1.isPointOver(DPoint(1,23)));
            TEST(l1.isPointOver(DPoint(1.9,-5)));
            TEST(!l1.isPointOver(DPoint(-1,1)));
            TEST(!l1.isPointOver(DPoint(3,-1)));
        }
        {
            DPoint pt0(DPoint(1,1));
            DPoint pt1(DPoint(1,3));
            DPoint pt2(DPoint(1,-2));
            vector<DPoint> poly;
            poly.push_back(DPoint(0,0));
            poly.push_back(DPoint(2,0));
            poly.push_back(DPoint(2,2));
            poly.push_back(DPoint(0,2));
            TEST(pointInPolygon(pt0, poly));
            TEST(!pointInPolygon(pt1, poly));
            TEST(!pointInPolygon(pt2, poly));
            poly.push_back(DPoint(2,1));
            TEST(!pointInPolygon(pt0, poly));
        }
        {
            DPoint p1(DPoint(0,0));
            DPoint v1(DPoint(1,1));
            DPoint p2(DPoint(2,1));
            DPoint v2(DPoint(1,0));
            TEST(getLineLineIntersection(p1, v1, p2, v2) == DPoint(1,1));
        }
        TEST(almostEqual(DPoint(10,0).getRotatedPivot(PI, DPoint(15,5)), DPoint(20,10)));
        TEST(almostEqual(DPoint(10,0).getRotatedPivot(PI*0.5, DPoint(15,5)),
                DPoint(20,0)));
        TEST(almostEqual(DPoint(10,0).getRotatedPivot(PI*1.5, DPoint(15,5)), 
                DPoint(10,10)));
        TEST(almostEqual(DPoint(10,0).getRotatedPivot(PI*2, DPoint(15,5)), 
                DPoint(10,0)));
        TEST(almostEqual(DPoint(23,0).getRotatedPivot(PI*0.5), DPoint(0,23)));

        TEST(almostEqual(DPoint(10,0), DPoint::fromPolar(0, 10)));
        TEST(almostEqual(DPoint(0,10), DPoint::fromPolar(PI*0.5, 10)));
        TEST(almostEqual(DPoint(0,-1), DPoint::fromPolar(PI*1.5, 1)));
        TEST(almostEqual(vecAngle(DPoint(0,1),DPoint(1,0)), PI*0.5));
        TEST(almostEqual(vecAngle(DPoint(0,-1),DPoint(1,0)), PI*1.5));
        TEST(almostEqual(vecAngle(DPoint(0,2),DPoint(1,0)), PI*0.5));
    }
};
#ifdef _MSC_VER
#pragma warning(pop)
#pragma optimize("", on)
#endif


class Matrix3x4Test: public Test
{
public:
    Matrix3x4Test()
        : Test("Matrix3x4Test", 2)
    {
    }

    void runTests()
    {
        Matrix3x4 mat1;
        Matrix3x4 mat2;
        mat1 *= mat2;
        TEST(almostEqual(mat1, Matrix3x4()));

        mat2 = Matrix3x4::createTranslate(0,0,0);
        mat1 *= mat2;
        TEST(almostEqual(mat1, Matrix3x4()));

        mat2 = Matrix3x4::createScale(1,1,1);
        mat1 *= mat2;
        TEST(almostEqual(mat1, Matrix3x4()));
    }
};


class TriangleTest: public Test
{
public:
    TriangleTest()
        : Test("TriangleTest", 2)
    {
    }

    void runTests()
    {
        Triangle tri(DPoint(0,0), DPoint(4,4), DPoint(4,8));
        TEST(tri.isInside(DPoint(3,4)));
        TEST(!tri.isInside(DPoint(1,4)));
        TEST(!tri.isInside(DPoint(2,1)));
        TEST(!tri.isInside(DPoint(-2,5)));
        TEST(!tri.isInside(DPoint(5,5)));
        tri = Triangle(DPoint(0,0), DPoint(4,8), DPoint(4,4));
        TEST(tri.isInside(DPoint(3,4)));

        DPoint polyArray[] = {DPoint(0,0), DPoint(8,2), DPoint(9,0), DPoint(9,3), 
                DPoint(1,1), DPoint(0,3)}; 

        DPointVector poly = vectorFromCArray(6, polyArray);

        vector<int> triangulation;
        triangulatePolygon(poly, triangulation);

        TEST(triangulation.size() == 4*3);
        int baselineIndexes[] = {1,2,3, 4,5,0, 0,1,3, 3,4,0};
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
#if defined(__APPLE__) || defined(_WIN32)
        TEST(getAvgLibPath() != "");
#endif
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
        TEST(almostEqual(stringToDouble("5.5"), 5.5));
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
        TEST(stringToDPoint(" ( 3.4 , 2.1 ) ") == DPoint(3.4, 2.1));
        vector<double> v;
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
            double xd[] = {0,1,2,3};
            vector<double> x = vectorFromCArray(4, xd);
            double yd[] = {3,2,1,0};
            vector<double> y = vectorFromCArray(4, yd);
            CubicSpline spline(x, y);
            TEST(almostEqual(spline.interpolate(-1), 4));
            TEST(almostEqual(spline.interpolate(0), 3));
            TEST(almostEqual(spline.interpolate(0.5), 2.5));
            TEST(almostEqual(spline.interpolate(3), 0));
            TEST(almostEqual(spline.interpolate(3.5), -0.5));
        }
        {
            double xd[] = {2,4,6,8};
            vector<double> x = vectorFromCArray(4, xd);
            double yd[] = {0,1,3,6};
            vector<double> y = vectorFromCArray(4, yd);
            CubicSpline spline(x, y);
            TEST(almostEqual(spline.interpolate(0), -1));
            TEST(almostEqual(spline.interpolate(1), -0.5));
            TEST(almostEqual(spline.interpolate(2), 0));
            TEST(spline.interpolate(3) < 0.5);
            TEST(almostEqual(spline.interpolate(8), 6));
            TEST(almostEqual(spline.interpolate(9), 7.5));
            TEST(almostEqual(spline.interpolate(10), 9));
        }
        {
            double xd[] = {0,1,2,3};
            vector<double> x = vectorFromCArray(4, xd);
            double yd[] = {0,1,2,3}; 
            vector<double> y = vectorFromCArray(4, yd);
            double fd[] = {0,0,0,0,
                           0,0,1,0,
                           0,0,0,0,
                           0,0,0,0
                          };
            vector<vector<double> > f = vector2DFromCArray(4, 4, fd);
            BicubicSpline spline(x, y, f);
/*
            cerr << spline.interpolate(DPoint(1,1)) << endl;
            cerr << spline.interpolate(DPoint(2,1)) << endl;
            cerr << spline.interpolate(DPoint(1.5,1)) << endl;
            cerr << spline.interpolate(DPoint(0.5,1)) << endl;
            cerr << spline.interpolate(DPoint(1,2)) << endl;
            cerr << spline.interpolate(DPoint(10,0)) << endl;
*/
            TEST(almostEqual(spline.interpolate(DPoint(0,0)), 0));
            TEST(almostEqual(spline.interpolate(DPoint(1,1)), 0));
            TEST(almostEqual(spline.interpolate(DPoint(2,1)), 1));
            TEST(almostEqual(spline.interpolate(DPoint(1,2)), 0));
            TEST(spline.interpolate(DPoint(1.5,1)) > 0.5);
            TEST(spline.interpolate(DPoint(1.5,1)) < 1);
            TEST(spline.interpolate(DPoint(0.5,1)) < 0);
            TEST(almostEqual(spline.interpolate(DPoint(-1,0)), 0));
            TEST(almostEqual(spline.interpolate(DPoint(-10,0)), 0));
            TEST(almostEqual(spline.interpolate(DPoint(0,-1)), 0));
            TEST(almostEqual(spline.interpolate(DPoint(0,-10)), 0));
            TEST(almostEqual(spline.interpolate(DPoint(0,10)), 0));
            TEST(almostEqual(spline.interpolate(DPoint(10,0)), 0));
        }
        {
            double xd[] = {0,2,4,6};
            vector<double> x = vectorFromCArray(4, xd);
            double yd[] = {0,3,6,9}; 
            vector<double> y = vectorFromCArray(4, yd);
            double fd[] = {0,0,0,0,
                           0,0,1,0,
                           0,0,0,0,
                           0,0,0,0
                          };
            vector<vector<double> > f = vector2DFromCArray(4, 4, fd);
            BicubicSpline spline(x, y, f);
            TEST(almostEqual(spline.interpolate(DPoint(2,3)), 0));
            TEST(almostEqual(spline.interpolate(DPoint(4,3)), 1));
            TEST(almostEqual(spline.interpolate(DPoint(2,6)), 0));
            TEST(spline.interpolate(DPoint(3,3)) > 0.5);
            TEST(spline.interpolate(DPoint(3,3)) < 1);
            TEST(spline.interpolate(DPoint(1,3)) < 0);
        }
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
        BezierCurve curve(DPoint(0,0), DPoint(1,0), DPoint(1,1), DPoint(0,1));
        TEST(almostEqual(curve.interpolate(0), DPoint(0,0)));
        TEST(almostEqual(curve.getDeriv(0), DPoint(3, 0)));
        TEST(almostEqual(curve.interpolate(1), DPoint(0,1)));
        TEST(almostEqual(curve.getDeriv(1), DPoint(-3, 0)));
        TEST(almostEqual(curve.interpolate(0.5), DPoint(0.75,0.5)));
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


class BaseTestSuite: public TestSuite
{
public:
    BaseTestSuite() 
        : TestSuite("BaseTestSuite")
    {
        addTest(TestPtr(new QueueTest));
        addTest(TestPtr(new WorkerThreadTest));
        addTest(TestPtr(new ObjectCounterTest));
        addTest(TestPtr(new PointTest));
        addTest(TestPtr(new Matrix3x4Test));
        addTest(TestPtr(new TriangleTest));
        addTest(TestPtr(new FileTest));
        addTest(TestPtr(new OSTest));
        addTest(TestPtr(new StringTest));
        addTest(TestPtr(new SplineTest));
        addTest(TestPtr(new BezierCurveTest));
        addTest(TestPtr(new SignalTest));
        addTest(TestPtr(new BacktraceTest));
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

