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
      static void init(int &ac, char **&av);

      struct Group {
        Group(MPI_Comm comm=MPI_COMM_NULL);
        Group dup() const;
        void barrier() const { MPI_CALL(Barrier(comm)); }

        MPI_Comm comm;
        int rank, size;
        int isInter;
      };
    };

  } // ::ospray::dw
} // ::ospray


