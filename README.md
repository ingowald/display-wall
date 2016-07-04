DisplayWald - A simple diplay wall driver
=========================================

Introduction
------------

This is a simple library for displaying large frame buffers on a
display wall. We do explicitly NOT offer any windowing abstractions,
I/O, etc; this code is EXCLUSIVELY for displaying. In particular, this
code allows for specifying an arrangement of WxH displays (all of
which have to have the same resolution); we then launch one process
per dipslay node, which opens a (usually fullscreen) window and
displays frames when they are done.

We do support stereo, and a library to connect to this display wall
and (in a mpi parallel fashion) write to this display wall from
multiple nodes. Both dw server (that runs the displays) as well as
client access library (which allows an (mpi-parallel) app to write to
this display wall) use MPI for communication. 

For network topologies where the app node(s) cannot 'see' the actual
display nodes directly we also support a 'head node' setup where the
server establishes a dispatcher on a single head node that is visible
from both app and display nodes.

Eventually this code will compress tiles before writing; but this is
NOT yet implemented (look at CompresssedTile::encode/decode() to
implement this, probably using libjpg, libjpeg-turbo, or libpng).

Starting the DisplayWald Server
-------------------------------

First: determine if you do want to run with a head node, or
without. If all display nodes can be seen from the nodes that generate
the pixels you do not need a head node (and arguably shouldn't use
one); if the actual display nodes are 'hidden' behind a head node you
have to use a head node.

Second, determine display wall arrangement (NX x NY displays, stereo?, arrangement?). 

Finaly, laucnh the ospDisplayWald executable through mpirun, with the proper paramters.

Example 1: Running on a 3x2 non-stereo display wall, WITHOUT head node

				 mpirun -perhost 1 -n 6 ./ospDisplayWald -w 3 -h 2 --no-head-node

Note how we use 6 (==3x2) ranks; which is one per display.

Example 2: Running on a 3x2 non-stereo display wall, WITH dedicated head node

				 mpirun -perhost 1 -n 7 ./ospDisplayWald -w 3 -h 2 --head-node
Note we use have to use *7* ranks; 6 for the displays, plus one (on rank 0) for the head node.

In all examples, make sure to use the right hosts file that properly
enumerates which ranks run which role. WHen using a head node, rank 0
should be head node; all other ranks are one rank per display,
starting on the lower left and progressing right first (for 'w'
nodes), then upwards ('h' times)


Implementing a DisplayWald client
---------------------------------

basically three steps:
- link to the displaywald-client lib
- create a client
- establish a connection (client->establishConnection) [do this together on all ranks!].
  YOu need to specify the MPI port name that the service is running on.
- look at the respective display wall cofig to figure out what frame res to deal with
- do writeTiles() until all of a frame's pixels have been set



TODO
====

high priority
-------------

- implement some sort of image compression on the tiles (libjpg, libpng, libjpeg-turbo, ...)
- multi-thread the tile receiving - in theory not a big problem, but needs to be done
- implement full-screen capabilities for glutwindow

low priority 
------------

- implement stereo support; let _client_ request stereo mode (not
  glutwindow), and have server react accordingly. need to modify 'api'
  for writeTile to specify which eye the tile belongs to.
  
  
