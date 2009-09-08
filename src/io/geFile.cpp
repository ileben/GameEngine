#include "util/geUtil.h"
#include "io/geFile.h"

//External headers
#include <sys/stat.h>

#if defined (WIN32)
#  include <direct.h> //_getcwd
#  define getcwd _getcwd
#  define access _access
#  define stat _stat
#  define F_OK 0
#else
#  include <dirent.h> //opendir
#endif

#if defined(__APPLE__)
#  include <Carbon/Carbon.h>
#endif

namespace GE
{
  void flipBytes( void *in, int size )
  {
    unsigned char *a = (unsigned char*)in;
    int b; unsigned char temp;
    int hsize = (size + (size & 0x1)) / 2;
    for (b=0; b<hsize; ++b) {
      temp = a[b];
      a[b] = a[size-b-1];
      a[size-b-1] = temp;
    }
  }

#if defined(WIN32)

  bool File::isPathAbs (const String &p)
  {
    return (p.sub(1,2) != ":/");
  }

  String File::makePathNative (const String &p)
  {
    return p.sub(1).findReplace("/", "\\");
  }

  UintSize File::getSize()
  {
    if (!isOpen()) return 0;

    LARGE_INTEGER size;
    GetFileSizeEx( handle, &size );
    
    return (UintSize) size.QuadPart;
  }

  bool File::exists()
  {
    CharString longpath = "\\\\?\\" + getPathName();
    DWORD atts = GetFileAttributes( longpath.buffer() );
    return atts != INVALID_FILE_ATTRIBUTES;
  }

  bool File::isDirectory()
  {
    CharString longpath = "\\\\?\\" + getPathName();
    DWORD atts = GetFileAttributes( longpath.buffer() );
    return (atts & FILE_ATTRIBUTE_DIRECTORY) != 0;
  }

  bool File::createDirectory (const String &p)
  {
    CharString longpath = "\\\\?\\" + p;
    return (CreateDirectory( longpath.buffer(), NULL ) == TRUE);
  }

  bool File::open (FileAccess::Enum access, FileCondition::Enum condition)
  {
    DWORD winAccess = 0;
    DWORD winCondition = 0;

    //Setup access permissions
    switch (access) {
    case FileAccess::Read:
      winAccess = GENERIC_READ;
      break;
    case FileAccess::Write:    
      winAccess = GENERIC_WRITE;
      break;
    case FileAccess::ReadWrite:
      winAccess = GENERIC_READ | GENERIC_WRITE;
      break;
    }

    //Setup creation disposition
    if (condition & FileCondition::MustNotExist) {
      winCondition = CREATE_NEW;
    }
    else if (condition & FileCondition::MustExist) {
      if (condition & FileCondition::Truncate)
        winCondition = TRUNCATE_EXISTING;
      else winCondition = OPEN_EXISTING;
    }
    else {
      if (condition & FileCondition::Truncate)
        winCondition = CREATE_ALWAYS;
      else winCondition = OPEN_ALWAYS;
    }

    //Create the file
    CharString longpath = "\\\\?\\" + getPathName();
    handle = CreateFile( longpath.buffer(), winAccess, 0, NULL, winCondition, 0, NULL );
    return (handle != INVALID_HANDLE_VALUE);
  }

  void File::close()
  {
    if (handle != INVALID_HANDLE_VALUE) {
      CloseHandle( handle );
      handle = INVALID_HANDLE_VALUE;
    }
  }

  void File::getSubFiles( ArrayList<File> *outFiles )
  {
    String SLASH = "/";		
    String pathname = path+name;
    
    //Windows doesn't have a unique API
    //for searching root "disk" files
    //and regular files!
    
    if( pathname == SLASH )
    {
      //"Disk" files
      //--------------------------------
      
      char volumes[256];
      char *vol;
      
      //find all the "disk" files
      if( GetLogicalDriveStrings( 256, volumes ) == 0 ){
        printf( "failed getting logical drives\n" );
        return;
      }
      
      //add every "disk" files
      for( vol = volumes; *vol != '\0'; vol += strlen(vol)+1 )
      {
        File f;
        f.name = ((String)vol).sub( 0,2 );
        f.path = SLASH;
        outFiles->pushBack( f );
      }
    }
    else
    {
      //"Regular" files
      //--------------------------------

      UintSize nextDirIndex = outFiles->size();

      //generate search string
      String findPath = pathname.sub(1) + "\\*";
      
      //obtain FindData handle
      WIN32_FIND_DATA findData;
      HANDLE findHandle = FindFirstFile( findPath.toCSTR().buffer(), &findData );
      
      if( findHandle == INVALID_HANDLE_VALUE ){
        printf( "failed opening dir '%s'\n", pathname.toCSTR().buffer() );
        return;
      }
      
      //if we are searching in a "disk" dir
      //the ".." won't be given by Windows,
      //so add it manually
      if( path == SLASH )
      {
        File f;
        f.name = "..";
        f.path = pathname + SLASH;
        outFiles->insertAt( nextDirIndex, f );
        nextDirIndex++;
      }
			
      //find all the files inside this dir
      while( FindNextFile( findHandle, &findData ))
      {
        String s = findData.cFileName;
        if( s == "." )
          continue;
        if( s == ".." && name == SLASH )
          continue;
        
        File f;
        f.name = s;
        f.path = (name==SLASH ? pathname : pathname+SLASH);
        
        if( findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY ){
          //insert directories at the beginning
          outFiles->insertAt( nextDirIndex, f );
          nextDirIndex++;
        }else{
          //add other files at the end
          outFiles->pushBack( f );
        }
      }
      
      //close FindData handle
      FindClose( findHandle );
    }
  }

