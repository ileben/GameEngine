#include <iostream>
#include <fstream>
#include <string>
using namespace std;

//#include "../../test/test.embedded"

void replace (string &str, const string &find, const string &replace)
{
  string::size_type replaceLen = replace.length();
  string::size_type loc = 0;

  do
  {
    loc = str.find( find, loc );
    if (loc != string::npos) {
      str.replace( loc, replaceLen, replace );
      loc += replaceLen; }
  }
  while (loc != string::npos);
}

void embed (fstream &infile, fstream &outfile, const string &arrayName, bool preprocess)
{
  bool good = true;

  //Output the array declaration
  outfile << "const char " << arrayName << "[] = {" << endl;
  int numChars = 0; do
  {
    //Read one byte
    char oneByte;
    infile.get( oneByte );

    //Zero-terminate on last one
    if (!infile.good() || (preprocess && oneByte=='#')) {
      oneByte = '\0';
      good = false;
    }

    //Comma if not first
    if (numChars > 0)
      outfile << ",";

    //Break line each 8 bytes
    if (numChars == 8) {
      outfile << endl;
      numChars = 0;
    }

    //Hex mark
    outfile << "0x";

    //Byte as hex filled with 0s on left side to width of 2
    outfile.width( 2 );
    outfile.fill( '0' );
    outfile.flags( ios::right | ios::hex );
    outfile << ((int)oneByte);
    outfile.flags( 0 );

    //Char count
    numChars++;

  } while (good);

  outfile << endl << "};" << endl;
}

void seekUntil (fstream &infile, char end)
{
  char oneByte; do
  {
    infile.get( oneByte );
    if (!infile.good())
      break;
  }
  while (oneByte != end);
}

int main (int argc, char **argv)
{
  //cout << array2;
  //getchar();
  //return 0;

  //Need at least 3 arguments
  if (argc < 3) {
    cerr << "Usage: embedit INPUT OUTPUT" << endl;
    return 1;
  }

  //Check for preprocess option
  bool preprocess = false;
  if (argc >=4) {
    string arg4 = argv[3];
    if (arg4 == "--preprocess")
      preprocess = true;
  }

  //Open the input and output stream
  fstream infile( argv[1], fstream::in );
  fstream outfile( argv[2], fstream::out );
  
  if (infile.fail()) {
    cerr << "Failed opening '" << argv[1] << "' for reading!" << endl;
    return 1;
  }

  if (outfile.fail()) {
    cerr << "Failed opening '" << argv[1] << "' for writing!" << endl;
    return 1;
  }

  //Preprocess data blocks if option given
  if (preprocess)
  {
    while (infile.good())
    {
      //Find next '#'
      seekUntil( infile, '#' );

      //Need more data
      if (!infile.good())
        break;

      //Get the command
      char buf[256];
      infile.width( 256 );
      infile >> buf;
      string command = buf;

      //Need more data
      if (!infile.good())
        break;

      //New data block on #begin
      if (command == "begin")
      {
        //Get the array name
        infile >> buf;
        string arrayName = buf;

        //Get to the end of line
        seekUntil( infile, '\n' );

        //Need more data
        if (!infile.good())
          break;

        //Embed into output file until '#' reached
        embed( infile, outfile, arrayName, true );

        //Get the command
        infile.width( 256 );
        infile >> buf;
        command = buf;

        //Stop if it's not #end
        if (command != "end")
          break;

        //Get to the end of line
        seekUntil( infile, '\n' );
      }
    }
  }
  else//if preprocess
  {
    //Find last slash and backslash in the output name
    string arrayName = argv[2];
    string::size_type lastSlash = arrayName.rfind( "/" );
    string::size_type lastBkSlash = arrayName.rfind( "\\" );
    
    //Pick the last of the two
    if (lastBkSlash != string::npos)
      if (lastSlash == string::npos || lastBkSlash > lastSlash)
        lastSlash = lastBkSlash;

    //Cut off everything before
    if (lastSlash != string::npos)
      arrayName = arrayName.substr( lastSlash+1 );

    //Replace invalid characters
    replace( arrayName, ".", "_" );

    //Embed whole input into output file
    embed( infile, outfile, arrayName, false );
  }

  outfile.close();
  infile.close();

  return 0;
}
