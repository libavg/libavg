// Fast and accurate bitmap scaling. Original code by Eran Yariv and Jake Montgomery,
// posted on codeguru.com.

#ifndef _TwoPassScale_h_
#define _TwoPassScale_h_

#include "ContribDefs.h"

#include "../base/Exception.h"

#include <math.h>
#include <algorithm>
#include <cstring>

namespace avg {

typedef struct
{
   int *Weights;     // Normalized weights of neighboring pixels
   int Left,Right;   // Bounds of source pixels window
} ContributionType;  // Contirbution information for a single pixel

typedef struct
{
   ContributionType *ContribRow; // Row (or column) of contribution weights
   int WindowSize,               // Filter window size (of affecting source pixels)
       LineLength;               // Length of line (no. or rows / cols)
} LineContribType;               // Contribution information for an entire line (row or column)

class CDataA_UBYTE
{
public:
  typedef unsigned char PixelClass;
  class _Accumulator {
  public:
      _Accumulator ()
      {
        val = 0;
      };
      void Accumulate (int Weight, PixelClass &value)
      {
        val += (Weight * value);
      };

      void Store (PixelClass* value)
      {
        *value = (unsigned char) ((val + 128)/256);
      };
      int val;
  };
};

class CDataRGB_UBYTE
{
public:
  typedef unsigned char PixelClass[3];
  class _Accumulator {
  public:
      _Accumulator ()
      {
        val [0] = val [1] = val [2] = 0;
      };
      void Accumulate (int Weight, PixelClass &value)
      {
        val [0] += (Weight * value [0]);
        val [1] += (Weight * value [1]);
        val [2] += (Weight * value [2]);
      };

      void Store (PixelClass* value)
      {
        (*value) [0] = (unsigned char) ((val [0] + 128)/256);
        (*value) [1] = (unsigned char) ((val [1] + 128)/256);
        (*value) [2] = (unsigned char) ((val [2] + 128)/256);
      };
      int val [3];
  };
};

class CDataRGBA_UBYTE {
public:
  typedef unsigned char PixelClass[4];
  class _Accumulator {
  public:
      _Accumulator ()
      {
        val [0] = val [1] = val [2] = val [3] = 0;
      };
      void Accumulate (int dWeight, PixelClass &value)
      {
        val [0] += (dWeight * (value [0]));
        val [1] += (dWeight * (value [1]));
        val [2] += (dWeight * (value [2]));
        val [3] += (dWeight * (value [3]));
      };

      void Store (PixelClass* value)
      {
        (*value) [0] = (unsigned char) ((val [0] + 128)/256);
        (*value) [1] = (unsigned char) ((val [1] + 128)/256);
        (*value) [2] = (unsigned char) ((val [2] + 128)/256);
        (*value) [3] = (unsigned char) ((val [3] + 128)/256);
      };
      int val [4];
  };
};

template <class DataClass>
class TwoPassScale
{
public:
    typedef typename DataClass::PixelClass PixelClass;

    TwoPassScale (const ContribDef& contribDef)
        : m_ContribDef (contribDef)
    {};

    virtual ~TwoPassScale() {};

    void Scale(PixelClass * pSrcData, const IntPoint& srcSize, int srcStride, 
            PixelClass *pDstData, const IntPoint& dstSize, int dstStride);

private:
    LineContribType *AllocContributions (unsigned uLineLength,
                                         unsigned uWindowSize);

    void FreeContributions (LineContribType * p);

    LineContribType *CalcContributions (unsigned    uLineSize,
                                        unsigned    uSrcSize);

    void ScaleRow(PixelClass *pSrc, int uSrcWidth, PixelClass *pDest, int uResWidth, 
            LineContribType *pContrib);

    void HorizScale(PixelClass * pSrcData, const IntPoint& srcSize, int srcStride, 
            PixelClass *pDestData, const IntPoint& destSize, int destStride);

    void VertScale(PixelClass *pSrcData, const IntPoint& srcSize, int srcStride,
            PixelClass *pDestData, const IntPoint& destSize, int destStride);