  bool File::remove()
  {
    CharString longpath = "\\\\?\\" + getPathName();
    
    if (isDirectory())
      return (RemoveDirectory( longpath.buffer() ) == TRUE);
    else
      return (DeleteFile( longpath.buffer() ) == TRUE);
  }
  
  bool File::rename (const String &name)
  {
    File newfile( name );
    CharString oldlongpath = "\\\\?\\" + getPathName();
    CharString newlongpath = "\\\\?\\" + newfile.getPathName();
    return (MoveFile( oldlongpath.buffer(), newlongpath.buffer() ) == TRUE);
  }
	
  UintSize File::read (void *data, UintSize size)
  {
    DWORD numRead = 0;
    ReadFile( handle, data, (DWORD) size, &numRead, NULL );
    return (UintSize) numRead;
  }
  
  UintSize File::write (const void *data, UintSize size)
  {
    DWORD numWritten = 0;
    WriteFile( handle, data, (DWORD) size, &numWritten, NULL );
    return (UintSize) numWritten;
  }

  bool File::setPointer (FileSeekOrigin::Enum origin, UintSize distance)
  {
    DWORD winOrigin = 0;
    
    switch (origin) {
    case FileSeekOrigin::Start: winOrigin = FILE_BEGIN; break;
    case FileSeekOrigin::Current: winOrigin = FILE_CURRENT; break;
    case FileSeekOrigin::End: winOrigin = FILE_END; break;
    }

    LARGE_INTEGER winDist;
    winDist.QuadPart = (LONGLONG) distance;
    return (SetFilePointerEx( handle, winDist, NULL, winOrigin ) == TRUE);
  }
  
  UintSize File::getPointer()
  {
    LARGE_INTEGER winPos;
    LARGE_INTEGER winDist;
    winDist.QuadPart = 0;
    SetFilePointerEx( handle, winDist, &winPos, FILE_CURRENT );
    return (UintSize) winPos.QuadPart;
  }
  

#else

  bool File::isPathAbs (const String &p)
  {
    return (p.left(1) != "/")
  }

  String File::makePathNative (const String &p)
  {
    return p;
  }

  UintSize File::getSize();
  {
    if (!isOpen()) return 0;

    struct stat fs;
    CharString pathname = getPathName();
    if (stat(pathname.buffer(), &fs) == 0)
      return fs.st_size;
    else return 0;
  }

  bool File::isDirectory()
  {
    struct stat fs;
    CharString pathname = getPathName();
    if (stat(pathname.buffer(), &fs) == 0)
      return (fs.st_mode & S_IFDIR) != 0;
    else return false;
  }

  bool File::exists()
  {
    CharString pathname = getPathName();
    return access( pathname.buffer(), F_OK ) == 0;
  }

  bool File::createDirectory (const String &p)
  {
    return (mkdir( p.toCSTR().buffer(), 0x0777 ) != 0)
  }

