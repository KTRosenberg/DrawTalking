/*
 
  MicroDollar: $1 Gesture Recognizer C++ port with optimizations for embedded hardware | Alex Olwal (www.olwal.com)

  Implementation of dollar recognizer as a C++ Template to allow use of different data types for data (e.g., int or float).
  This allows portability between 8-bit microcontrollers and more advanced platforms (for increased precision).

  Templates are loaded using either of:
    loadTemplate(...)
    loadTemplateNormalized(...)

  normalizePath(...) transforms coordinate sequences for matching:
  - resample
  - rotateToZero
  - scaleToSquare
  - translateToOrigin

  recognize(...) uses distanceAtBestAngle(...) to match normalized paths

  Based on:
  $1 Unistroke Recognizer (http://depts.washington.edu/madlab/proj/dollar/)
  - Gestures without libraries, toolkits or training: a $1 recognizer for user interface prototypes (https://doi.org/10.1145/1294211.1294238)
    
*/

#ifndef _RECOGNIZER_H_
#define _RECOGNIZER_H_

#include "Templates.h"

#define GET_ABS(X) X < 0 ? -X : X

#include "DebugPrint.h"

template <class T>
class Recognizer
{
    //static indices
    const static int X = 0;
    const static int Y = 1;

    //temp variables for reuse to avoid reallocation
    float x;
    float y;
    float radians;
    float bounds[4];

    //temp variables for distanceAtBestAngle(...)
    float startRange;
    float endRange;
    float x1;
    float f1;
    float x2;
    float f2;
    float absolute;

    //temp variables for recognize(...) -> don't use outside!
    T * pRecognize;
    int nTemp;
	
    //temp variable for distanceAtAngle(...) -> don't use outside!
	T * pDistanceAtAngle;

    //constants
    float halfDiagonal;
    float anglePrecision;
    float goldenRatio;
    float squareSize;
    float angleRange;

  public:

  	int nValues;
	int nTemplates;

  
    int	resample(T * src, int m, T * dst, int n);
    float pathLength(T * points, int n);
    float getDistance(T * p1, T * p2);

    void normalizePath(T * src, int nSrc, T * dst, int nDst);
    void rotateToZero(T * points, int n);
    void getCentroid(T * points, int n, float * x, float * y);

    void rotateBy(T * points, int n, float radians);
    void rotateBy(T * src, int nSrc, T * dst, int nDst, float radians);

    void translateToOrigin(T * points, int n);

    void scaleToSquare(T * points, int n);
    void getBoundingBox(T * points, int n);
    float distanceAtAngle(T * points, int n, int templateIndex, float radians);
    float distanceAtBestAngle(T * points, int n, int templateIndex);

    float pathDistance(T * points1, int n1, T * points2, int n2);

    void recognize(T * points, int n, int * index, float * score, float * distances = NULL, int nDistances = 0);

    Templates <T> templates;

    void loadTemplate(T * points, int n, int index);
    void loadTemplateNormalized(T * points, int n, int index);

    float getScore(float distance);

    static float roundFloat(float n)
    {
      return n - floor(n) >= 0.5 ? ceil(n) : floor(n);
    }

    Recognizer<T>();
    ~Recognizer<T>();
	
	void init(int nValues, int nTemplates);
	bool allocated;

	void allocate();
	void deallocate();
};

template <class T>
void Recognizer<T>::init(int nValues, int nTemplates)
{
	this->nValues = nValues;
	this->nTemplates = nTemplates;
	nTemp = nValues;	

	allocate();	
}

template <class T>
void Recognizer<T>::allocate()
{
	if (allocated)
		deallocate();
	
    pRecognize = new T[nValues];
 	pDistanceAtAngle = new T[nValues];
	
	templates.init(nValues, nTemplates);
	
	allocated = true;
}

template <class T>
void Recognizer<T>::deallocate()
{	
	if (allocated)
	{
		delete[] pRecognize;
		delete[] pDistanceAtAngle;
	}
	
	allocated = false;
}


