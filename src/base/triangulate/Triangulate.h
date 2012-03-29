/*
 * Triangulation.h
 *
 *  Created on: Mar 27, 2012
 *      Author: Benjamin Granzow
 */

#ifndef TRIANGULATION_H_
#define TRIANGULATION_H_

#include "/home/dysman/devel/libavg/libavg/src/base/GLMHelper.h" ///TODO remove prefix of include
namespace avg {

std::vector<int> triangulatePolygon(const Vec2Vector& points,
		const std::vector<int>& holeIndexes = std::vector<int>());

}

#endif /* TRIANGULATION_H_ */
