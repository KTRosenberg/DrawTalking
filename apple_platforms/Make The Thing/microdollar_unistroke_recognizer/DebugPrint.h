/*
 
  MicroDollar: $1 Gesture Recognizer C++ port with optimizations for embedded hardware | Alex Olwal (www.olwal.com)

  The debug class maps DEBUG_PRINT and DEBUG_PRINTLN to nothing, or to platform specific print statemetns
     
*/

#ifndef _DEBUG_H_
#define _DEBUG_H_

//#define DEBUG

#ifndef DEBUG
#define DEBUG_PRINT(X)
#define DEBUG_PRINTLN(X)
#endif

/* --- Windows --- */
#if defined(WIN32)

#include <iostream>
using namespace std;
#include <ctime>
#include <math.h>

#ifdef DEBUG
#define DEBUG_PRINT(X)		cout<<X;
#define DEBUG_PRINTLN(X)	cout<<X<<endl;
#endif

#define millis()	clock()
/* --- end Windows --- */
/* --- Arduino Uno --- */
#elif defined(ARDUINO_AVR_UNO)

#include <Arduino.h>
#include <stdlib.h>

#ifdef DEBUG
#define DEBUG_PRINT(X)		Serial.print(X);
#define DEBUG_PRINTLN(X)	Serial.println(X);
#endif

#define millis()	millis()

/* --- end Arduino Uno --- */
/* --- ESP8266 --- */
#elif defined(ESP8266)

#include <Arduino.h>
#include <stdlib.h>

#ifdef DEBUG
#define DEBUG_PRINT(X)		Serial.print(X);
#define DEBUG_PRINTLN(X)	Serial.println(X);
#endif

#define millis()	millis()
/* --- other platform --- */
#else
#include <iostream>
#include <stdlib.h>
#include <time.h>
#include <math.h>
//#include <cstdio>

using namespace std;

#define DEBUG_PRINT(X)		cout << X;
#define DEBUG_PRINTLN(X)	cout << X << endl;

#define millis()	clock()
#endif
/* --- end other platform --- */

#endif // _DEBUG_H_
