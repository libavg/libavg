#ifndef _ConnectedComps
#define _ConnectedComps


#include "../graphics/Bitmap.h"
#include "../graphics/Point.h"
#include "../graphics/Rect.h"
#include "../graphics/Pixel32.h"

#include <assert.h>
#include <boost/shared_ptr.hpp>
#include <list>
#include <vector>
#include <map>
namespace avg {
struct Run {
        Run(int row, int start_col, int end_col, int color);
        int m_Row;
        int m_StartCol;
        int m_EndCol;
        int m_Color;
        int length();
        DPoint center();
        int m_Label;
    private:
        static int s_LastLabel;
};

typedef boost::shared_ptr<struct Run> RunPtr;
typedef std::vector<struct Run> RunList;

struct BlobInfo {
    int m_ID;
    DPoint m_Center;
    double m_Area;
    IntRect m_BoundingBox;
    double m_Eccentricity;
    double m_Inertia;
    double m_Orientation;
    DPoint m_ScaledBasis[2];
    DPoint m_EigenVectors[2];
    DPoint m_EigenValues;
    // More to follow?
};

typedef boost::shared_ptr<BlobInfo> BlobInfoPtr;
typedef std::vector<BlobInfoPtr> BlobInfoList;
typedef boost::shared_ptr<BlobInfoList> BlobInfoListPtr;

class Blob;
typedef boost::shared_ptr<class Blob> BlobPtr;
class Blob {
    public:
        Blob(Run run);
        ~Blob();
        RunList& get_runs();
        DPoint center();
        int area();
        int getLabel();
        BlobInfoPtr getInfo();
        IntRect bbox();
        void merge( BlobPtr other);
        RunList* getList();
        void render(Bitmap *pTarget, Pixel32 Color, bool bMarkCenter, 
                Pixel32 CenterColor= Pixel32(0x00, 0x00, 0xFF, 0xFF));
        BlobPtr m_pParent;
    private:
        Blob(const Blob &);
        RunList *m_pRuns;
};


typedef std::vector<BlobPtr> BlobList;
typedef boost::shared_ptr<BlobList> BlobListPtr;
typedef std::map<int, BlobPtr> CompsMap;

BlobListPtr connected_components(BitmapPtr image, unsigned char object_threshold);
BlobListPtr connected_components(BitmapPtr image, BitmapPtr thresholds);
}
#endif
