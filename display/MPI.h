#pragma once

#include <mpi.h>
#include "common/vec.h"

// IMPI on Windows defines MPI_CALL already, erroneously
#ifdef MPI_CALL
# undef MPI_CALL
#endif
/*! helper macro that checks the return value of all MPI_xxx(...)
    calls via MPI_CALL(xxx(...)).  */
#define MPI_CALL(a) { int rc = MPI_##a; if (rc != MPI_SUCCESS) throw std::runtime_error("MPI call returned error"); }

namespace ospray {
  namespace dw {

    struct MPI {
      static void init(int &ac, char **&av)
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

      struct Group {
        Group() 
          : comm(MPI_COMM_NULL),rank(-1),size(-1)
        {}

        Group dup() {
          PING;
          MPI_Comm newComm;
          MPI_CALL(Comm_dup(comm,&newComm));
          return Group(newComm);
        }

        Group(MPI_Comm comm)
          : comm(comm)
        {
          MPI_CALL(Comm_test_inter(comm,&isInter));
          if (isInter) {
            rank = -1;
            MPI_CALL(Comm_remote_size(comm,&size));
          } else {
            MPI_CALL(Comm_rank(comm,&rank));
            MPI_CALL(Comm_size(comm,&size));
          }
        }

        void barrier() { MPI_CALL(Barrier(comm)); }
        MPI_Comm comm;
        int rank, size;
        int isInter;
      };
    };

  } // ::ospray::dw
} // ::ospray


