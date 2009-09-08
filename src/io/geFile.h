#ifndef _FILE_SYSTEM_INCLUDED
#define _FILE_SYSTEM_INCLUDED

#include "util/geUtil.h"

namespace GE
{
  namespace FileAccess {
    enum Enum {
      Read,
      Write,
      ReadWrite
    };
  }

  namespace FileCondition {
    enum Enum {
      None               = (0     ),
      MustExist          = (1     ),
      MustNotExist       = (1 << 1),
      Truncate           = (1 << 2)
    };
  }

  namespace FileSeekOrigin {
    enum Enum {
      Start,
      Current,
      End
    };
  }

  class File
  {
  protected:
    bool lilend;
    String name;
    String path;
    void findEndian();

  #if defined(WIN32)
    HANDLE handle;
    #define GE_INVALID_FILE_HANDLE  INVALID_HANDLE_VALUE
  #else
    FILE* handle;
    #define GE_INVALID_FILE_HANDLE  NULL
  #endif
    
    bool isPathAbs (const String &path);
    String makePathNative (const String &path);
    bool createDirectory (const String &p);

  public:
    File();
    File( const File &f );
    File( const String &path );
    ~File();
    
    File& operator=( const File &f );

    #if defined(WIN32)
    HANDLE getHandle() { return handle; }
    #else
    FILE* getHandle() { return handle; }
    #endif
    
    String getName(bool withExtension=true);
    String getPath();
    String getPathName();
    String getExtension();
    UintSize getSize();
    String getRelationTo( const File &f );
    File getRelativeFile( const String &relation );
    File getSuperFile();
    void getSubFiles( ArrayList<File> *outFiles );

    bool isRoot();
    bool isOpen();
    bool isDirectory();
    bool exists();
    bool remove();
    bool rename( const String &fullpath );

    bool createPath();
    bool open (FileAccess::Enum access, FileCondition::Enum condition);
    void close();

    bool setPointer (FileSeekOrigin::Enum origin, UintSize distance);
    UintSize getPointer ();
    
    UintSize read( void *data, UintSize size );
    UintSize write( const void *data, UintSize size );
    
    ByteString read( UintSize size );
    UintSize read( ByteString &str, UintSize size );
    UintSize write( const ByteString &str );
    
    static File GetModule();
    static File GetHome();
  };
}

#endif
