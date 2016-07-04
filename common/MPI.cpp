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


