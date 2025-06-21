/*
 
  MicroDollar: $1 Gesture Recognizer C++ port with optimizations for embedded hardware | Alex Olwal (www.olwal.com)

*/

#include "GestureProcessing.h"

bool
GestureProcessing::update(int x, int y, bool relative)
{
  t1 = millis();
  dt = t1 - t0;

  bool added = false;

  if ( dt < dtGestures )
  {
    if (command == GESTURE_END)
      process(GESTURE_START, x, y, relative);

    added = process(GESTURE_POINT, x, y, relative);

    DEBUG_PRINT(".");

    if (added)
      t0 = t1;

    return added;
  }
  else
  {
    t0 = t1;
    DEBUG_PRINT("x");

    return added;
  }
}

void
GestureProcessing::add(int x, int y)
{
  samples[nSamples] = x;
  samples[nSamples + 1] = y;

  if (nSamples + 2 >= MAX_SAMPLES)
    overflow++;

  nSamples = (nSamples + 2) % MAX_SAMPLES;
}

void
GestureProcessing::accumulate(int x, int y)
{
  samples[nSamples] = ( x + samples[nSamples] ) / 2;
  samples[nSamples + 1] = ( y + samples[nSamples + 1] ) / 2;
}

bool
GestureProcessing::process(int c, int x, int y, bool relative)
{
  //  if ( (t1 - t0) > updateDt && c != GESTURE_END)
  //    return;

  command = c;

  switch (command)
  {
    case GESTURE_START:
      DEBUG_PRINTLN("gesture start");
      gx = 0;
      gy = 0;
      nSamples = 0;
      overflow = 0;

      return false;

      break;

    case GESTURE_POINT:

      gx = relative ? gx + x : x;
      gy = relative ? gy + y : y;

      if (nSamples >= 2)
      {
        float d;

        d = dist(gx, gy, samples[nSamples - 2], samples[nSamples - 1]);

        //if we haven't moved much or too small time between the points
        if ( d < minDistance )
        {
          //					nSamples -= 2;
          DEBUG_PRINT("d");
          accumulate(gx, gy);

          return false;
        }
        else if ( dt < dtPoints )
        {
          DEBUG_PRINT("t");
          accumulate(gx, gy);

          return false;
        }
        else
        {
          DEBUG_PRINT(".");
          add(gx, gy);

          return true;
        }
      }
      else //always add first point
      {
        add(gx, gy);
        return true;
      }

      return false;

      break;

    case GESTURE_END:
      DEBUG_PRINTLN("\n");
      DEBUG_PRINT("gesture end (");
      DEBUG_PRINT(nSamples);
      DEBUG_PRINT(" values in buffer w/ ");
      DEBUG_PRINT(overflow);
      DEBUG_PRINTLN(" overflows");

      return false;
      break;
  }
    return false;
}

bool
GestureProcessing::hasGestureEnded()
{
  if ( (millis() - t0) > dtGestures && command == GESTURE_POINT )
  {
    gestureEnd();
    return true;
  }

  return false;
}

void
GestureProcessing::gestureEnd()
{
  process(GESTURE_END, 0, 0);
}


float
GestureProcessing::dist(int x1, int y1, int x2, int y2)
{
  float dx = x2 - x1;
  float dy = y2 - y1;

  return sqrt( dx * dx + dy * dy );
}

void
GestureProcessing::init(int maxSamples, long _dtGestures, long _dtPoints, float _minDistance)
{
  dtGestures = _dtGestures;
  dtPoints = _dtPoints;
  minDistance = _minDistance;
  overflow = 0;
  k = 0.1;

  command = GESTURE_END;
  samples = (int *)malloc(maxSamples * sizeof(int));
  MAX_SAMPLES = maxSamples;
  nSamples = 0;
  gx = 0;
  gy = 0;
  t0 = millis();

  initialized = true;
}

GestureProcessing::GestureProcessing() : initialized(false)
{

}

GestureProcessing::~GestureProcessing()
{
  free(samples);
}
