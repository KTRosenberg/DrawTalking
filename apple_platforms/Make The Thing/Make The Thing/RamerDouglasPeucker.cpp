//2D implementation of the Ramer-Douglas-Peucker algorithm
//By Tim Sheerman-Chase, 2016
//Released under CC0
//https://en.wikipedia.org/wiki/Ramer%E2%80%93Douglas%E2%80%93Peucker_algorithm

#include <cmath>
#include <utility>
#include <vector>

#include "RamerDouglasPeucker.hpp"

namespace rdp {

double PerpendicularDistance(const rdp::Point &pt, const rdp::Point &lineStart, const rdp::Point &lineEnd)
{
    double dx = lineEnd.x - lineStart.x;//lineEnd.first - lineStart.first;
    double dy = lineEnd.y - lineStart.y;//lineEnd.second - lineStart.second;

	//Normalise
	double mag = pow(pow(dx,2.0)+pow(dy,2.0),0.5);
	if(mag > 0.0)
	{
		dx /= mag; dy /= mag;
	}

    double pvx = pt.x - lineStart.x;//pt.first - lineStart.first;
    double pvy = pt.y - lineStart.y;//pt.second - lineStart.second;

	//Get dot product (project pv onto normalized direction)
	double pvdot = dx * pvx + dy * pvy;

	//Scale line direction vector
	double dsx = pvdot * dx;
	double dsy = pvdot * dy;

	//Subtract this from pv
	double ax = pvx - dsx;
	double ay = pvy - dsy;

	return pow(pow(ax,2.0)+pow(ay,2.0),0.5);
}

bool RamerDouglasPeucker(std::vector<rdp::Point> &pointList, double epsilon, std::vector<rdp::Point> &out)
{
    if(pointList.size()<2) {
        out = pointList;
        return false;
    }
	// Find the point with the maximum distance from line between start and end
	double dmax = 0.0;
	size_t index = 0;
	size_t end = pointList.size()-1;
	for(size_t i = 1; i < end; i++)
	{
		double d = PerpendicularDistance(pointList[i], pointList[0], pointList[end]);
		if (d > dmax)
		{
			index = i;
			dmax = d;
		}
	}

	// If max distance is greater than epsilon, recursively simplify
	if(dmax > epsilon)
	{
		// Recursive call
		std::vector<Point> recResults1;
		std::vector<Point> recResults2;
		std::vector<Point> firstLine(pointList.begin(), pointList.begin()+index+1);
		std::vector<Point> lastLine(pointList.begin()+index, pointList.end());
		RamerDouglasPeucker(firstLine, epsilon, recResults1);
		RamerDouglasPeucker(lastLine, epsilon, recResults2);
 
		// Build the result list
		out.assign(recResults1.begin(), recResults1.end()-1);
		out.insert(out.end(), recResults2.begin(), recResults2.end());
		if(out.size()<2)
            return false;
	} 
	else 
	{
		//Just return start and end points
		out.clear();
		out.push_back(pointList[0]);
		out.push_back(pointList[end]);
	}
    
    return true;
}

}
