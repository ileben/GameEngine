#include <iostream>
#include <fstream>
#include <string>
using namespace std;

//#include "../../test/test.embedded"

void replace( string &str, const string &find, const string &replace )
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

int main (int argc, char **argv)
{
  //cout << test_embedded;
  //getchar();
  //return 0;

  if (argc < 3) {
    cerr << "Usage: embedit INPUT OUTPUT" << endl;
    return 1;
  }

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

  //Output the array declaration
  outfile << "const char " << arrayName << "[] = {" << endl;
  int numChars = 0; do
  {
    //Read one byte
    char oneByte;
    infile.get( oneByte );

    //Zero-terminate on last one
    if (!infile.good())
      oneByte = '\0';

    //Comma if not first
    if (numChars > 0)
      outfile << ", ";

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

  } while (infile.good());

  outfile << endl << "};" << endl;
  outfile.close();
  infile.close();

  return 0;
}
