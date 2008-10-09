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
    int getSize();
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
    
    int read( void *data, int size, int count );
    int write( const void *data, int size, int count );
    
    int read( void *data, int size );
    int write( const void *data, int size );
    
    ByteString read( int size );
    int read( ByteString &str, int size );
    int write( const ByteString &str );
    
    int readLE( void *data, int size );
    int readBE( void *data, int size );
    int writeLE( const void *data, int size );
    int writeBE( const void *data, int size );
    
    bool seek( int offset, int whence );
    int tell();
    
    //statics
    static File GetModule();
    static File GetHome();
  };
}

#endif
