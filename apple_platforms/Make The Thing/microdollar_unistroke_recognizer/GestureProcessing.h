/*
 
  MicroDollar: $1 Gesture Recognizer C++ port with optimizations for embedded hardware | Alex Olwal (www.olwal.com)

  The GestureProcessing class interfaces manages coordinate samples:
  - Filters coordinates based on time and distance (to control points being added to sequence)
  - Sets endpoints for coordinate sequence

  Important parameters:

    maxSamples: Number of samples that we allocate fixed memory for. (for performance reasons)
                overflow variable will indicate if allocated memory is too small. 
    
    dtGestures: Time w/o data between different gestures (i.e., maximum time between two points in a gesture). 
                Can be used for motion-based sensors, for end-pointing.
    
    dtPoints: Minimum time between points for a gesture (assuming that a gesture will move with minimum velocity)
    
    minDistance: Minimum distance between points (e.g., avoid adding many points with small offset)

*/

#ifndef _GESTURE_PROCESSING_H_
#define _GESTURE_PROCESSING_H_

#include "DebugPrint.h"

#include <time.h>
#include <math.h>
#include <stdlib.h>

class GestureProcessing
{
    float dist(int x1, int y1, int x2, int y2);
    bool process(int c, int x, int y, bool relative = true);

  public:
    unsigned long t0;
    unsigned long t1;
    unsigned long dt;

    unsigned long minDistance; //minimum distance between points

    unsigned long dtGestures; //time w/o data between different gestures (i.e., maximum time between two points in a gesture)
    unsigned long dtPoints; //the minimum time between points for a gesture

    float k;
    int overflow;

    enum { GESTURE_START, GESTURE_POINT, GESTURE_END };

    int command;

    int gx;
    int gy;

    float x;
    float y;

    int * samples;
    int nSamples;
    int MAX_SAMPLES;

    bool initialized;

    GestureProcessing();
    ~GestureProcessing();

    bool hasGestureEnded();
    bool update(int x, int y, bool relative = true);
    void add(int x, int y);
    void accumulate(int x, int y);

    void gestureEnd();

    void init(int maxSamples, long _dtGestures, long _dtPoints, float _minDistance);

};

#endif
