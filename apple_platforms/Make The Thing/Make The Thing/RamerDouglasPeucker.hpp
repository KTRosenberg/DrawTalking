//
//  RamerDouglasPeucker.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/6/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef RamerDouglasPeucker_hpp
#define RamerDouglasPeucker_hpp

namespace rdp {

//typedef std::pair<double, double> Point;
typedef vec3 Point;

double PerpendicularDistance(const Point &pt, const Point &lineStart, const Point &lineEnd);

bool RamerDouglasPeucker(std::vector<rdp::Point> &pointList, double epsilon, std::vector<rdp::Point> &out);
}

#endif /* RamerDouglasPeucker_h */
