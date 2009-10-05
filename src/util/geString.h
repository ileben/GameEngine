#ifndef __GESTRING_H
#define __GESTRING_H

namespace GE
{

  #if defined( WIN32 )
  int GE_API_ENTRY win_snprintf (char *str, size_t size, const char *format, ...);
  int GE_API_ENTRY win_vsnprintf (char *str, size_t size, const char *format, va_list ap);
  # define snprintf win_snprintf
  # define vsnprintf win_vsnprintf
  #endif

  /*
  ----------------------------------------
  Forward declarations
  ----------------------------------------*/

  class File;
  typedef unsigned int Unicode;
  
  /*
  ----------------------------------------
  Base template class
  ----------------------------------------*/

  template <class C> class BasicString : public Object
  {
    DECLARE_SERIAL_SUBCLASS( BasicString, Object );
    DECLARE_DATAVAR( size );
    DECLARE_MEMBER_FUNC( buf, bufInfo );
    DECLARE_CALLBACK( ClassEvent::Loaded, deserialized );
    DECLARE_CALLBACK( ClassEvent::Deserialized, deserialized );
    DECLARE_END;

    friend class File;
    friend class BasicString<char>;
    friend class BasicString<Byte>;
    friend class BasicString<Unicode>;
    
  public:
    
    typedef C CharType;
    
  protected:
    
    CharType *buf;
    int cap;
    int size;
    
  public:

    MemberInfo bufInfo () {
      return MEMBER_DATAPTR( (size+1) * sizeof(CharType) );
    }

    void deserialized (void *param) {
      cap = size+1;
    }

    BasicString (SM *sm)
    {}

    BasicString ()
    {
      buf = (CharType*) malloc( sizeof(CharType) );
      cap = 1;
      size = 0;
      buf[ size ] = (CharType) 0;
    }
    
    BasicString (const CharType *chars, int size)
    {
      int newcap = (size<=0) ? 1 : size + 1;
      int newsize = (size<=0) ? 0 : size;
      this->cap = newcap;
      this->size = newsize;
      buf = (CharType*) malloc( cap * sizeof(CharType) );
      memcpy( buf, chars, newsize * sizeof(CharType) );
      buf[ size ] = (CharType) 0;
    }
    
    BasicString (const BasicString<CharType> &str)
    {
      int newcap = (str.size<=0) ? 1 : str.size + 1;
      int newsize = (str.size<=0) ? 0 : str.size;
      cap = newcap;
      size = newsize;
      buf = (CharType*) malloc( cap * sizeof(CharType) );
      memcpy( buf, str.buf, newsize * sizeof(CharType) );
      buf[ size ] = (CharType) 0;
    }
    
    template <class OtherCharType>
    BasicString (const BasicString<OtherCharType> &str)
    {
      int newcap = (str.size<=0) ? 1 : str.size + 1;
      int newsize = (str.size<=0) ? 0 : str.size;
      cap = newcap;
      size = newsize;
      buf = (CharType*) malloc( cap * sizeof(CharType) );
      for (int s=0; s<newsize; ++s) buf[s] = (CharType) str[s];
      buf[ size ] = (CharType) 0;
    }
 
    BasicString (const char *str)
    {
      int len = (int) strlen (str);
      int newcap = (len<=0) ? 1 : len + 1;
      int newsize = (len<=0) ? 0 : len;
      cap = newcap;
      size = newsize;
      buf = (CharType*) malloc( cap * sizeof(CharType) );
      for (int s=0; s<newsize; ++s) buf[s] = (CharType) str[s];
      buf[ size ] = (CharType) 0;
    }
    
    BasicString (int capacity)
    {
      if (capacity < 0) capacity = 0;
      cap = capacity + 1;
      size = 0;
      buf = (CharType*) malloc( cap * sizeof(CharType) );
      buf[ size ] = (CharType) 0;
    }
    
    virtual ~BasicString()
    {
      if (buf)
        free( buf );
    }
    
    void clear()
    {
      size = 0;
      buf[ size ] = (CharType) 0;
    }
    
    /**
    Assures the string can hold at least [capacity] number of characters.
    Existing contents are cleared and size is reset to 0.
    **/
    void reserve (int capacity)
    {
      if (cap < capacity + 1) {
        
        free(buf);
        cap = capacity + 1;
        buf = (CharType*) malloc( cap * sizeof(CharType) );
      }
      
      size = 0;
      buf[ size ] = 0;
    }
    
    /**
    Assures the string can hold at least [capacity] number of characters.
    Existing contents are preserved in case of reallocation.
    **/
    void reserveAndCopy (int capacity)
    {
      if (cap >= capacity + 1) return;
      
      cap = capacity + 1;
      buf = (CharType*) realloc( buf, cap * sizeof(CharType) );
    }
    
    
    /**
    Change the string capacity so that it can hold exactly [capacity]
    number of characters. The existing contents are copied but the size is
    truncated if the new capacity is smaller.
    **/
    void setCapacity (int capacity)
    {
      if (capacity <= 0) return;
      int newsize = capacity < size ? capacity : size;
      
      cap = capacity + 1;
      size = newsize;
      buf = (CharType*) realloc( buf, cap * sizeof(CharType) );
      buf[ size ] = (CharType) 0;
    }
    
  private:
    
    /*
    The <mychars> argument is there just so we can write the specialized
    case when a const char* C-string is being assigned to String of type
    unsigned char*.
    */
    BasicString<CharType>& __assign (CharType *mychars,
                                     const CharType *achars, int asize)
    {
      if (cap < asize + 1) {
        
        free( buf );
        cap = asize + 1;
        buf = (CharType*) malloc( cap * sizeof(CharType) );
      }
      
      memcpy( buf, achars, asize * sizeof(CharType) );
      size = asize;
      buf[ size ] = 0;
      
      return *this;
    }
    
    /*
    This assignment route will be taken when a string of any different
    character type is being assigned.
    */
    template<class OtherCharType>
    BasicString<CharType>& __assign (CharType *mychars,
                                     const OtherCharType *achars, int asize)
    {
      if (cap < asize + 1) {
        
        free( buf );
        cap = asize + 1;
        buf = (CharType*) malloc( cap * sizeof(CharType) );
      }
      
      for (int c=0; c<asize; ++c)
        buf[c] = (CharType) achars[c];
      size = asize;
      buf[ size ] = 0;
      
      return *this;
    }

    /*
    This is a specialized function for the case when a const char*
    C-string is being assigned to a String of type unsigned char* so
    it doesn't take the route for assigning a different character type
    string.
    */
    BasicString <CharType>& __assign (unsigned char *mychars,
                                      const char *achars, int size)
    {
      __assign (buf, (const unsigned char *) achars, size);
      return *this;
    }
    
  public:
    
    /*
    Public assign function resolves to one of internal ones at compile time.
    */
    template <class AnyCharType>
    BasicString <CharType>& assign (const AnyCharType *achars, int size)
    {
      __assign (buf, achars, size);
      return *this;
    }
    
    /*
    The assignment operator functions utilize the appropriate overloaded
    variations of the assign() function, efficiently resolved at compile time.
    */
    BasicString<CharType>& operator= (CharType c)
    {
      __assign (buf, &c, 1);
      return *this;
    }
    
    BasicString<CharType>& operator= (const BasicString<CharType> &str)
    {
      __assign (buf, str.buffer(), str.length());
      return *this;
    }
    
    template <class OtherCharType>
    BasicString<CharType>& operator= (const BasicString<OtherCharType> &str)
    {
      __assign (buf, str.buffer(), str.length());
      return *this;
    }
    
    BasicString<CharType>& operator= (const char *cstr)
    {
      __assign (buf, cstr, (int) strlen (cstr));
      return *this;
    }
    
    /*
    The <mychars> argument is there just so we can write the specialized
    case when a const char* C-string is being appended to a String of type
    unsigned char* character type.
    */
    BasicString<CharType>& __append (CharType *mychars,
                                     const CharType *achars, int asize)
    {
      if (cap < size + asize + 1) {
        
        cap = size + asize + 1;
        buf = (CharType*) realloc (buf, cap * sizeof (CharType));
      }

      memcpy (&buf [size], achars, asize * sizeof (CharType));
      size += asize;
      buf [size] = (CharType) 0;
      
      return *this;
    }
    
    /*
    This appending route will be taken when a string of any different
    character type is being appended.
    */
    template <class OtherCharType>
    BasicString<CharType>& __append (CharType *mychars,
                                     const OtherCharType *achars, int asize)
    {
      if (cap < size + asize + 1) {
        
        cap = size + asize + 1;
        buf = (CharType*) realloc (buf, cap * sizeof (CharType));
      }
      
      for (int c=0; c<asize; ++c)
        buf [size + c] = (CharType) achars [c];
      size += asize;
      buf [size] = (CharType) 0;
      
      return *this;
    }
    
    /*
    This is a specialized function for the case when a const char*
    C-string is being appended to a String of type unsigned char* so
    it doesn't take the route for appending a different character type
    string.
    */
    BasicString <CharType>& __append (unsigned char *mychars,
                                      const char *achars, int size)
    {
      __append (buf, (const unsigned char *) achars, size);
      return *this;
    }
    
    /*
    Public append function resolves to one of internal ones at compile time.
    */
    template <class AnyCharType>
    BasicString <CharType>& append (const AnyCharType *achars, int asize)
    {
      __append (buf, achars, size);
      return *this;
    }
    
    /*
    The append operator functions utilize the appropriate overloaded
    variations of the assign() function, efficiently resolved at compile time.
    */
    BasicString<CharType>& operator+= (CharType c)
    {
      __append (buf, &c, 1);
      return *this;
    }
    
    BasicString<CharType>& operator+= (const BasicString<CharType> &str)
    {
      __append (buf, str.buf, str.size);
      return *this;
    }
    
    template <class OtherCharType>
    BasicString<CharType>& operator+= (const BasicString<OtherCharType> &str)
    {
      __append (buf, str.buf, str.size);
      return *this;
    }
    
    BasicString<CharType>& operator += (const char *cstr)
    {
      __append (buf, cstr, (int) strlen (cstr));
      return *this;
    }
    
    /*
    The inline append operator functions utilize the appropriate overloaded
    variations of the append operator, efficiently resolved at compile time.
    */
    BasicString<CharType> operator+ (CharType b) const
    {
      BasicString newstr (size + 1);
      newstr = *this;
      newstr += b;
      return newstr;
    }
    
    BasicString<CharType> operator+ (const BasicString<CharType> &str) const
    {
      BasicString newstr (size + str.size);
      newstr = *this;
      newstr += str;
      return newstr;
    }
    
    template <class OtherCharType>
    BasicString<CharType> operator+ (const BasicString<OtherCharType> &str) const
    {
      BasicString newstr (size + str.size);
      newstr = *this;
      newstr += str;
      return newstr;
    }
    
    BasicString<CharType> operator+ (const char *cstr) const
    {
      BasicString newstr (size + (int) strlen (cstr));
      newstr = *this;
      newstr += cstr;
      return newstr;
    }
    
    /*
    Replaces a part of the string with another substring. The replacement
    string does not have to be the same length as the part being replaced.
    */
    BasicString<CharType>& replace (int start, int count,
                                    const BasicString<CharType> &str)
    {
      if (start < 0) return *this;
      if (count <= 0) return *this;
      if (count > size) return *this;
      if (start + count > size) return *this;
      
      int leftlen = start;
      int rightstart = start + count;
      int rightlen = size - (start + count);
      int newlen = leftlen + str.size + rightlen;
      
      cap = newlen + 1;
      
      CharType *newbuf = (CharType*) malloc (cap * sizeof(CharType));
      memcpy (newbuf, buf, leftlen * sizeof(CharType));
      memcpy (&newbuf[leftlen], str.buf, str.size * sizeof(CharType));
      memcpy (&newbuf[leftlen+str.size], &buf[rightstart], rightlen * sizeof(CharType));
      
      free(buf);
      buf = newbuf;
      size = newlen;
      buf [size] = (CharType) 0;
      
      return *this;
    }
    
    /*
    Compares a part of the string with another string of any character
    type. The comparision is done in this string's character type domain.
    If <caseMatters> is false, uppercase characters will be matched against
    their lowercase pairs and vice-versa.
    */
    template <class AnyCharType>
    int compareAt (int start, int len,
                   const AnyCharType *str, int strsize,
                   bool caseMatters = true) const
    {
      int b, s;
      int caseOffset = 'a' - 'A';
      int end = start + len;
      
      //Return -1 on invalid input
      if (start < 0) return -1;
      if (len < 0) return -1;
      if (end > size) return -1;
      
      //Return early for different string lengths
      if (len < strsize) return -1;
      if (len > strsize) return 1;
      
      //Compare character by character
      for (b=start, s=0; b<end && s<strsize; ++b, ++s) {
        
        if (!caseMatters){
          if (buf[b] >= 'A' && buf[b] <= 'Z' &&
              ((CharType) str[s]) - buf[b] == caseOffset)
            continue;
          if (buf[b] >= 'a' && buf[b] <= 'z' &&
              buf[b] - ((CharType) str[s]) == caseOffset)
            continue;}
        
        if (buf[b] < (CharType) str[s])
          return -1;
        else if (buf[b] > (CharType) str[s])
          return 1;
      }
      
      //Both string must have ended by now
      return 0;
    }

    int compareAt (int start, int len,
                   const char *str,
                   bool caseMatters = true) const
    {
      return compareAt (start, len, str, strlen (str), caseMatters);
    }
    
    
    /*
    Wrappers that compare the whole string */

    template <class AnyCharType>
    int compare (AnyCharType *str, int strsize,
                 bool caseMatters = true) const
    {
      return compareAt (0, size, str, strsize, caseMatters);
    }

    template <class AnyCharType>
    int compare (const BasicString< AnyCharType > &str,
                 bool caseMatters = true) const
    {
      return compareAt( 0, size, str.buf, str.size, caseMatters);
    }
   
    int compare (const char *str,
                 bool caseMatters = true) const
    {
      return compareAt (0, size, str, (int) strlen (str), caseMatters);
    }
    
    /*
    Wrappers returning bool instead of -1, 0, 1. */

    template <class AnyCharType>
    bool equals (const AnyCharType *echars, int esize, bool caseMatters=true) const
    {
      return compare (echars, esize, caseMatters) == 0;
    }
    
    template <class AnyCharType>
    bool equals (const BasicString<AnyCharType> &str, bool caseMatters=true) const
    {
      return compare (str.buf, str.size, caseMatters) == 0;
    }
    
    bool equals (const char *cstr, bool caseMatters=true) const
    {
      return compare (cstr, caseMatters) == 0;
    }
    
    template <class AnyCharType>
    bool equalsIgnoreCase (const AnyCharType *echars, int esize) const
    {
      return compare (echars, esize, false) == 0;
    }
    
    template <class AnyCharType>
    bool equalsIgnoreCase (const BasicString<AnyCharType> &str) const
    {
      return compare (str.buf, str.size, false) == 0;
    }
    
    bool equalsIgnoreCase (const char *cstr) const
    {
      return compare (cstr, false) == 0;
    }
    
    /*
    Comparison operators wrappers returning bool instead of -1, 0, 1. */

    bool operator== (const BasicString<CharType> &str) const {
      return compare (str.buf, str.size) == 0;
    }
    
    bool operator!= (const BasicString<CharType> &str) const {
      return compare (str.buf, str.size) != 0;
    }
    
    bool operator== (const char *cstr) const {
      return compare (cstr) == 0;
    }
    
    bool operator!= (const char *cstr) const {
      return compare (cstr) != 0;
    }

    bool operator< (const BasicString<CharType> &str) const {
      return compare (str.buf, str.size) == -1;
    }
    
    /*
    Content accessor functions.
    */
    CharType operator[] (int i) const {
      return buf[i];
    }
    
    CharType at (int i) const {
      return buf[i];
    }
    
    const CharType* buffer() const {
      return (const CharType*)buf;
    }
    
    int capacity() const {
      return cap - 1;
    }
    
    int length() const {
      return size;
    }
    
    BasicString<CharType> sub(int start, int length) const
    {
      if (start < 0)
        return BasicString<CharType>();
      
      if (start >= size)
        return BasicString<CharType>();
      
      if (length <= 0)
        return BasicString<CharType>();
      
      if (start + length > size)
        length = size - start;
      
      return BasicString<CharType>(&buf[start], length);
    }
    
    BasicString<CharType> sub(int start) const
    {
      if (start < 0)
        return BasicString<CharType>();
      
      if (start >= size)
        return BasicString<CharType>();
      
      int length = size - start;
      
      return BasicString<CharType>(&buf[start], length);
    }
    
    BasicString<CharType> left(int length) const
    {
      if (length <= 0)
        return BasicString<CharType>();
      
      if (length > size)
        length = size;
      
      return BasicString<CharType>(buf, length);
    }
    
    BasicString<CharType> right(int length) const
    {
      if (length <= 0)
        return BasicString<CharType>();
      
      if (length > size)
        length = size;
      
      int start = size - length;
      
      return BasicString<CharType>(&buf[start], length);
    }
    
    int find(const BasicString<CharType> &str, int start=0) const
    {
      if (str.size > size) return -1;
      if (start < 0) start = 0;
      
      for (int b=start; b<=size-str.size; ++b) {
        
        bool found = true;
        
        for (int s=0; s<str.size; ++s)
          if (buf[b+s] != str.buf[s])
            {found = false; break;}
        
        if (found) return b;
      }
      
      return -1;
    }
    
    int findRev(const BasicString<CharType> &str, int start=-1) const
    {
      if (str.size > size) return -1;
      if (start < 0) start = size - str.size;
      if (start > size-str.size) start = size - str.size;
      
      for (int b=start; b>=0; --b) {
        
        bool found = true;
        
        for (int s=0; s<str.size; ++s)
          if (buf[b+s] != str.buf[s])
            {found = false; break;}
        
        if (found) return b;
      }
      
      return -1;
    }
    
    BasicString<CharType>& findReplace(const BasicString<CharType> &strFind,
                                       const BasicString<CharType> &strReplace)
    {
      int start = find(strFind);
      
      while (start != -1) {
        replace(start, strFind.size, strReplace);
        start = find(strFind, start + strReplace.size);
      }
      
      return *this;
    }

    void tokenize( const BasicString<CharType> &separator,
      ArrayList<BasicString<CharType> > *list ) const
    {
      if (separator == "")
        return;
      
      int start = 0;
      int end = find(separator);
      
      while (end != -1) {
        
        if (end > 0)
          list->pushBack(sub(start, end - start));
        
        start = end + separator.size;
        end = find(separator, start);
      }
      
      if (start < size)
        list->pushBack(sub(start, size - start));
    }
    
    BasicString<CharType>& trim(CharType trimchar=' ')
    {
      int start = 0;
      int end = 0;
      
      for (start=0; start<size; ++start)
        if (buf[start] != trimchar)
          break;
      
      for (end=size-1; end>=start; --end)
        if (buf[end] != trimchar)
          break;
      
      *this = sub(start, end-start+1);
      return *this;
    }
    
    BasicString<char> toCSTR () const
    {
      BasicString<char> chars (size+1);
      
      for (int c=0; c<size; ++c)
        chars.buf[c] = (char)buf[c];
      
      chars.buf [size] = 0;
      chars.size = size+1;
      
      return chars;
    }
    
    void fromCSTR (const char *str)
    {
      assign (str);
    }
    
    int lengthUTF8() const
    {
      Unicode ucs4Range[] = {
        0x0000007F,
        0x000007FF,
        0x0000FFFF,
        0x001FFFFF,
        0x03FFFFFF,
        0x7FFFFFFF
      };
      
      int utf8len = 0;
      
      //check codes of each char
      for (int c=0; c<size; c++) {
        Unicode code = (Unicode)buf[c];
        //find which range does it fits into
        for (int i=0; i<6; i++)
          if (code < ucs4Range[i])
            {utf8len += i+1; break;}
      }
      
      return utf8len;
    }

    void toUTF8(char *outbuf, int maxsize) const
    { 
      Unicode ucs4Range[] = {
        0x0000007F,
        0x000007FF,
        0x0000FFFF,
        0x001FFFFF,
        0x03FFFFFF,
        0x7FFFFFFF
      };
      
      int firstControlBits[] = {
        0x00, 0xC0, 0xE0,
        0xF0, 0xF8, 0xFC
      };
      int nextControlBits = 
        0x80;
      
      int firstDataMask[] = {
        0xFF,	0x1F, 0x0F,
        0x07, 0x03, 0x01,
      };
      int nextDataMask =
        0x3F;
      
      char *bufp = outbuf;
      int cursize = 0;
      
      //encode characters
      for (int c=0; c<size; c++) {
        
        Unicode code = (Unicode)buf[c];
        int utf8len = 0;
        
        //how many UTF octets this char has?
        for (int i=0; i<6; i++)
          if (code < ucs4Range[i])
            {utf8len = i+1; break;}
        
        //check if size too large
        cursize += utf8len;
        if (cursize > maxsize)
          return;
        
        //for each UTF octet (left to right)
        for (int b=0; b<utf8len; b++) {
          
          //which UTF octet are we filling
          //looking from right to left?
          int right2leftIndex = (utf8len-1-b);
          //since each UTF octet (except first)
          //takes 6 data bits, which data bit
          //is going to be next filed?
          int dataBitIndex = right2leftIndex*6;
          
          if (b==0) {
            //UTF control bits
            bufp[b] = firstControlBits[utf8len-1];
            //UCS data bits
            bufp[b] |= (code & (firstDataMask[utf8len-1] << dataBitIndex)) >> dataBitIndex;
          }else{
            //UTF control bits
            bufp[b] = nextControlBits;
            //UCS data bits
            bufp[b] |= (code & (nextDataMask << dataBitIndex)) >> dataBitIndex;
          }
        }
        
        //increase buffer pointer
        bufp+=utf8len;
      }
    }
  
    void fromUTF8(const char *utf8, int bufsize)
    {
      int firstDataMask[] = {
        0xFF,	0x1F, 0x0F,
        0x07, 0x03, 0x01,
      };
      int nextDataMask =
        0x3F;
      
      char *bufp = (char*)utf8;
      
      //clear string
      clear();
      setCapacity(bufsize);
      
      //go thru entire buffer
      int doneBytes = 0;
      while (doneBytes < bufsize) {
        
        //how many UTF octets does
        //this char have?
        int utf8len = 0;
        //is this an ASCII char?
        if (!(*bufp & (1 << 7))) {
          //yes - we only got 1 octet
          utf8len = 1;
        }else{
          //no - measure number of 1 bits
          for (int i=7; i>=0; i--) {
            if (*bufp & (1 << i))
              utf8len++;
            else break;
          }
        }
        
        //safety-case
        if (utf8len == 0) return;
        if (*bufp == 0) return;
        
        
        //Unicode char code
        int code = 0;
        
        //for each UTF octet (left to right)
        for (int b=0; b<utf8len; b++) {
          
          //safety-check for buffer-overrun
          if (++doneBytes > bufsize) break;
          
          //which UTF octet are we filling
          //looking from right to left?
          int right2leftIndex = (utf8len-1-b);
          //since each UTF octet (except first)
          //takes 6 data bits, which data bit
          //is going to be next filed?
          int dataBitIndex = right2leftIndex*6;
          
          if (b==0){
            //UCS data bits
            code |= (bufp[b] & firstDataMask[utf8len-1]) << dataBitIndex;
          }else{
            //UCS data bits
            code |= (bufp[b] & nextDataMask) << dataBitIndex;
          }
        }
        
        //add char to string
        *this += code;
        
        //increase buffer pointer
        bufp += utf8len;
      }
    }
  
    void flipBytes (char *ptr, int size) {
      int max = size/2 + (size % 2);
      for (int b=0; b<max; b++) {
        char tmp = ptr[b];
        ptr[b] = ptr[size-1-b];
        ptr[size-1-b] = tmp;
      }
    }
    
    bool findIsLilEndian ()
    {
      unsigned int a = 1;
      return ( ((unsigned char*)&a)[0] == 1 );
    }
  
    int lengthUTF16 () const
    {
      int outsize = 0;
      
      //find the size of result
      for (int i=0; i<size; ++i) {
        Unicode code = (Unicode)buf[i];
        if (code==0xFFFE || code==0xFFFF) continue;
        if (code>=0xD800 && code<=0xDFFF) continue;
        if (code>=0x110000) continue;
        outsize += (code <= 0xFFFD ? 2 : 4);
      }
      
      //endianness sign
      outsize += 2;
      
      return outsize;
    }
    
    void toUTF16 (char *outbuf, int maxsize) const
    { 
      char *bufp = outbuf + 2;
      int cursize = 0;
      bool isLilEndian = findIsLilEndian();
      
      //mark the endianness
      if( isLilEndian ){
        outbuf[0] = 0xFF;
        outbuf[1] = 0xFE;
      }else{
        outbuf[0] = 0xFE;
        outbuf[1] = 0xFF;
      }
      
      //traverse characters
      for (int c=0; c<size; c++) {
        
        //check for code range
        Unicode code = (Unicode)buf[c];
        if (code==0xFFFE || code==0xFFFF) continue;
        if (code>=0xD800 && code<=0xDFFF) continue;
        if (code>=0x110000) continue;
        if (code <= 0xFFFD) {
          
          //check if size too large
          cursize += 2;
          if (cursize > maxsize)
            return;
          
          //write out the code
          short scode = code;
          *bufp = ((char*)&scode)[0]; bufp++;
          *bufp = ((char*)&scode)[1]; bufp++;
          
        }else{
          
          //check if size too large
          cursize += 4;
          if (cursize > maxsize)
            return;
          
          //strip off the base
          code -= 0x10000;
          //separate in the high and low 10 bits
          short w1 = (code & 0xFFC00) >> 10;
          short w2 = (code & 0x3FF);
          //combine with word bases
          w1 = w1 | 0xD800;
          w2 = w2 | 0xDC00;
          
          //write out the codes
          *bufp = ((char*)&w1)[0]; bufp++;
          *bufp = ((char*)&w1)[1]; bufp++;
          *bufp = ((char*)&w2)[0]; bufp++;
          *bufp = ((char*)&w2)[1]; bufp++;
        }
      }
    }
  
    void fromUTF16 (const char *utf16, int bufsize)
    {
      Byte *bufp = (Byte*)utf16;
      int doneBytes = 0;
      Unicode code = 0;
      
      //check byte order
      bool isLilEndian = findIsLilEndian();
      bool isSourceLilEndian = false;
      if (bufp[0] == 0xFF && bufp[1] == 0xFE) {
        //little endian byte order
        isSourceLilEndian = true;
        bufp += 2; doneBytes += 2;
      }else if(bufp[0] == 0xFE && bufp[1] == 0xFF) {
        //big endian byte order
        isSourceLilEndian = false;
        bufp += 2; doneBytes = 2;
      }else{
        //assume order native to system
        isSourceLilEndian = isLilEndian;
      }
      
      //traverse input buffer
      while (doneBytes != bufsize) {
        
        //check if alignment OK
        if (bufsize - doneBytes == 1)
          break;
        
        //pick first word
        unsigned short s1;
        Byte *b1 = (Byte*)&s1;
        if (isSourceLilEndian == isLilEndian)
          {b1[0]=bufp[0]; b1[1]=bufp[1];}
        else
          {b1[0]=bufp[1]; b1[1]=bufp[0];}
        
        //check for code range
        if (s1==0xFFFF || s1==0xFFFE)
          {bufp+=2; doneBytes+=2; continue;}
        if (s1>=0xDC00)
          {bufp+=2; doneBytes+=2; continue;}
        if (s1>=0xD800 && s1<=0xDBFF) {
          
          //check if alignment OK
          if (bufsize - doneBytes < 4)
            break;
          
          //pick second word
          unsigned short s2;
          Byte *b2 = (Byte*)&s2;
          if (isSourceLilEndian == isLilEndian)
            {b2[0]=bufp[2]; b2[1]=bufp[3];}
          else
            {b2[0]=bufp[3]; b2[1]=bufp[2];}
          
          //check for code range
          if (!(s2>=0xDC00 && s2<=0xDFFF))
            {bufp+=2; doneBytes+=2; continue;}
          
          //decode
          code = (s1 & 0x3FF) << 10 + 
            (s2 & 0x3FF) +
            0x10000;
          
          //increase buf pointer
          bufp+=4; doneBytes+=4;
          
        }else{
          
          //just copy value
          code = s1;
          
          //increase buf pointer
          bufp+=2; doneBytes+=2;
        }
        
        //add code to string
        *this += code;
        
      }//end::traverse buffer
    }//end::function
  
    BasicString<Byte> toUTF8() const
    {
      int len = lengthUTF8();
      BasicString<Byte> out(len);
      toUTF8((char*)out.buf, len);
      out.size = len;
      return out;
    }
  
    void fromUTF8(const BasicString<Byte> &utf8)
    {
      fromUTF8((char*)utf8.buf, utf8.size);
    }
  
    BasicString<Byte> toUTF16() const
    {
      int len = lengthUTF16();
      BasicString<Byte> out(len);
      toUTF16((char*)out.buf, len);
      out.size = len;
      return out;
    }
  
    void fromUTF16(const BasicString<Byte> &utf16)
    {
      fromUTF8((char*)utf16.buf, utf16.size);
    }
  
    static BasicString<Unicode> FromUTF8(const char *utf8, int bufsize)
    {
      BasicString<Unicode> str;
      str.fromUTF8(utf8, bufsize);
      return str;
    }
  
    static BasicString<Unicode> FromUTF16(const char *utf16, int bufsize)
    {
      BasicString<Unicode> str;
      str.fromUTF16(utf16, bufsize);
      return str;
    }
    
    static BasicString<Unicode> FromUTF8(const BasicString<Byte> &utf8)
    {
      BasicString<Unicode> str;
      str.fromUTF8(utf8);
      return str;
    }
    
    static BasicString<Unicode> FromUTF16(const BasicString<Byte> &utf16)
    {
      BasicString<Unicode> str;
      str.fromUTF16(utf16);
      return str;
    }
    
  private:
    
    /*
    Internal parsing functions work on given C string
    */
    int __parseHexAt (const char *str, int start, int *length=NULL) const
    {
      const char *startPtr = &str [start];
      char *endPtr = (char*) startPtr;
      int out = strtol (startPtr, &endPtr, 16);
      if (length != NULL) *length = (int) (endPtr - startPtr);
      return out;
    }
    
    int __parseIntegerAt (const char *str, int start, int *length=NULL) const
    {
      const char *startPtr = &str [start];
      char *endPtr = (char*) startPtr;
      int out = strtol (startPtr, &endPtr, 10);
      if (length != NULL) *length = (int) (endPtr - startPtr);
      return out;
    }
    
    float __parseFloatAt (const char *str, int start, int *length=NULL) const
    {
      const char *startPtr = &str [start];
      char *endPtr = (char*) startPtr;
      float out = (float) strtod (startPtr, &endPtr);
      if (length != NULL) *length = (int) (endPtr - startPtr);
      return out;
    }
    
    /*
    Specialized overloads of parsing functions to route the unsigned char*
    type strings down the fast path instead of doing the conversion to char*.
    This is safe since parsing functions only check for ASCII range characters
    which have the same bit values in both signed and unsigned char.
    */
    int __parseHexAt (const unsigned char *str, int start, int *length=NULL) const
    {
      return __parseHexAt ((const char*) str, start, length);
    }
    
    int __parseIntegerAt (const unsigned char *str, int start, int *length=NULL) const
    {
      return __parseIntegerAt ((const char*) str, start, length);
    }
    
    float __parseFloatAt (const unsigned char *str, int start, int *length=NULL) const
    {
      return __parseFloatAt ((const char*) str, start, length);
    }
    
    /*
    General purpose parsing functions for any other character type that
    convert to char* first
    */
    
    template <class AnyCharType>
    int __parseHexAt (const AnyCharType *str, int start, int *length=NULL) const
    {
      BasicString<char> chars (size);
      chars.assign (str, size);
      return __parseHexAt (chars.buf, start, length);
    }
    
    template <class AnyCharType>
    int __parseIntegerAt (const AnyCharType *str, int start, int *length=NULL) const
    {
      BasicString<char> chars (size);
      chars.assign (str, size);
      return __parseIntegerAt (chars.buf, start, length);
    }
    
    template <class AnyCharType>
    float __parseFloatAt (const AnyCharType *str, int start, int *length=NULL) const
    {
      BasicString<char> chars (size);
      chars.assign (str, size);
      return __parseFloatAt (chars.buf, start, length);
    }
    
  public:
    
    /*
    Public parsing functions resolve to fastest possible path based
    on the string's character type.
    */
    int parseHexAt (int start, int *length=NULL) const
    {
      return __parseHexAt (buf, start, length);
    }
    
    int parseIntegerAt (int start, int *length=NULL) const
    {
      return __parseIntegerAt (buf, start, length);
    }
    
    float parseFloatAt (int start, int *length=NULL) const
    {
      return __parseFloatAt (buf, start, length);
    }
    
    int parseHex() const
    {
      return __parseHexAt (buf, 0, NULL);
    }
    
    int parseInteger() const
    {
      return __parseIntegerAt (buf, 0, NULL);
    }
    
    float parseFloat() const
    {
      return __parseFloatAt (buf, 0, NULL);
    }
    
    static BasicString<CharType> FromHex(int value)
    {
      int len = snprintf(NULL, 0, "%x", value);
      char *buf = new char[len+2];
      memset(buf, 0, len+2);
      snprintf(buf, len+1, "%x", value);
      BasicString<CharType> out(buf);
      delete[] buf;
      return out;
    }
    
    static BasicString<CharType> FromInteger(int value)
    {
      int len = snprintf(NULL, 0, "%d", value);
      char *buf = new char[len+2];
      memset(buf, 0, len+2);
      snprintf(buf, len+1, "%d", value);
      BasicString<CharType> out(buf);
      delete[] buf;
      return out;
    }
    
    static BasicString<CharType> FromFloat(float value)
    {
      int len = snprintf(NULL, 0, "%f", value);
      char *buf = new char[len+2];
      memset(buf, 0, len+2);
      snprintf(buf, len+1, "%f", value);
      BasicString<CharType> out(buf);
      delete[] buf;
      return out;
    }

    static BasicString<CharType> FHex (int value) {
      return FromHex( value );
    }

    static BasicString<CharType> FInt (int value) {
      return FromInteger( value );
    }

    static BasicString<CharType> FFloat (float value) {
      return FromFloat( value );
    }
    
    void format(char *format, ...)
    {
      va_list ap;
      va_start(ap, format);

      int len = vsnprintf(NULL, 0, format, ap);
      char *buf = new char[len+2];
      memset(buf, 0, len+2);
      vsnprintf(buf, len+1, format, ap);
      
      *this = buf;
      delete[] buf;
      
      va_end(ap);
    }
    
    void formatV(char *format, va_list ap)
    {
      int len = vsnprintf(NULL, 0, format, ap);
      char *buf = new char[len+2];
      memset(buf, 0, len+2);
      vsnprintf(buf, len+1, format, ap);
      
      *this = buf;
      delete[] buf;
    }
    
    static BasicString<CharType> Format(char *format, ...)
    {
      va_list ap;
      va_start(ap, format);
      
      BasicString<CharType> str;
      str.formatV(format, ap);
      
      va_end(ap);
      return str;
    }
    
    static BasicString<CharType> FormatV(char *format, va_list ap)
    {
      BasicString<CharType> str;
      str.formatV(format, ap);
      return str;
    }
  };

  template <class CharType>
  BasicString<CharType> operator+ (const char *cstr, const BasicString<CharType> &str)
  {
    return BasicString<CharType>( cstr ) + str;
  }
  
  typedef BasicString<char> CharString;
  typedef BasicString<Byte> ByteString;
  typedef BasicString<Unicode> String;

}//namespace GE
#endif//__GESTRING_H
