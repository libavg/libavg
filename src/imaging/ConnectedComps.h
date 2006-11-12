
#include "../graphics/Bitmap.h"
#include "../graphics/Point.h"
#include "../graphics/Rect.h"

#include <assert.h>
#include <boost/shared_ptr.hpp>
#include <list>
#include <map>
namespace avg {
class Run {
    public:
        Run(int row, int start_col, int end_col, int color);
        int m_Row;
        int m_StartCol;
        int m_EndCol;
        int m_Color;
        int length();
        DPoint center();
        int m_Label;
    private:
        static int last_label;
};

typedef boost::shared_ptr<class Run> RunPtr;
typedef std::list<RunPtr> RunList;
typedef std::list<DPoint> DPointList;

class Blob;
typedef boost::shared_ptr<class Blob> BlobPtr;
class Blob {
    public:
        Blob(RunPtr run);
        ~Blob();
        RunList& get_runs();
        BlobPtr m_pParent;
        DPoint center();
        int area();
        double stddev();
        DPointList *pca();
        IntRect bbox();
        void merge( BlobPtr other);
        RunList* getlist();
    private:
        RunList *m_pRuns;
};


typedef std::list<BlobPtr> BlobList;
typedef std::map<int, BlobPtr> CompsMap;
BlobList *connected_components(BitmapPtr image);
}
