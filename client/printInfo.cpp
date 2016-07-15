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

#include "Client.h"

namespace ospray {
  namespace dw {

    using namespace ospcommon;
    
    using std::cout; 
    using std::endl;
    using std::flush;

    extern "C" int main(int ac, char **av)
    {
      if (ac != 3)
        throw std::runtime_error("usage: ./ospDwPrintInfo <hostname> <portNum>");

      const std::string hostName = av[1];
      const int portNum = atoi(av[2]);

      cout << "Trying to connect to display wall info port at " << hostName << ":" << portNum << endl;

      ServiceInfo info;
      info.getFrom(hostName,portNum);

      cout << "=======================================================" << endl;
      cout << "Found display wall service at " << hostName << ":" << portNum << endl;
      cout << "- mpi port name of service: " << info.mpiPortName << endl;
      cout << "- total num pixels in wall: " << info.totalPixelsInWall << endl;
      cout << "=======================================================" << endl;
      return 0;
    }
    
  } // ::ospray::dw
} // ::ospray
