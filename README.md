# CoraxOS

CoraxOS is a hobbyist kernel built to better understand how operating systems work.

<video src="demo.mp4" controls preload></video>

# Features
- Memory Management
- Partial libc implementation (see acknowledgement for parts that were copied)
- Signals
- Command line arguments
- Processes and kernel/user threads
- Rudimentary VFS
- Ramfs
- SDL library port (see https://github.com/coraxos/SDL)
- Runs DOOM

# Building

1. Run the do-build.sh script which will create the cmake files in the "build" folder and build the project
2. cd into the build folder and run "make"

### TODO
- SMP
- Rework the filesystem to better handle disk fs
- Set proper errorno on syscall failure
- Implement more signals that do stuff (currently only SIGALRM)
- Get rid of all the //TODO in the code

# Acknowledgement
Special thanks to http://github.com/arjun024/mkernel, whose code was used to start writing the kernel
[skiftOS](https://github.com/skiftOS/skift) for the design inspiration and the toolchain compiler scripts
[toaruos](https://github.com/klange/toaruos) for how to port sdl and DOOM to a new OS
[MUSL](https://www.musl-libc.org/). The ctype and math code in libc were copied from here.
[Standalone-printf](https://github.com/rqou/standalone-printf-scanf). I had a printf implementation, but decided to use this one to save time creating libc.