template <class T>
Recognizer<T>::Recognizer()
{
  allocated = false;
  squareSize = 4096;
  //	halfDiagonal = 0.5 * sqrt( pow((float)squareSize, 2) + pow((float)squareSize, 2));
  halfDiagonal = 0.5 * sqrt( (squareSize * squareSize) + (squareSize * squareSize) );

  anglePrecision = 2.0;
  goldenRatio = 0.5 * (-1.0 + sqrt(5.0));
  angleRange = 45.0;
}

template <class T>
Recognizer<T>::~Recognizer()
{
	deallocate();
}

template <class T>
void Recognizer<T>::loadTemplate(T * points, int n, int index)
{
  Template <T> * t = &templates.templates[templates.nLoaded];

  normalizePath( points, n, t->points, nValues );

  t->nPoints = nValues;

  t->index = index;

  templates.nLoaded++;
}

template <class T>
void Recognizer<T>::loadTemplateNormalized(T * points, int n, int index)
{
  Template <T> * t = &templates.templates[templates.nLoaded];

  //	normalizePath( points, n, t->points, nValues );

  for (int i = 0; i < nValues; i++)
    t->points[i] = points[i];

  t->nPoints = nValues;
  t->index = index;

  templates.nLoaded++;
}

template <class T>
void print(const char * name, T * src, int nSrc)
{
  DEBUG_PRINT(name);
  DEBUG_PRINT(" (");
  DEBUG_PRINT(nSrc);
  DEBUG_PRINTLN(" values):");

  for (int i = 0; i < nSrc; i += 2, src += 2)
  {
    DEBUG_PRINT(src[0]);
    DEBUG_PRINT(" ");
    DEBUG_PRINT(src[1]);

    if (i < nSrc - 2)
      DEBUG_PRINT(", ");
  }

  DEBUG_PRINTLN("");
  DEBUG_PRINTLN("");

}

template <class T>
int Recognizer<T>::resample(T * src, int nSrc, T * dst, int nDst)
{
  float interval = pathLength(src, nSrc) / (nDst / 2 - 1); // interval length
  float D = 0.0;

  int j = 0;

  dst[X] = src[X];
  dst[Y] = src[Y];
  int resampled = 1; //we've added one point
  dst += 2; //advance pointer

  T * p1 = src + 2; //p
  T * p0 = src; //p-1

  float d;
  T resampledP[2];

  bool wasResampled = false;

  float v[2];

  for (int i = 2, j = 0; i < nSrc; i += 2, p1 += 2)
  {
    //		d = getDistance(p0, p1);

    if (wasResampled)
      p0 = resampledP;
    else
      p0 = p1 - 2;

    d = getDistance(p0, p1);

    DEBUG_PRINT(i);
    DEBUG_PRINT(": ");
    DEBUG_PRINT(d);
    DEBUG_PRINT("\t(");
    DEBUG_PRINT(p0[X]);
    DEBUG_PRINT(", ");
    DEBUG_PRINT(p0[Y]);
    DEBUG_PRINT(") (");
    DEBUG_PRINT(p1[X]);
    DEBUG_PRINT(", ");
    DEBUG_PRINT(p1[Y]);
    DEBUG_PRINTLN(")");

    wasResampled = false;

    if ((D + d) >= interval)
    {
      v[X] = p0[X] + ((interval - D) / d) * (p1[X] - p0[X]);
      v[Y] = p0[Y] + ((interval - D) / d) * (p1[Y] - p0[Y]);

      //should only be necesary for Arduino when using <int>
      dst[X] = roundFloat(v[X]);
      dst[Y] = roundFloat(v[Y]);

      resampled++; //keep track of added points

      //to avoid inserting elements -> reuse array
      i -= 2; //back the pointers
      p1 -= 2; //make sure that we use the same p1 for next comparison

      wasResampled = true;

      resampledP[X] = dst[X];
      resampledP[Y] = dst[Y];

      DEBUG_PRINT(dst[X]);
      DEBUG_PRINT(" ");
      DEBUG_PRINT(dst[Y]);
      DEBUG_PRINTLN(",");

      dst += 2; //advance pointer

      D = 0.0;
    }
    else
      D += d;
  }

  // somtimes we fall a rounding-error short of adding the last point, so add it if so
  if (resampled == nDst / 2 - 1)
  {
    resampled++;
    dst[X] = src[nSrc - 2];
    dst[Y] = src[nSrc - 1];
  }

  print("end of resample", dst, nDst);

  return resampled;
}

