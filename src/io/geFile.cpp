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
#endif

#if defined(__APPLE__)
	#include <Carbon/Carbon.h>
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

  void File::findEndian()
  {
    unsigned int a = 1;
    lilend = ( ((unsigned char*)&a)[0] == 1 );
  }
  
  File::File() : handle(NULL) {
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
  
  File::File( const String &newPath ) : handle(NULL)
  {
    findEndian();

    ArrayList<String> pathNames;
    ArrayList<String> truePath;

    //Make all forward-slashes
    String SLASH = "/";
    path = newPath;
    path.findReplace( "\\", SLASH );


    //Break the folder/file names apart
    //-------------------------------------
    #if defined( WIN32 )
    if( path.left(1) != SLASH && path.sub(1,2) != ":/" )
    #else
    if( path.left(1) != SLASH )
    #endif
    {
      //Try fiding current working directory
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
  
  String& File::getName()
  {
    return name;
  }
  
  String File::getPath()
  {
    #if defined(WIN32)
    return path.sub(1).findReplace("/", "\\");
    #else
    return path;
    #endif
  }
  
  String File::getPathName()
  {
    return getPath() + getName();
  }
  
  FILE* File::getHandle()
  {
    return handle;
  }
  
  int File::getSize()
  {
    String pathname = getPathName();
    
    struct stat fs;
    if (stat(pathname.toCSTR().buffer(), &fs) == 0)
      return fs.st_size;
    else
      return -1;
  }
  
  bool File::isDirectory()
  {
    String pathname = getPathName();
    
    struct stat fs;
    if (stat(pathname.toCSTR().buffer(), &fs) == 0)
      return (fs.st_mode & S_IFDIR) != 0;
    else
      return false;
  }
  
  bool File::isOpen()
  {
    return (handle != NULL);
  }
  
  bool File::exists()
  {
    #if defined( WIN32 )
    return false;
    #else
    String pathname = getPathName();
    return access(pathname.toCSTR().buffer(), F_OK) == 0;
    #endif
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
    
    #if defined(WIN32)
    bool isAbsolute = unixrel.sub( 1,2 ) == ":/";
    #else
    bool isAbsolute = unixrel.left( 1 ) == "/";
    #endif
    
    if (isAbsolute)
      return File( relation );
    else
      return File( getPath() + relation );
  }
  
  File File::getSuperFile()
  {
    //Check if current file is root
    if( name == (String)"/" )
      return File( "/" );
    
    //Check if parent folder is root
    if( path == (String)"/" ){
      
      //Root directory has no name but "/"
      return File( "/" );
      
    }else{
      
      //Skip root slash on windows to get native path
      #if defined(WIN32)
      return File( path.sub(1) );
      #else
      return File( path );
      #endif
    }
  }
  
  void File::getSubFiles( ArrayList<File> *outFiles )
  {
    String SLASH = "/";		
    String pathname = path+name;
    
    #if defined(WIN32)
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
    
    #else
    
    //open searching directory
    DIR *newDir = opendir(pathname.toCSTR().buffer());
    if (newDir == NULL){
      printf("failed opening dir '%s'\n", pathname.toCSTR().buffer());
      return files;
    }
    
    dirent *ent;
    int nextDirIndex=files->size();
		
    //find all the files inside this dir
    while ( (ent = readdir(newDir)) != NULL)
    {
      String s = ent->d_name;
      if (s == ".")
        continue;
      if (s == ".." && name == SLASH)
        continue;
      
      FileRef f = new File;
      f->name = s;
      f->path = (name==SLASH ? pathname : pathname+SLASH);
      
      if(ent->d_type == DT_DIR) {
        //insert directories at the beginning
        files->insertAt(nextDirIndex, f);
        nextDirIndex++;
      }else{
        //add other files at the end
        files->pushBack(f);
      }
    }
    
    //close searching directory
    closedir(newDir);
    #endif
  }
  
  bool File::remove()
  {
    if (::remove(getPathName().toCSTR().buffer()) == 0)
      return true;
    else
      return false;
  }
  
  bool File::rename(const String &name)
  {
    File fnew(name); 
    if (::rename(getPathName().toCSTR().buffer(),
                 fnew.getPathName().toCSTR().buffer()) == 0)
        return true;
    else
      return false;
  }
	
  bool File::open(const String &mode)
  {
    String f = getPathName();
    handle = fopen(f.toCSTR().buffer(),
                   mode.toCSTR().buffer());
    
    if (handle == NULL)
      return false;
    else
      return true;
  }
  
  void File::close()
  {
    if (handle != NULL) {
      fclose(handle);
      handle = NULL;
    }
  }
  
  int File::read(void *data, int size, int count)
  {
    return (int)fread(data, size, count, handle);
  }
  
  int File::write(const void *data, int size, int count)
  {
    return (int)fwrite(data, size, count, handle);
  }
  
  int File::read(void *data, int count)
  {
    return (int)fread(data, 1, count, handle);
  }
  
  int File::write(const void *data, int count)
  {
    return (int)fwrite(data, 1, count, handle);
  }
  
  ByteString File::read(int size)
  {
    ByteString out(size);
    int r = (int)fread(out.buf, 1, size, handle);
    out.size = r;
    return out;
  }
  
  int File::read(ByteString &str, int size)
  {
    str.reserveAndCopy(str.length() + size);
    int r = (int)fread(str.buf, 1, size, handle);
    str.size += size;
    return r;
  }
  
  int File::write(const ByteString &str)
  {
    return (int)fwrite(str.buf, 1, str.size, handle);
  }
  
  int File::readLE(void *data, int size)
  {
    int r = (int)fread(data, 1, size, handle);
    if (!lilend) flipBytes(data, size);
    return r;
  }

  int File::readBE(void *data, int size)
  {
    int r = (int)fread(data, 1, size, handle);
    if (lilend) flipBytes(data, size);
    return r;
  }

  int File::writeLE(const void *data, int size)
  {
    char *temp = new char[size];
    memcpy(temp, data, size);
    if (!lilend) flipBytes(temp, size);
    return (int)fwrite(temp, 1, size, handle);
  }

  int File::writeBE(const void *data, int size)
  {
    char *temp = new char[size];
    memcpy(temp, data, size);
    if (lilend) flipBytes(temp, size);
    return (int)fwrite(temp, 1, size, handle);
  }
  
  bool File::seek(int offset, int whence)
  {
    return (fseek(handle, offset, whence) == 0 ? true : false);
  }
  
  int File::tell()
  {
    return ftell(handle);
  }
    
  //statics
  /////////////////////////////////////////////////////////////////
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
    snprintf(procLink, 64, "/proc/%i/exe", pid);
    
    char exePath[1024];
    int pathLen = readlink(procLink, exePath, 1024);
    if (pathLen == -1) return NULL;
    exePath[pathLen] = 0;
    
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
