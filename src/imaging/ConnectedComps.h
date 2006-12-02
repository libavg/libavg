#ifndef _ConnectedComps
#define _ConnectedComps


#include "../graphics/Bitmap.h"
#include "../graphics/Point.h"
#include "../graphics/Rect.h"

#include <assert.h>
#include <boost/shared_ptr.hpp>
#include <list>
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
typedef std::list<RunPtr> RunList;

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
typedef std::list<BlobInfoPtr> BlobInfoList;
typedef boost::shared_ptr<BlobInfoList> BlobInfoListPtr;

class Blob;
typedef boost::shared_ptr<class Blob> BlobPtr;
class Blob {
    public:
        Blob(RunPtr run);
        ~Blob();
        RunList& get_runs();
        DPoint center();
        int area();
        int getLabel();
        BlobInfoPtr getInfo();
        IntRect bbox();
        void merge( BlobPtr other);
        RunList* getList();
        BlobPtr m_pParent;
        friend void render(const Bitmap *target, BlobPtr blob);
    private:
        Blob(const Blob &);
        RunList *m_pRuns;
};


typedef std::list<BlobPtr> BlobList;
typedef boost::shared_ptr<BlobList> BlobListPtr;
typedef std::map<int, BlobPtr> CompsMap;



void render(Bitmap *target, BlobPtr blob, unsigned char col);
BlobListPtr connected_components(BitmapPtr image, int object_threshold);
}
#endif