template <class T>
float Recognizer<T>::pathLength(T * points, int n)
{
  float distance = 0;

  T * p1 = points + 2; //p
  T * p0 = points; //p-1

  for (int i = 2; i < n; i += 2, p1 += 2, p0 += 2)
    distance += getDistance(p0, p1);

  return distance;
}

template <class T>
float Recognizer<T>::getDistance(T * p1, T * p2)
{
  float dx = p2[X] - p1[X];
  float dy = p2[Y] - p1[Y];

  return sqrt( dx * dx + dy * dy );

  //	return sqrt( pow( (float)(p2[X]-p1[X]), 2) + pow( p2[Y]-p1[Y], 2) );
  //	return sqrt( (float)((p2[X]-p1[X])*(p2[X]-p1[X]) + (p2[Y]-p1[Y])*(p2[Y]-p1[Y])) );
}

template <class T>
void Recognizer<T>::normalizePath(T * src, int nSrc, T * dst, int nDst)
{
  resample(src, nSrc, dst, nDst);

  print("resampled", dst, nDst);

  rotateToZero(dst, nDst);

  print("rotated to zero", dst, nDst);

  scaleToSquare(dst, nDst);

  print("scaled to square", dst, nDst);

  translateToOrigin(dst, nDst);

  print("translated to origin", dst, nDst);

}

template <class T>
void Recognizer<T>::rotateToZero(T * points, int n)
{
  getCentroid(points, n, &x, &y);
  radians = atan2(y - points[Y], x - points[X]);
  rotateBy(points, n, -radians);
}

template <class T>
void Recognizer<T>::getCentroid(T * points, int n, float * x, float * y)
{
  *x = 0;
  *y = 0;

  for (int i = 0; i < n; i += 2, points += 2)
  {
    *x += points[X];
    *y += points[Y];
  }

  *x /= (n / 2);
  *y /= (n / 2);
}

template <class T>
void Recognizer<T>::rotateBy(T * src, int nSrc, T * dst, int nDst, float radians)
{
  getCentroid(src, nSrc, &x, &y);

  float c = cos(radians);
  float s = sin(radians);

  float v[2];

  for (int i = 0; i < nSrc; i += 2, src += 2, dst += 2)
  {
    v[X] = (src[X] - x) * c - (src[Y] - y) * s + x;
    v[Y] = (src[X] - x) * s + (src[Y] - y) * c + y;

    //should only be necesary for Arduino when using <int>
    dst[X] = roundFloat(v[X]);
    dst[Y] = roundFloat(v[Y]);
  }
}

template <class T>
void Recognizer<T>::rotateBy(T * src, int nSrc, float radians)
{
  getCentroid(src, nSrc, &x, &y);

  float c = cos(radians);
  float s = sin(radians);
  float v[2];

  for (int i = 0; i < nSrc; i += 2, src += 2)
  {
    v[X] = (src[X] - x) * c - (src[Y] - y) * s + x;
    v[Y] = (src[X] - x) * s + (src[Y] - y) * c + y;

    //should only be necesary for Arduino when using <int>
    src[X] = roundFloat(v[X]);
    src[Y] = roundFloat(v[Y]);
  }
}

template <class T>
void Recognizer<T>::translateToOrigin(T * points, int n)
{
  getCentroid(points, n, &x, &y);

  for (int i = 0; i < n; i += 2, points += 2)
  {
    points[X] -= x;
    points[Y] -= y;
  }
}

template <class T>
void Recognizer<T>::scaleToSquare(T * points, int n)
{
  getBoundingBox(points, n);
  float v[2];

  for (int i = 0; i < n; i += 2, points += 2)
  {
    v[X] = points[X] * (squareSize / bounds[2]);
    v[Y] = points[Y] * (squareSize / bounds[3]);

    //should only be necesary for Arduino when using <int>
    points[X] = roundFloat(v[X]);
    points[Y] = roundFloat(v[Y]);
  }
}

