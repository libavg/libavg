//
//  libavg - Media Playback Engine. 
//  Copyright (C) 2003-2008 Ulrich von Zadow
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

#include "BicubicSpline.h"

#include <iostream>

using namespace std;

namespace avg {

BicubicSpline::BicubicSpline(const vector<double>& x, const vector<double>& y,
            const vector<vector<double> >& f)
    : m_X(x),
      m_Y(y),
      m_F(f)
{
    assert(y.size() == f.size());
    for (unsigned i=0; i<y.size(); ++i) {
        assert(x.size() == f[i].size());
    }
/*
    for (int i=-2; i<=int(m_Y.size()+1); ++i) {
        for (int j=-2; j<=int(m_X.size()+1); ++j) {
            cerr << getF(i, j) << ", ";
        }
        cerr << endl;
    }
*/
    // Add extra support points at the borders.
    vector<vector<double> > tempF = f;
    tempF.insert(tempF.begin(), vector<double>());
    tempF.push_back(vector<double>());
    for (unsigned j=0; j<m_X.size(); ++j) {
        tempF[0].push_back(getF(-100,j));
        tempF[tempF.size()-1].push_back(getF(m_F.size()+99,j));
    }
    for (unsigned i=0; i<m_Y.size()+2; ++i) {
        tempF[i].insert(tempF[i].begin(), getF(i-1, -100));
        tempF[i].push_back(getF(i-1, m_F.size()+99));
    }
    m_F = tempF;
    
    double edgeX = m_X[0]-100*(m_X[1]-m_X[0]);
    double edgeY = m_Y[0]-100*(m_Y[1]-m_Y[0]);
    m_X.insert(m_X.begin(), edgeX);
    m_Y.insert(m_Y.begin(), edgeY);
    int len = m_X.size();
    edgeX = m_X[len-1]+100*(m_X[len-1]-m_X[len-2]);
    len = m_Y.size();
    edgeY = m_Y[len-1]+100*(m_Y[len-1]-m_Y[len-2]);
    m_X.push_back(edgeX);
    m_Y.push_back(edgeY);
/*
    cerr << "----" << endl;
    for (int i=0; i<int(m_Y.size()); ++i) {
        for (int j=0; j<int(m_X.size()); ++j) {
            cerr << m_F[i][j] << ", ";
        }
        cerr << endl;
    }
*/

    // Calculate needed derivatives.
    for (int i=0; i<int(m_Y.size()); ++i) {
        m_Fdx.push_back(vector<double>(m_X.size(),0.0));
        m_Fdy.push_back(vector<double>(m_X.size(),0.0));
        m_Fdxy.push_back(vector<double>(m_X.size(),0.0));
        for (int j=0; j<int(m_X.size()); ++j) {
            m_Fdy[i][j] = (getF(i+1,j)-getF(i-1,j))/(getY(j+1)-getY(j-1));
            m_Fdx[i][j] = (getF(i,j+1)-getF(i,j-1))/(getX(i+1)-getX(i-1));
            m_Fdxy[i][j] = (getF(i+1,j+1)-getF(i+1,j-1)-getF(i-1,j+1)+getF(i-1, j-1))/
                    ((getX(i+1)-getX(i-1))*(getY(j+1)-getY(j-1)));
        }
    }

}

BicubicSpline::~BicubicSpline()
{
}

// Adapted from Numerical Recipies in C
double BicubicSpline::interpolate(const DPoint& orig)
{
    int j=0;
    if (m_X[m_X.size()-1] <= orig.x) {
        j = m_X.size();
    } else {
        while (m_X[j] < orig.x) {
            j++;
        }
    }
    assert (j>0 && j<int(m_X.size()));
    int i=0;
    if (m_Y[m_Y.size()-1] <= orig.y) {
        i = m_Y.size();
    } else {
        while (m_Y[i] < orig.y) {
            i++;
        }
    }
    assert (i>0 && i<int(m_Y.size()));

    vector<vector<double> > coeffs; 
    getCoeffs(i, j, coeffs);
//    cerr << "i: " << i << ", j: " << j << endl;
    double d1 = m_X[j]-m_X[j-1];
    double d2 = m_Y[i]-m_Y[i-1];
    double t=(orig.x-m_X[j-1])/d1;
    double u=(orig.y-m_Y[i-1])/d2;
/*
    cerr << "--  F --" << endl;
    cerr << "(" << m_F[i-1][j-1] << "), (" << m_F[i-1][j] << ")" << endl;
    cerr << "(" << m_F[i][j-1] << "), (" << m_F[i][j] << ")" << endl;
    cerr << "--  Fdx --" << endl;
    cerr << "(" << m_Fdx[i-1][j-1] << "), (" << m_Fdx[i-1][j] << ")" << endl;
    cerr << "(" << m_Fdx[i][j-1] << "), (" << m_Fdx[i][j] << ")" << endl;
    cerr << "--  Fdy --" << endl;
    cerr << "(" << m_Fdy[i-1][j-1] << "), (" << m_Fdy[i-1][j] << ")" << endl;
    cerr << "(" << m_Fdy[i][j-1] << "), (" << m_Fdy[i][j] << ")" << endl;
    cerr << "--  Fdxy --" << endl;
    cerr << "(" << m_Fdxy[i-1][j-1] << "), (" << m_Fdxy[i-1][j] << ")" << endl;
    cerr << "(" << m_Fdxy[i][j-1] << "), (" << m_Fdxy[i][j] << ")" << endl;
    cerr << "d1: " << d1 << ", d2: " << d2 << ", t: " << t << ", u: " << u << endl;
*/
    double result=0.0; 
    for (i=3; i>=0; i--) {
        result=t*result + ((coeffs[i][3]*u + coeffs[i][2])*u + coeffs[i][1])*u + coeffs[i][0]; 
    } 
    return result;
}
    
double BicubicSpline::getX(int j)
{
    if (j==-1) {
       return 2*m_X[0]-m_X[1];
    } else if (j==int(m_X.size())) {
       return 2*m_X[j-1]-m_X[j-2];
    } else {
        return m_X[j];
    }
}

double BicubicSpline::getY(int i)
{
    if (i==-1) {
       return 2*m_Y[0]-m_Y[1];
    } else if (i==int(m_Y.size())) {
       return 2*m_Y[i-1]-m_Y[i-2];
    } else {
        return m_Y[i];
    }
}

double BicubicSpline::getF(int i, int j)
{
    if (i <= -1) {
       double pt0 = getF(0, j);
       double df = getF(1, j)-pt0;
       return pt0+df*i;
    } else if (i >= int(m_Y.size())) {
       double pt0 = getF(m_Y.size()-1, j);
       double df = getF(m_Y.size()-2, j)-pt0;
       return pt0+df*(i-m_Y.size()+1);
    } else if (j <= -1) {
       double pt0 = getF(i, 0);
       double df = getF(i, 1)-pt0;
       return pt0+df*j;
    } else if (j >= int(m_X.size())) {
       double pt0 = getF(i, m_X.size()-1);
       double df = getF(i, m_X.size()-2)-pt0;
       return pt0+df*(j-m_X.size()+1);
    } else {
        return m_F[i][j];
    }
}

// Adapted from Numerical Recipies in C
void BicubicSpline::getCoeffs(int i, int j, vector<vector<double> > & coeffs)
{
    static int wt[16][16] = 
       {{1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0}, 
        {0,0,0,0,0,0,0,0,1,0,0,0,0,0,0,0}, 
        {-3,0,0,3,0,0,0,0,-2,0,0,-1,0,0,0,0}, 
        {2,0,0,-2,0,0,0,0,1,0,0,1,0,0,0,0}, 
        {0,0,0,0,1,0,0,0,0,0,0,0,0,0,0,0}, 
        {0,0,0,0,0,0,0,0,0,0,0,0,1,0,0,0}, 
        {0,0,0,0,-3,0,0,3,0,0,0,0,-2,0,0,-1}, 
        {0,0,0,0,2,0,0,-2,0,0,0,0,1,0,0,1}, 
        {-3,3,0,0,-2,-1,0,0,0,0,0,0,0,0,0,0}, 
        {0,0,0,0,0,0,0,0,-3,3,0,0,-2,-1,0,0}, 
        {9,-9,9,-9,6,3,-3,-6,6,-6,-3,3,4,2,1,2}, 
        {-6,6,-6,6,-4,-2,2,4,-3,3,3,-3,-2,-1,-1,-2}, 
        {2,-2,0,0,1,1,0,0,0,0,0,0,0,0,0,0}, 
        {0,0,0,0,0,0,0,0,2,-2,0,0,1,1,0,0}, 
        {-6,6,-6,6,-3,-3,3,3,-4,4,2,-2,-2,-2,-1,-1}, 
        {4,-4,4,-4,2,2,-2,-2,2,-2,-2,2,1,1,1,1}}; 

    double d1 = m_X[j]-m_X[j-1];
    double d2 = m_Y[i]-m_Y[i-1];

    double d1d2=d1*d2; 
    double x[16];

    // Pack temporary vector with the input variables.
    x[0] = m_F[i-1][j-1];
    x[1] = m_F[i-1][j];
    x[2] = m_F[i][j];
    x[3] = m_F[i][j-1];

    x[4] = m_Fdx[i-1][j-1]*d1;
    x[5] = m_Fdx[i-1][j]*d1;
    x[6] = m_Fdx[i][j]*d1;
    x[7] = m_Fdx[i][j-1]*d1;

    x[8] = m_Fdy[i-1][j-1]*d2;
    x[9] = m_Fdy[i-1][j]*d2;
    x[10] = m_Fdy[i][j]*d2;
    x[11] = m_Fdy[i][j-1]*d2;

    x[12] = m_Fdxy[i-1][j-1]*d1d2;
    x[13] = m_Fdxy[i-1][j]*d1d2;
    x[14] = m_Fdxy[i][j]*d1d2;
    x[15] = m_Fdxy[i][j-1]*d1d2;

    // Matrix multiply by the stored table, solving the set of linear equations.
    // XXX: This could be made much faster by multiplying the values inline
    double cl[16];
    for (int i=0;i<=15;i++) {
        double xx=0.0; 
        for (int k=0;k<=15;k++) { 
            xx += wt[i][k]*x[k];
        }
        cl[i]=xx; 
    }

    // Unpack the result into the result 4x4 table.
    int l=0;
    coeffs = vector<vector<double> >(4, vector<double>());
    for (int i=0;i<=3;i++) {
        for (int j=0;j<=3;j++) {
            coeffs[i].push_back(cl[l++]);
        }
    }
}

}
