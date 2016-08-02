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

#include "MPI.h"

namespace ospray {
  namespace dw {

    void MPI::init(int &ac, char **&av)
    {
      int initialized = false;
      MPI_CALL(Initialized(&initialized));
      
      if (!initialized) {
        // MPI_Init(ac,(char ***)&av);
        int required = MPI_THREAD_MULTIPLE;
        int provided = 0;
        MPI_CALL(Init_thread(&ac,(char ***)&av,required,&provided));
        if (provided != required)
          throw std::runtime_error("MPI implementation does not offer multi-threading capabilities");
      }
    }

    MPI::Group::Group(MPI_Comm comm)
      : comm(comm)
    {
      if (comm == MPI_COMM_NULL) {
        rank = -1;
        size = -1;
        isInter = false;
      } else {
        MPI_CALL(Comm_test_inter(comm,&isInter));
        if (isInter) {
          rank = -1;
          MPI_CALL(Comm_remote_size(comm,&size));
        } else {
          MPI_CALL(Comm_rank(comm,&rank));
          MPI_CALL(Comm_size(comm,&size));
          int len;
          MPI_CALL(Get_processor_name(name,&len));
        }
      }
    }
    
    MPI::Group MPI::Group::dup() const
    {
      MPI_Comm newComm;
      MPI_CALL(Comm_dup(comm,&newComm));
      return Group(newComm);
    }
    
  } // ::ospray::dw
} // ::ospray