    const ContribDef& m_ContribDef;
};

template <class DataClass>
LineContribType *
TwoPassScale<DataClass>::AllocContributions (unsigned uLineLength, unsigned uWindowSize)
{
    LineContribType *res = new LineContribType;
        // Init structure header
    res->WindowSize = uWindowSize;
    res->LineLength = uLineLength;
        // Allocate list of contributions
    res->ContribRow = new ContributionType[uLineLength];
    for (unsigned u = 0 ; u < uLineLength ; u++)
    {
        // Allocate contributions for every pixel
        res->ContribRow[u].Weights = new int[uWindowSize];
    }
    return res;
}

template <class DataClass>
void
TwoPassScale<DataClass>::FreeContributions (LineContribType * p)
{
    for (int u = 0; u < p->LineLength; u++)
    {
        // Free contribs for every pixel
        delete [] p->ContribRow[u].Weights;
    }
    delete [] p->ContribRow;    // Free list of pixels contribs
    delete p;                   // Free contribs header
}

template <class DataClass>
LineContribType *
TwoPassScale<DataClass>::CalcContributions(unsigned uLineSize, unsigned uSrcSize)
{
    float dScale = float(uLineSize)/uSrcSize;
    float dWidth;
    float dFScale = 1.0;
    float dFilterWidth = m_ContribDef.GetWidth();

    if (dScale < 1.0) {
        // Minification
        dWidth = dFilterWidth / dScale;
        dFScale = dScale;
    } else {
        // Magnification
        dWidth= dFilterWidth;
    }

    // Window size is the number of sampled pixels
    int iWindowSize = 2 * (int)ceil(dWidth) + 1;

    // Allocate a new line contributions strucutre
    LineContribType *res = AllocContributions (uLineSize, iWindowSize);

    for (unsigned u = 0; u < uLineSize; u++) {
        // Scan through line of contributions
        float dCenter = (u+0.5f)/dScale-0.5f;   // Reverse mapping
        // Find the significant edge points that affect the pixel
        int iLeft = std::max (0, (int)floor (dCenter - dWidth));
        int iRight = std::min ((int)ceil (dCenter + dWidth), int(uSrcSize) - 1);

        // Cut edge points to fit in filter window in case of spill-off
        if (iRight - iLeft + 1 > iWindowSize) {
            if (iLeft < (int(uSrcSize) - 1 / 2)) {
                iLeft++;
            } else {
                iRight--;
            }
        }
        res->ContribRow[u].Left = iLeft;
        res->ContribRow[u].Right = iRight;
        
        int dTotalWeight = 0;  // Zero sum of weights
        for (int iSrc = iLeft; iSrc <= iRight; iSrc++) {
            // Calculate weights
            int CurWeight = int (dFScale * (m_ContribDef.Filter (dFScale * 
                    (dCenter - (float)iSrc)))*256);
            res->ContribRow[u].Weights[iSrc-iLeft] = CurWeight;
            dTotalWeight += CurWeight;
        }
        AVG_ASSERT(dTotalWeight >= 0);   // An error in the filter function can cause this
        int UsedWeight = 0;
        if (dTotalWeight > 0) {
            // Normalize weight of neighbouring points
            for (int iSrc = iLeft; iSrc < iRight; iSrc++) {
                // Normalize point
                int CurWeight = (res->ContribRow[u].Weights[iSrc-iLeft]*256)/dTotalWeight;
                res->ContribRow[u].Weights[iSrc-iLeft] = CurWeight;
                UsedWeight += CurWeight;
            }
            // The last point gets everything that's left over so the sum is
            // always correct.
            res->ContribRow[u].Weights[iRight-iLeft] = 256 - UsedWeight;
        }
   }
   return res;
}

template <class DataClass>
void
TwoPassScale<DataClass>::ScaleRow(PixelClass *pSrc, int uSrcWidth,
        PixelClass *pDest, int uResWidth, LineContribType *pContrib)
{
    PixelClass * pDestPixel = pDest;
    for (int x = 0; x < uResWidth; x++) {
        typename DataClass::_Accumulator a;
        int iLeft = pContrib->ContribRow[x].Left;    // Retrieve left boundries
        int iRight = pContrib->ContribRow[x].Right;  // Retrieve right boundries
        int * Weights = pContrib->ContribRow[x].Weights;
        for (int i = iLeft; i <= iRight; i++) {
            // Scan between boundries
            // Accumulate weighted effect of each neighboring pixel
            a.Accumulate(Weights[i-iLeft], pSrc[i]);
        }
        a.Store(pDestPixel);
        pDestPixel++;
    }
}

template <class DataClass>
void TwoPassScale<DataClass>::HorizScale(PixelClass * pSrcData, const IntPoint& srcSize, 
        int srcStride, PixelClass *pDestData, const IntPoint& destSize, int destStride)
{
    PixelClass * pSrc = pSrcData;
    PixelClass * pDest = pDestData;
    if (srcSize.x == destSize.x) {
        // No scaling required, just copy
        for (int y = 0; y < destSize.y; y++) {
            memcpy(pDest, pSrc, sizeof (PixelClass) * srcSize.x);
            pSrc = (PixelClass*)((char*)(pSrc)+srcStride);
            pDest = (PixelClass*)((char*)(pDest)+destStride);
        }
    } else {
        LineContribType * pContrib = CalcContributions(destSize.x, srcSize.x);
        for (int y = 0; y < destSize.y; y++) {
            ScaleRow(pSrc, srcSize.x, pDest, destSize.x, pContrib);
            pSrc = (PixelClass*)((char*)(pSrc)+srcStride);
            pDest = (PixelClass*)((char*)(pDest)+destStride);
        }
        FreeContributions(pContrib);  // Free contributions structure
    }
}


template <class DataClass>
void TwoPassScale<DataClass>::VertScale(PixelClass *pSrcData, const IntPoint& srcSize,
        int srcStride, PixelClass *pDestData, const IntPoint& destSize, int destStride)
{
    PixelClass * pSrc = pSrcData;
    PixelClass * pDest = pDestData;
    if (srcSize.y == destSize.y) {
        // No scaling required, just copy
        for (int y = 0; y < destSize.y; y++) {
            memcpy(pDest, pSrc, sizeof (PixelClass) * srcSize.x);
            pSrc = (PixelClass*)((char*)(pSrc)+srcStride);
            pDest = (PixelClass*)((char*)(pDest)+destStride);
        }
    } else {
        LineContribType * pContrib = CalcContributions(destSize.y, srcSize.y);
        for (int y = 0; y < destSize.y; y++) {
            PixelClass * pDestPixel = pDest;
            int * pWeights = pContrib->ContribRow[y].Weights;
            int iLeft = pContrib->ContribRow[y].Left;
            int iRight = pContrib->ContribRow[y].Right;
            PixelClass* pSrcPixelBase = (PixelClass*)((char*)(pSrc)
                    + size_t(iLeft)*srcStride);
            for (int x = 0; x < destSize.x; x++) {
                typename DataClass::_Accumulator a;
                int * pWeight = pWeights;
                PixelClass * pSrcPixel = pSrcPixelBase;
                pSrcPixelBase++;
                for (int i = iLeft; i <= iRight; i++) {
                    // Scan between boundries
                    // Accumulate weighted effect of each neighboring pixel
                    a.Accumulate(*pWeight, *pSrcPixel);
                    pWeight++;
                    pSrcPixel = (PixelClass*)((char*)(pSrcPixel)+srcStride);
                }
                a.Store(pDestPixel);
                pDestPixel++;
            }
            pDest = (PixelClass*)((char*)(pDest)+destStride);
        }
        FreeContributions(pContrib);     // Free contributions structure
    }
}


template <class DataClass>
void TwoPassScale<DataClass>::Scale(PixelClass * pSrcData, const IntPoint& srcSize, 
        int srcStride, PixelClass *pDstData, const IntPoint& dstSize, int dstStride)
{
    // Allocate temp image
    PixelClass *pTempData = new PixelClass[srcSize.y*dstSize.x];
    IntPoint tempSize(dstSize.x, srcSize.y);
    int tempStride = dstSize.x*sizeof(PixelClass);

    // Scale source image horizontally into temporary image
    HorizScale(pSrcData, srcSize, srcStride, 
            pTempData, tempSize, tempStride);
 
    // Scale temporary image vertically into result image
    VertScale (pTempData, tempSize, tempStride, 
            pDstData, dstSize, dstStride);
    delete [] pTempData;
}

}

#endif

