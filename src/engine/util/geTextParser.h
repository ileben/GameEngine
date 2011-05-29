#ifndef __TEXTPARSER_H
#define __TEXTPARSER_H

namespace GE
{
  
  namespace TextParserCommon
  {
    class S { public:
      inline static bool Test (char c)
        { return (c == ' ' || c == '\t'); }};
    
    class N { public:
      inline static bool Test (char c)
        { return (c == '\n'); }};
    
    class R { public:
      inline static bool Test (char c)
        { return (c == '\r'); }};
    
    class NR { public:
      inline static bool Test (char c)
        { return (c == '\n' || c == '\r'); }};
    
    class SNR { public:
      inline static bool Test (char c)
        { return (c == ' ' || c == '\t' || c == '\n' || c == '\r'); }};
    
    class Num { public:
      inline static bool Test (char c)
        { return (c >= '0' && c <= '9'); }};
    
    class Alpha { public:
      inline static bool Test (char c)
        { return ((c >= 'A' && c <= 'Z') ||
                  (c >= 'a' && c <= 'z')); }};
    
    class Var { public:
      inline static bool Test (char c)
        { return (Num::Test (c) || Alpha::Test (c) || c == '_'); }};
    
    template <char C>
      class Char { public:
      inline static bool Test (char c)
        { return (c == C); }};
    
    template <class T1, class T2>
      class Or { public:
      inline static bool Test (char c)
        { return (T1::Test(c) || T2::Test(c)); }};
    
    template <class T1, class T2>
      class And { public:
      inline static bool Test (char c)
        { return (T1::Test()(c) && T2::Test()(c)); }};
  }
  
  template <class StringType> class TextParser
  {
    friend class StaticMesh;
    
  protected:
    
    const StringType *text;
    int tokenStart;
    int tokenEnd;
    int index;
    
    typedef typename StringType::CharType CharType;
    
  public:
    
    TextParser ()
    {
      text = NULL;
      index = 0;
      tokenEnd = 0;
      tokenStart = 0;
    }
    
    TextParser (const StringType *text, int index)
    {
      begin (text, index);
    }
    
    void begin (const StringType *text, int index)
    {
      this->text = text;
      this->index = index;
      tokenEnd = 0;
      tokenStart = 0;
    }
    
    bool end ()
    {
      return index >= text->length();
    }
    
    int getIndex ()
    {
      return index;
    }
    
    CharType at ()
    {
      return text->at (index);
    }
    
    template <class CharTest>
      int jump (bool trueorfalse = true, int max = 0)
    {
      int count = 0;
      
      //Return if at end of input
      if (index >= text->length ())
        return 0;
      
      if (trueorfalse)
      {
        //Skip characters while they pass the test
        for (; CharTest::Test (text->at (index)); ++count)
        {
          //Stop if end reached
          if (++index >= text->length ())
            return count;
          
          //Stop if maximum allowed jumps reached
          if (max > 0 && count >= max)
            return count;
        }
      }
      else
      {
        //Skip characters until one passes the test
        for (; ! CharTest::Test (text->at (index)); ++count)
        {
          //Stop if end reached
          if (++index >= text->length ())
            return count;
          
          //Stop if maximum allowed jumps reached
          if (max > 0 && count >= max)
            return count;
        }
      }
      
      return count;
    }
    
    void fw (int num)
    {
      if (index < text->length())
        index += num;
    }
    
    int parseInteger ()
    {
      int length = 0;
      int value = text->parseIntegerAt (index, &length);
      index += length;
      return value;
    }
    
    float parseFloat ()
    {
      int length = 0;
      float value = text->parseFloatAt (index, &length);
      index += length;
      return value;
    }
    
    void beginToken ()
    {
      tokenStart = index;
      tokenEnd = index;
    }
    
    void endToken ()
    {
      tokenEnd = index;
    }
    
    bool compareToken (const char *str)
    {
      return text->compareAt (tokenStart, tokenEnd - tokenStart, str) == 0;
    }
    
    bool compareTokenIgnoreCase (const char *str)
    {
      return text->compareAt (tokenStart, tokenEnd - tokenStart, str, false) == 0;
    }
    
    template <class AnyCharType>
    bool compareToken (const BasicString<AnyCharType> &str)
    {
      return text->compareAt (tokenStart, tokenEnd - tokenStart,
                              str.buffer (), str.length ()) == 0;
    }
    
    template <class AnyCharType>
    bool compareTokenIgnoreCase (const BasicString<AnyCharType> &str)
    {
      return text->compareAt (tokenStart, tokenEnd - tokenStart,
                              str.buffer (), str.length (), false) == 0;
    }
    
    StringType getToken ()
    {
      return text->sub (tokenStart, tokenEnd - tokenStart);
    }
  };
  
}//namespace GE
#endif//__TEXTPARSER_H
