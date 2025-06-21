/*
 
  MicroDollar: $1 Gesture Recognizer C++ port with optimizations for embedded hardware | Alex Olwal (www.olwal.com)
   
*/

#include "MicroDollar.h"

void
MicroDollar::init(int ** samplePoints, int * nSamplePoints, bool samplesNormalized, const char ** sampleNames, int nTemplates, int maxSamples, long _dtGestures, long _dtPoints, float _minDistance)
{
  this->samplePoints = samplePoints;
  this->nSamplePoints = nSamplePoints;
  this->samplesNormalized = samplesNormalized;
  this->sampleNames = sampleNames;
  this->nTemplates = nTemplates; 
	
  gesture.init(maxSamples, _dtGestures, _dtPoints, _minDistance);
  
  recognizer.init(nSamplePoints[0], nTemplates);
  loadTemplates(&recognizer);
}

void
MicroDollar::init(int ** samplePoints, int * nSamplePoints, bool samplesNormalized, const char ** sampleNames, int nTemplates)
{
  init(samplePoints, nSamplePoints, samplesNormalized, sampleNames, nTemplates, nSamplePoints[0] * 1.5f, 1000, 0, 0);
}

void
MicroDollar::printResult()
{
  DEBUG_PRINT(index);
  DEBUG_PRINT(" ");
  DEBUG_PRINT(score);
  DEBUG_PRINT(" ");

  if (index >= 0 && index < nTemplates)
  {
    DEBUG_PRINTLN(sampleNames[index]);
  }
  else
  {
    DEBUG_PRINTLN("");
  }
}

void
MicroDollar::loadTemplates(Recognizer<int> * r)
{
  for (int i = 0; i < nTemplates; i++)
  {
    if (samplesNormalized)
      r->loadTemplateNormalized(samplePoints[i], nSamplePoints[i], i);
    else
      r->loadTemplate(samplePoints[i], nSamplePoints[i], i);
  }
}

bool
MicroDollar::hasGestureEnded()
{

  if (gesture.hasGestureEnded())
  {
    recognize();
    return true;
  }

  return false;
}

void
MicroDollar::recognize()
{
  recognizer.recognize( gesture.samples, gesture.nSamples, &index, &scoreFloat );
  score = (int)(scoreFloat * 100);
  gesture.gestureEnd();
}

bool
MicroDollar::update(int x, int y, bool relative)
{
  return gesture.update(x, y, relative);
}

bool
MicroDollar::update(int x, int y)
{
  return update(x, y, false);
}

int
MicroDollar::getIndex()
{
  return index;
}

int
MicroDollar::getScore()
{
  return score;
}

const char *
MicroDollar::getName()
{
    if (index >= 0 && index < nTemplates)
        return sampleNames[index];
    
    return nullptr;
}

MicroDollar::MicroDollar()
{

}

MicroDollar::~MicroDollar()
{

}
