//---------------------------------------------------------------------------
#include "timedist.h"
//---------------------------------------------------------------------------
using namespace Taquart;

//---------------------------------------------------------------------------
TimeDist::TimeDist(double ATime, double ADistance) :
    Time(ATime), Distance(ADistance) {
  // Default constructor
}

//---------------------------------------------------------------------------
TimeDist::TimeDist(void) :
    Time(0.0), Distance(0.0) {
  // Default constructor
}

//---------------------------------------------------------------------------
TimeDist::TimeDist(const TimeDist &Src) {
  Time = Src.Time;
  Distance = Src.Distance;
}

//---------------------------------------------------------------------------
const TimeDist& TimeDist::operator=(const TimeDist &Src) {
  if (this != &Src) {
    Time = Src.Time;
    Distance = Src.Distance;
  }
  return *this;
}

//---------------------------------------------------------------------------
bool operator<(const Taquart::TimeDist& X, const Taquart::TimeDist& Y) {
  return X.Distance < Y.Distance;
}

//---------------------------------------------------------------------------
bool operator==(const Taquart::TimeDist& X, const Taquart::TimeDist& Y) {
  return X.Distance == Y.Distance;
}
