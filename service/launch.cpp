/*
 Copyright (c) 2016 Ingo Wald
 
 Permission is hereby granted, free of charge, to any person obtaining a copy
 of this software and associated documentation files (the "Software"), to deal
 in the Software without restriction, including without limitation the rights
 to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 copies of the Software, and to permit persons to whom the Software is
 furnished to do so, subject to the following conditions:
 
 The above copyright notice and this permission notice shall be included in all
 copies or substantial portions of the Software.
 
 THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
 SOFTWARE.
 */

#include <stdlib.h>
#include "../common/tinyxml/tinyxml.h"
#include <iostream>
#include <fstream>
#include <string>
#include <sstream>

#define COMPARE(a,b) (strcmp(a,b)==0)

inline std::string toString (int i) {
  std::ostringstream convert;
  convert << i;
  return convert.str();
}

void ProcessConfigFile (const char * fileName)
{
  bool useHeadNode = false;
  
  TiXmlDocument doc( fileName );
  bool loadOkay = doc.LoadFile();
  
  if (!loadOkay)
  {
    printf( "Could not load displaywall configuration file %s. Error='%s'. Exiting.\n", fileName, doc.ErrorDesc() );
    exit( 1 );
  } else {
    //printf("Successfully loaded displayWald configuration file.\n");
  }
  
  // Read in display configuration.
  TiXmlElement *xml = doc.FirstChildElement("configuration");
  if (!xml) {
    printf("Displaywall configuration file does not contain a <configuration> element.\n");
    exit( 1 );
  }
  
  std::ofstream launchScript("launchDisplayWaldScript");
  std::string s;
  std::string mpirunCommand = "mpirun";
  std::string hostList = "";
  std::string displayWallCommand = "./ospDisplayWald";
  
  int nScreens = 0;
  int nNodes = 0;
  int numTilesWidth = 0, numTilesHeight = 0;
  int defaultScreenWidth = 0, defaultScreenHeight = 0;
  
  for (TiXmlElement *child = xml->FirstChildElement(); child!=NULL; child = child->NextSiblingElement()) {
    
    if ( COMPARE( child->Value(), "dimensions" ) ) {
      child->QueryIntAttribute( "numTilesWidth", &numTilesWidth);
      child->QueryIntAttribute( "numTilesHeight", &numTilesHeight);
      
      child->QueryIntAttribute( "screenWidth", &defaultScreenWidth);
      child->QueryIntAttribute( "screenHeight", &defaultScreenHeight);
      
    } else if ( COMPARE( child->Value(), "process" ) ) {
      ++nNodes;
      
      const char * hostName = child->Attribute("host");
      const char * displayName = child->Attribute("display");
      
      for (TiXmlElement *process = child->FirstChildElement(); process != NULL; process = process->NextSiblingElement()) {
        if ( COMPARE( process->Value(), "screen" ) ) {
          
          if (hostName) {
            if (hostList.size() > 0)
              hostList += ",";
            hostList += hostName;
          }
          
          ++nScreens;
        }
      }
    }
  }
  
  displayWallCommand += " --width " + toString(numTilesWidth) + " --height "  + toString(numTilesHeight);
  displayWallCommand += " --window-size " + toString(defaultScreenWidth) + " " + toString(defaultScreenHeight);
  
  //printf("\nDisplaywall: %d screens on %d nodes.\n", nScreens, nNodes);
  
  if (useHeadNode) {
    hostList = "localhost," + hostList;
    ++nScreens;
    displayWallCommand += " --head-node";
  } else {
    displayWallCommand += " --no-head-node";
  }
  
  s = mpirunCommand + " -np " + toString(nScreens) + " -hosts " + hostList + " " + displayWallCommand;
  
  printf("launching with: %s\n", s.c_str());
  
  launchScript << s;
  
  launchScript.close();
}

int main(int ac, char **av)
{
  ProcessConfigFile("configuration.xml");
  
  system("chmod +x launchDisplayWaldScript; ./launchDisplayWaldScript");
  
  return 0;
}
