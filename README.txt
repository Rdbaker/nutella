Ryan Baker
rdbaker

Distributed Shell

NOTE: If you do not need it, please remove "-lcrypt" from the Makefile $(CFLAGS) variable. It works just fine on the CCC machines, but I could not compile it locally with that.


******************************************
        DEPENDENCIES (Server side)
******************************************
`gcc --version` returns the following:
gcc (GCC) 4.8.3 20140911 (Red Hat 4.8.3-9)


`lscpu` returns the following:
Architecture:          x86_64
CPU op-mode(s):        32-bit, 64-bit
Byte Order:            Little Endian
CPU(s):                1
On-line CPU(s) list:   0
Thread(s) per core:    1
Core(s) per socket:    1
Socket(s):             1
NUMA node(s):          1
Vendor ID:             GenuineIntel
CPU family:            6
Model:                 63
Model name:            Intel(R) Xeon(R) CPU E5-2676 v3 @ 2.40GHz
Stepping:              2
CPU MHz:               2400.050
BogoMIPS:              4800.10
Hypervisor vendor:     Xen
Virtualization type:   full
L1d cache:             32K
L1i cache:             32K
L2 cache:              256K
L3 cache:              30720K
NUMA node0 CPU(s):     0


`lsblk` returns the following:
NAME    MAJ:MIN RM SIZE RO TYPE MOUNTPOINT
xvda    202:0    0   8G  0 disk 
└─xvda1 202:1    0   8G  0 part /


`free` returns the following:
             total       used       free     shared    buffers     cached
Mem:        503380     433520      69860         60      22940     331256
-/+ buffers/cache:      79324     424056
Swap:            0          0          0


`uname -a` returns the following:
Linux ip-172-31-58-142 4.1.10-17.31.amzn1.x86_64 #1 SMP Sat Oct 24 01:31:37 UTC 2015 x86_64 x86_64 x86_64 GNU/Linux



******************************************
        Running It (Server side)
******************************************
usage: server [flags], where flags are:
  -p #    the port to server from (default is 80085)
  -d dir    the dir to serve files from (default is /home/ubuntu/dsh)
  -h    display this help message

by default, this will run on port 8080



******************************************
        Running It (Client side)
******************************************
usage: dsh [flags] {-c command}, where flags are:
  {-c command}  command to execute remotely
  {-s host} the host the server is on
  [-p #]    the port the server is on
  -h    display this help message

by default, this will run on port 8080



******************************************
        Example Run (Server side)
******************************************
$ make
$ ./bin/server

Starting server
  port: 8080
  dir: /home/ec2-user/dsh

server: got connection from 130.215.36.57
received connection from: rdbaker
Password is ok
Running command: ls

server: got connection from 130.215.36.57
received connection from: rdbaker
Password is ok
Running command: cat src/server.c


******************************************
        Example Run (Client side)
******************************************
$ make
$ ./bin/dsh -s 54.209.124.158 -c "ls"
client: connecting to 54.209.124.158
bin
Makefile
obj
README.txt
src

$ ./bin/dsh -s 54.209.124.158 -c "src/server.c"
client: connecting to 54.209.124.158
#include "server.h"
...