  bool File::open (FileAccess::Enum access, FileCondition::Enum condition)
  {
    CharString unixAccess;

    switch (access)
    {
    case FileAccess::Read:
      unixAccess = "rb";
      break;

    case FileAccess::Write:
      
      if (condition & FileCondition::Truncate
        unixAccess = "wb";
      else
        unixAccess = "ab";
      break;

    case FileAccess::ReadWrite:
      
      if (condition & FileCondition::Truncate)
        unixAccess = "wb+";
      else
        unixAccess = "ab+";
      break;
    }

    CharString pathname = getPathName();
    handle = fopen( pathname.buffer(), unixAccess.buffer() );
    return (handle != NULL);
  }

  
  void File::close()
  {
    if (handle != NULL) {
      fclose(handle);
      handle = NULL;
    }
  }

  void File::getSubFiles( ArrayList<File> *outFiles )
  {
    String SLASH = "/";		
    String pathname = path+name;
    
    //open searching directory
    DIR *newDir = opendir( pathname.toCSTR().buffer() );
    if( newDir == NULL ){
      printf( "failed opening dir '%s'\n", pathname.toCSTR().buffer() );
      return;
    }
    
    dirent *ent;
    int nextDirIndex = outFiles->size();
		
    //find all the files inside this dir
    while( (ent = readdir( newDir )) != NULL )
    {
      String s = ent->d_name;
      if( s == "." )
        continue;
      if( s == ".." && name == SLASH )
        continue;
      
      File f;
      f.name = s;
      f.path = (name==SLASH ? pathname : pathname+SLASH);
      
      if( ent->d_type == DT_DIR ){
        //insert directories at the beginning
        outFiles->insertAt( nextDirIndex, f );
        nextDirIndex++;
      }else{
        //add other files at the end
        outFiles->pushBack( f );
      }
    }
    
    //close searching directory
    closedir( newDir );
  }

  bool File::remove()
  {
    CharString pathname = getPathName();
    return (::remove( pathname.buffer() ) == 0)
  }
  
  bool File::rename(const String &name)
  {
    File newFile(name);
    CharString oldName = getPathName();
    CharString newName = newFile.getPathName();
    return (::rename( oldName.buffer(), newName.buffer() ) == 0)
  }
  
  UintSize File::read (void *data, UintSize size)
  {
    return fread( data, 1, size, handle );
  }
  
  UintSize File::write (const void *data, UintSize size)
  {
    return fwrite( data, 1, size, handle );
  }

  bool File::setPointer (FileSeekOrigin::Enum origin, UintSize distance)
  {
    int unixOrigin = 0;
    
    switch (origin) {
    case FileSeekOrigin::Start: unixOrigin = SEEK_SET; break;
    case FileSeekOrigin::Current: unixOrigin = SEEK_CUR; break;
    case FileSeekOrigin::End: unixOrigin = SEEK_END; break;
    }

    return (fseek( handle, (long int)offset, unixOrigin) == 0);
  }
  
  UintSize File::getPointer()
  {
    return (UintSize) ftell( handle );
  }

#endif


  void File::findEndian()
  {
    unsigned int a = 1;
    lilend = ( ((unsigned char*)&a)[0] == 1 );
  }
  
  File::File()
  {
    handle = GE_INVALID_FILE_HANDLE;
    findEndian();
  }

  File::File( const File &f )
  {
    lilend = f.lilend;
    name = f.name;
    path = f.path;
    handle = f.handle;
  }

  File& File::operator=( const File &f )
  {
    lilend = f.lilend;
    name = f.name;
    path = f.path;
    handle = f.handle;
    return *this;
  }
  
  File::File( const String &newPath )
  {
    handle = GE_INVALID_FILE_HANDLE;
    findEndian();

    ArrayList<String> pathNames;
    ArrayList<String> truePath;

    //Make all forward-slashes
    String SLASH = "/";
    path = newPath;
    path.findReplace( "\\", SLASH );

    //Break the folder/file names apart
    //-------------------------------------
    if (isPathAbs( path ))
    {
      //Try finding current working directory
      char *cwd = getcwd( NULL, 0 );
      if( cwd == NULL )
        { path=""; name=""; return; }

      String strCwd = cwd;
      free( cwd );
      
      //Tokenize working dir names
      strCwd.findReplace( "\\", SLASH );
      strCwd.tokenize( SLASH, &pathNames );
    }
    
    //Tokenize given path names
    path.tokenize( SLASH, &pathNames );
    

    //Resolve '..'s and '.'s
    //-----------------------------------
    for( UintSize s=0; s<pathNames.size(); s++ )
    {
      String &name = pathNames[ s ];
      
      //remove previous dir on ".."
      if( name == ".." ){
        if( truePath.size() > 0 )
          truePath.popBack();
        continue;
      }
      
      //add dir if not "."
      if( name != "." && name != "" )
        truePath.pushBack( name );
    }
    
    //Compose final path and name
    //---------------------------------
    if( truePath.size() == 0 ){
      
      path = "";
      name = "/";
      
    }else{
      
      path = "/";
      for( UintSize t=0; t<truePath.size()-1; t++ )
        path += truePath[ t ] + "/";
      name = truePath.last();
    }
  }
  
  File::~File()
  {
  }

  String File::getPath()
  {
    return makePathNative( path );
  }
  
  String File::getName (bool withExtension)
  {
    if (withExtension) return name;
    int dot = name.findRev( "." );
    if (dot == -1) return name;
    return name.left( dot );
  }

  String File::getExtension ()
  {
    int dot = name.findRev( "." );
    if (dot == -1) return "";
    return name.sub( dot );
  }
  
  String File::getPathName()
  {
    return getPath() + getName();
  }

  bool File::isOpen()
  {
    return (handle != GE_INVALID_FILE_HANDLE);
  }
  
  bool File::isRoot()
  {
    return name == (String)"/";
  }
  
  String File::getRelationTo( const File &f )
  {
    int x = 0, y = 0;
    String out;
    
    //get "/" after last common dir
    while (true) {
      
      int newx = f.path.find("/", x+1);
      if (newx == -1) break;
      String namex = f.path.sub(x+1, newx-x-1);
      
      int newy = path.find("/", y+1);
      if (newy == -1) break;
      String namey = path.sub(y+1, newy-y-1);
      
      if (namex != namey) break;
      x = newx; y = newy;
    }
    
    //add "../" for each different dir of [f]
    while (true) {
      
      int newx = f.path.find("/", x+1);
      if (newx == -1) break;
      out += "../";
      x = newx;
    }
          
    //add names for each different dir of [this]
    out += path.sub(y+1);
    out += name;
    
    #if defined( WIN32 )
    out.findReplace( "/", "\\" );
    #endif
    
    return out;
  }
  
  File File::getRelativeFile( const String &relation )
  {
    String unixrel = relation;
    unixrel.findReplace( "\\", "/" );
    
    if (isPathAbs( unixrel ))
      return File( relation );
    else return File( getPath() + relation );
  }
  
  File File::getSuperFile()
  {
    //Check if current file is root
    if (isRoot())
      return File( "/" );
    
    //Check if parent folder is root
    if (path == "/")
      return File( "/" );
    
    //Take only path and use it as full name
    return File( makePathNative( path ) );
  }

  bool File::createPath()
  {
    //Find first existing directory in the path
    ArrayList<String> dirs;
    File dir( getPath() );
    while (!dir.exists())
    {
      //Stop if root reached
      if (dir.isRoot()) return false;
      dirs.pushBack( dir.getPathName() );
      dir = dir.getSuperFile();
    }

    //Create all the missing directories
    for (UintSize d=0; d<dirs.size(); ++d)
      if (!createDirectory( dirs[d] ))
        return false;

    return true;
  }
  
  ByteString File::read (UintSize size)
  {
    ByteString out( (int)size );
    UintSize r = read( out.buf, size );
    out.size = (int) r;
    return out;
  }
  
  UintSize File::read (ByteString &str, UintSize size)
  {
    str.reserveAndCopy( str.length() + (int)size );
    UintSize r = read( str.buf, size );
    str.size += (int) size;
    return r;
  }
  
  UintSize File::write (const ByteString &str)
  {
    return write( str.buf, str.size );
  }
  
  File File::GetModule()
  {
    #if defined(WIN32)
    
    TCHAR buf[256];
    memset( buf, 0, 256 * sizeof(TCHAR) );
    ::GetModuleFileName( NULL, buf, 256 );
    
    return File( buf );
    
    #elif defined(__APPLE__)
    
    CFBundleRef bundle = CFBundleGetMainBundle();
    
    UInt32 pkgType = 0;
    UInt32 pkgCreator = 0;
    CFBundleGetPackageInfo(bundle, &pkgType, &pkgCreator);
    
    CFURLRef burl;
    switch (pkgType) {
    case 'APPL': burl = CFBundleCopyBundleURL(bundle); break;
    default: burl = CFBundleCopyExecutableURL(bundle);
    }
    
    char urlutf8[256];
    memset(urlutf8,0,256);
    CFURLGetFileSystemRepresentation(burl, true, (UInt8*)urlutf8, 255);
    String strPath = String::FromUTF8(urlutf8, strlen(urlutf8));
    
    return File( strPath );
    
    #else
    
    char procLink[64];
    pid_t pid = getpid();
    snprintf( procLink, 64, "/proc/%i/exe", pid );
    
    char exePath[1024];
    int pathLen = readlink( procLink, exePath, 1024 );
    if( pathLen == -1 ) return File();
    exePath[ pathLen ] = 0;
    
    return File( exePath );
    
    #endif
  }
  
  File File::GetHome()
  {
    #if defined(WIN32)
    return File::GetModule().getSuperFile();
    #else
    return File( getenv("HOME") );
    #endif
  }
}
