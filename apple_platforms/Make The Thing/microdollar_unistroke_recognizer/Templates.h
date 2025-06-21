/*
 
  MicroDollar: $1 Gesture Recognizer C++ port with optimizations for embedded hardware | Alex Olwal (www.olwal.com)

  Gesture template classes, specified as a C++ Template to allow use of different data types for data (e.g., int or float).
  This allows portability between 8-bit microcontrollers and more advanced platforms (for increased precision).

*/

#ifndef _TEMPLATES_H_
#define _TEMPLATES_H_

// Template with points and index for a gesture to recognize
template <class T>
class Template
{
  public:
    T * points; 
    int nPoints;
    int index;

	bool allocated;
	
    Template() : nPoints(0), index(0), allocated(false)
    {

    }
	
	void init(int nValues)
	{
		allocate(nValues);
	}
	
	~Template()
	{
		deallocate();
	}
	
	void allocate(int nValues)
	{		
		if (allocated)
			deallocate();

		points = new T[nValues];
		nPoints = nValues;
		memset(points, 0, nValues);
		//points[N_VALUES - 1] = -1; //for debugging
		allocated = true;		
	}
	
	void deallocate()
	{
		if (allocated)
		{
			delete[] points;
		}		
		
		allocated = false;
	}
	
};

// Holds a set of Templates for each gesture that we want to recognize
template <class T>
class Templates
{
  public:
  
	bool allocated;
  
    Template <T> * templates;
    int nLoaded;
	
    Templates() : nLoaded(0), allocated(false)
    {
		
    }

    ~Templates()
    {
		deallocate();
    }
		
	void init(int nValues, int nTemplates)
	{
		allocate(nValues, nTemplates);
	}

	void allocate(int nValues, int nTemplates)
	{		
		if (allocated)
			deallocate();
	
		templates = new Template<T>[nTemplates];
		
		for (int i = 0; i < nTemplates; i++)
			templates[i].init(nValues);
		
		allocated = true;	
	}
	
	void deallocate()
	{
		if (allocated)
		{
			delete[] templates;
		}
		
		allocated = false;
	}
	
    T * getPoints(int index)
    {
      return templates[index].points;
    }

    int getNPoints(int index)
    {
      return templates[index].nPoints;
    }


};

#endif
