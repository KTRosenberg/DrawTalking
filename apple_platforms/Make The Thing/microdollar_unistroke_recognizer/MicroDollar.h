/*
 
  MicroDollar: $1 Gesture Recognizer C++ port with optimizations for embedded hardware | Alex Olwal (www.olwal.com)

  DollarGestures provides a simple API for the gesture recognizer functionality. 

  It provides funcationality to:
  - initialize the gesture processing class (with corresponding parameters for memory and filtering)
  - load templates
  - adding coordinates
  - running recognizer on coordinate sequence, and storing result
  
    Optional use of int (microcontrollers) or float (better precision) for the template data
*/

#ifndef _DOLLAR_GESTURES_H_
#define _DOLLAR_GESTURES_H_

//#include <limits.h>
//#define MAX_VALUE INT_MAX;

#include <float.h>
#define MAX_VALUE FLT_MAX;

#include "DebugPrint.h"

#include "Recognizer.h"
#include "Templates.h"
#include "GestureProcessing.h"

class MicroDollar
{
    int index;
    float scoreFloat;
    int score;
    int x;
    int y;

    void loadTemplates(Recognizer<int> *r); //must match the data types in data_templates.h and data_parameters.h

	int ** samplePoints;
	int * nSamplePoints;
	bool samplesNormalized;
	const char ** sampleNames;
	int nTemplates;
	
  public:
    GestureProcessing gesture;
    Recognizer<int> recognizer; //must match the data types in data_templates.h and data_parameters.h
    
    /*
		samplePoints: 	Array of int arrays with x, y coordinates for sample data.
						Example: 
							int circleCCWPoints[] = { x0, y0, x1, y1, ... }
							int circleCWPoints[] = { a0, b0, a1, b1, ... }
							int * samplePoints = { circleCCWPoints, circleCWPoints, ... }
						
		nSamplePoints: 	Array of ints with lenght of each of the samplePoints arrays
						Example:
							int nSamplePoints[] = { 64, 64, ... }

		samplesNormalized:	Indicates if the samples are already normalized. 
							Should be true to save processing power on microcontrollers.
		
		sampleNames:	Array of character arrays (strings) that describe the gestures the 
						points represent.
						Examples:
							const char * sampleNames[] = { "circleCCW", "circleCW", ... }
							
        maxSamples: Number of samples that we allocate fixed memory for. (for performance reasons)
                    Overflow variable will indicate if allocated memory is too small. 
        
        dtGestures: Time w/o data between different gestures (i.e., maximum time between two points in a gesture). 
                    Can be used for motion-based sensors, for end-pointing.
        
        dtPoints: Minimum time between points for a gesture (assuming that a gesture will move with minimum velocity)
        
        minDistance: Minimum distance between points (e.g., avoid adding many points with small offset)
    */
    void init(int ** samplePoints, int * nSamplePoints, bool samplesNormalized, const char ** sampleNames, int nTemplates, int maxSamples, long dtGestures, long dtPoints, float minDistance);

    void init(int ** samplePoints, int * nSamplePoints, bool samplesNormalized, const char ** sampleNames, int nTemplates);
	
	
    MicroDollar();
    ~MicroDollar();

    int getIndex();
    int getScore();
    const char * getName();

    void printResult();
    bool hasGestureEnded();
    void recognize();
    
    bool update(int x, int y, bool relative);
    bool update(int x, int y); //assumes absolute
};

#endif
