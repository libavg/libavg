#include "FilterDistortion.h"
#include "../graphics/Rect.h"
#include "../graphics/Point.h"
#include <boost/multi_array.hpp>
#include <math.h>

//Really ugly code. do not use...


namespace avg{
    FilterDistortion::FilterDistortion(IntPoint srcSize, double K1, double T):
        m_srcRect(0,0,srcSize.x,srcSize.y),
        m_trafo(m_srcRect, -K1, -T) //inverse of Distortion by reversing the sign.
    {
        //we use the same dimensions for both of src and dest and just crop...
        //for each pixel at (x,y) in the dest
        //m_pMap[x][y] contains a IntPoint that gives the coords in the src Bitmap 
        m_pMap = new IntPoint[m_srcRect.Height()*m_srcRect.Width()];
        int w = m_srcRect.Width();

        for(int y=0;y<srcSize.y;++y){
            for(int x=0;x<srcSize.x;++x){
                DPoint tmp = m_trafo.transform_point(DPoint(int(x),int(y)));
                IntPoint tmp2(int(tmp.x+0.5),int(tmp.y+0.5));
                if(m_srcRect.Contains(tmp2)){
                    m_pMap[y*w+x] = tmp2;
                }else{
                    IntPoint SrcPoint(tmp2);
                    if (SrcPoint.x < m_srcRect.tl.x) {
                        SrcPoint.x = m_srcRect.tl.x);
                    } else if (SrcPoint.x >= m_srcRect.br.x) {
                        SrcPoint.x = m_srcRect.br.x;
                    } else if (SrcPoint.y < m_srcRect.tl.y) {
                        SrcPoint.y = m_srcRect.tl.y;
                    } else if (SrcPoint.y >= m_srcRect.br.y) {
                        SrcPoint.y = m_srcRect.br.y;
                    }
                    m_pMap[y*w+x] = SrcPoint;
                }

            }
        }
    }

    BitmapPtr FilterDistortion::apply(BitmapPtr pBmpSource){
        
        BitmapPtr res = BitmapPtr(new Bitmap(*pBmpSource));
        unsigned char *p = res->getPixels();
        unsigned char *src = pBmpSource->getPixels();
        unsigned char *pLine = p;
        int destStride = res->getStride();
        int srcStride = pBmpSource->getStride();
        int w=m_srcRect.Width();
        int h=m_srcRect.Height();
        IntPoint *pMapPos = m_pMap;
        for(int y=m_srcRect.tl.y;y<h;++y){
            for(int x=m_srcRect.tl.x;x<w;++x){
                *pLine = src[pMapPos->x + srcStride*pMapPos->y];
                pLine++;
                pMapPos++;
            }
            p+=destStride;
            pLine = p;
        }
        return res;

    }
}
