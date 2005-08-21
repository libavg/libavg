//
// $Id$
//

#ifndef _Filtercolorize_H_
#define _Filtercolorize_H_

#include "Filter.h"

namespace avg {

// Filter that colorizes a bitmap given a hue and saturation. Corresponds
// loosely to the photoshop hue/saturation control when set to 'colorize'.
// The range of hue is 0..359, the range of saturation is 0..100.
class FilterColorize : public Filter
{
public:
  FilterColorize(double Hue, double Saturation);
  virtual ~FilterColorize();
  virtual void applyInPlace(BitmapPtr pBmp) const;

private:
  double m_Hue;
  double m_Saturation;
};

}  // namespace

#endif

