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

#include "ospcommon/box.h"
#include <sstream>

namespace ospray {
  namespace dw {

    using namespace ospcommon;

    /*! @{ settings for nasa hyperwall */
    // 16 displays wide, 8 displays high */
    const vec2i numPhysicalDisplays(16,8);
    const box2i physicalRange(vec2i(0),numPhysicalDisplays);

    /* numbering of displays is x major, y minor, y reverse (up to
       down), x normal (left to right)  */
    const std::string arrangement = "xY";
    /*! @} */

    /*! for nasa hyperwall */
    std::string nodeNameOfDisplay(const vec2i &coords)
    {
      const int nodeID = coords.x * numPhysicalDisplays.y + coords.y;
      char nodeName[100];
      sprintf(nodeName,"n%03i",nodeID);
      return nodeName;
    }

    extern "C" int main(int ac, char **av)
    {
      // 
      // num displays to run on
      vec2i cmdLine_numDisplays(3,2);
      vec2i cmdLine_firstDisplay(2,2);

      for (int i=1;i<ac;i++) {
        const std::string arg = av[i];
        if (arg == "-fd" || arg == "--first-display") {
          cmdLine_firstDisplay.x = atoi(av[++i]);
          cmdLine_firstDisplay.y = atoi(av[++i]);
        } else if (arg == "-nd" || arg == "--num-displays") {
          cmdLine_numDisplays.x = atoi(av[++i]);
          cmdLine_numDisplays.y = atoi(av[++i]);
        } else 
          throw std::runtime_error("unknown parameter '"+arg+"'");
      }
      box2i cmdLine_range(cmdLine_firstDisplay,
                          cmdLine_firstDisplay+cmdLine_numDisplays);
      
      // make sure we clamp everything to the
      box2i displayRange
        = intersectionOf(cmdLine_range,physicalRange);

      std::cout << "creating launch commands for display wall of " 
                << displayRange.size().x << "x" << displayRange.size().y << " displays, starting at " << displayRange.lower << std::endl;
      
      FILE *displayNodeList = fopen("nodelist.display","w");
      FILE *renderNodeList = fopen("nodelist.render","w");
      
      std::string pbsNames="";

      fprintf(renderNodeList,"localhost\n");
      // OUTER loop is on x
      for (int ix=displayRange.lower.x;ix<displayRange.upper.x;ix++) {
        // INNER loop is on y
        for (int iy=displayRange.lower.y;iy<displayRange.upper.y;iy++) {
          const std::string nodeName = nodeNameOfDisplay(vec2i(ix,iy));
          fprintf(renderNodeList,"%s\n",nodeName.c_str());
          fprintf(displayNodeList,"%s\n",nodeName.c_str());
          
          if (pbsNames != "") pbsNames = pbsNames+"+";
          pbsNames = pbsNames+"1:"+nodeName;
        }
      }

      fclose(displayNodeList);
      fclose(renderNodeList);
    
      std::stringstream pbsCommand;
      pbsCommand << "qsub -I -V select=" << pbsNames;
      std::cout << "# pbs command to aquire displays: " << std::endl;
      std::cout << pbsCommand.str() << std::endl << std::endl;
      std::cout << "# mpirun to start the displays:" << std::endl;
      std::cout << "  mpirun"
                << " -genv DISPLAY :0 "
                << " -perhost 1 "
                << " -hostfile nodelist.display"
                << " -n " << displayRange.size().product()
                << " ospDisplayWald"
                << " -w " << displayRange.size().x
                << " -h " << displayRange.size().y
                << " -nhn -fs"
                << std::endl << std::endl;;

      std::cout << "# mpirun to start ospray (workes on same nodes as display):" << std::endl;
      std::cout << "export DISPLAY_WALL=" << nodeNameOfDisplay(displayRange.lower) << std::endl;
      std::cout << "  mpirun"
                << " -perhost 1 "
                << " -hostfile nodelist.render"
                << " -n 1 OSPRAY_VIEWER_AND_ARGS --osp:mpi"
                << " :"
                << " -n " << displayRange.size().product()
                << " ./ospray_mpi_worker --osp:mpi"
                << std::endl << std::endl;;

      return 0;
    }
  }
}