template <class T>
void Recognizer<T>::getBoundingBox(T * points, int n)
{
  bounds[0] = MAX_VALUE; //minX
  bounds[1] = MAX_VALUE; //minY
  bounds[2] = 0; //maxX
  bounds[3] = 0; //maxY

  for (int i = 0; i < n; i += 2, points += 2)
  {
    if (points[X] < bounds[0])
      bounds[0] = points[X];
    if (points[X] > bounds[2])
      bounds[2] = points[X];
    if (points[Y] < bounds[1])
      bounds[1] = points[Y];
    if (points[Y] > bounds[3])
      bounds[3] = points[Y];
  }

  //bounds(minX, minY, (maxX - minX), (maxY - minY));
  bounds[2] = bounds[2] - bounds[0];
  bounds[3] = bounds[3] - bounds[1];
}

template <class T>
float Recognizer<T>::distanceAtAngle(T * points, int n, int templateIndex, float radians)
{
  //	memcpy(p, points, n * sizeof(T));
  rotateBy(points, n, pDistanceAtAngle, nValues, radians);
  return pathDistance(pDistanceAtAngle, nValues, templates.getPoints(templateIndex), templates.getNPoints(templateIndex));
}

template <class T>
float Recognizer<T>::distanceAtBestAngle(T * points, int n, int templateIndex)
{
  startRange = -angleRange;
  endRange   =  angleRange;
  x1 = goldenRatio * startRange + (1.0 - goldenRatio) * endRange;
  f1 = distanceAtAngle(points, n, templateIndex, x1);
  x2 = (1.0 - goldenRatio) * startRange + goldenRatio * endRange;
  f2 = distanceAtAngle(points, n, templateIndex, x2);

  while ( GET_ABS(endRange - startRange) > anglePrecision )
  {
    if (f1 < f2)
    {
      endRange = x2;
      x2 = x1;
      f2 = f1;
      x1 = goldenRatio * startRange + (1.0 - goldenRatio) * endRange;
      f1 = distanceAtAngle(points, n, templateIndex, x1);
    }
    else
    {
      startRange = x1;
      x1 = x2;
      f1 = f2;
      x2 = (1.0 - goldenRatio) * startRange + goldenRatio * endRange;
      f2 = distanceAtAngle(points, n, templateIndex, x2);
    }
  }

  //	return min(f1, f2);
  return f1 < f2 ? f1 : f2;
}

template <class T>
float Recognizer<T>::pathDistance(T * points1, int n1, T * points2, int n2)
{
  // assumes pts1.size == pts2.size

  float distance = 0.0;

  for (int i = 0; i < n1; i += 2, points1 += 2, points2 += 2)
    distance += getDistance(points1, points2);

  return (distance / (n1 / 2));
}

template <class T>
void Recognizer<T>::recognize(T * src, int nSrc, int * index, float * score, float * distances, int nDistances)
{
  //store all distances?
  bool storeMultipleDistances = (distances != NULL || nDistances == 0);

  memset(pRecognize, 0, nValues * sizeof(T));
  pRecognize[nValues - 1] = -1; //for debugging

  normalizePath(src, nSrc, pRecognize, nValues);

  float bestDistance = MAX_VALUE;

  int bestMatch = -1;

  DEBUG_PRINTLN("---------");

  for (int i = 0; i < templates.nLoaded; i++)
  {
    float distance = distanceAtBestAngle(pRecognize, nValues, i);

    if (i < nDistances)
      distances[i] = distance;

    if (distance < bestDistance)
    {
      bestDistance = distance;
      bestMatch = i;

    }

    DEBUG_PRINT("template: ");
    DEBUG_PRINT(i);
    DEBUG_PRINT(" = ");
    DEBUG_PRINT(distance);
    DEBUG_PRINTLN("");

  }

  *index = bestMatch;
  *score = getScore(bestDistance);
}

template <class T>
float Recognizer<T>::getScore(float distance)
{
  return 1.0 - (distance / halfDiagonal);
}

#endif



