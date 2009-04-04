#ifndef _FILE_SYSTEM_INCLUDED
#define _FILE_SYSTEM_INCLUDED

namespace GE
{
  class File
  {
  protected:
    bool lilend;
    String name;
    String path;
    FILE *handle;
    void findEndian();
    
  public:
    File();
    File( const File &f );
    File( const String &path );
    ~File();
    
    File& operator=( const File &f );
    
    String& getName();
    String getPath();
    String getPathName();
    FILE* getHandle();
    UintSize getSize();
    bool isDirectory();
    bool isOpen();
    bool exists();
    
    bool isRoot();
    String getRelationTo( const File &f );
    File getRelativeFile( const String &relation );
    File getSuperFile();
    void getSubFiles( ArrayList<File> *outFiles );
    
    bool remove();
    bool rename( const String &fullpath );
    bool open( const String &mode );
    void close();
    
    UintSize read( void *data, UintSize size, int count );
    UintSize write( const void *data, UintSize size, int count );
    
    UintSize read( void *data, UintSize size );
    UintSize write( const void *data, UintSize size );
    
    ByteString read( UintSize size );
    UintSize read( ByteString &str, UintSize size );
    UintSize write( const ByteString &str );
    
    UintSize readLE( void *data, UintSize size );
    UintSize readBE( void *data, UintSize size );
    UintSize writeLE( const void *data, UintSize size );
    UintSize writeBE( const void *data, UintSize size );
    
    bool seek( int offset, int whence );
    int tell();
    
    //statics
    static File GetModule();
    static File GetHome();
  };
}

#endif
