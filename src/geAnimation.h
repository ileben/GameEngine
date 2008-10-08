#ifndef __GEANIMATION_H
#define __GEANIMATION_H

namespace GE
{

  template <class T> TimedKey
  {
    Float32 time;
    T value;
  };
  
  template <class T> TimedTrack
  {
    ArrayListTT <TimedKey<T> > *keys;
    T evalAt (Float time, int keyHint = 0, int *outKeyHint = NULL);
  };

  template <class T>
  T SkinTrack<T>::evalAt (Float time,
                          int keyHint,
                          int *outKeyHint)
  {
    //Exit soon if just 1 key
    if (keys->size() == 1)
      return keys->first().value;
    
    //Exit soon if time negative
    if (time <= 0.0f)
      return keys->first().value;
    
    //Exit soon if time too large
    if (time >= keys->last().time)
      return keys->last().value;
    
    //Start searching at the hinted key
    int key = keyHint >= keys->size() ? 0 : keyHint;
    
    //Make sure we start to search before the time sought
    if (keys->at (key) .time > time)
      key = 0;
    
    //Find first key with greater or equal time
    while (keys->at (key+1) .time < time)
      key++;
    
    //Calculate interpolation coeff
    SkinKey &key1 = keys->at (key);
    SkinKey &key2 = keys->at (key+1);
    Float alpha = (time - key1.time) / (key2.time - key1.time);
    
    //Output the key hint
    if (outKeyHint != NULL)
      *outKeyHint = key;
    
    //Here's where you return the interpolated value
  }


}//namespace GE
#endif//__GEANIMATION_H
