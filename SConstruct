# -*- python -*-

env = Environment()

env.Append(CCFLAGS = [
    "-I.",
    "-DMULTIASSIGN",
#    "-DDEBUG",
    "-ggdb",
    "-Wall",
    "-Wextra",
    "-Wpedantic",
    "-Wno-bool-operation",
    "-Werror",
    "-std=c++11",
    "-pedantic",
    "-O2",
    "-pthread",
    ""])
    
env.Append(LINKFLAGS = [
    "-pthread",
    ""])
    
env.SharedLibrary(target = 'hdlsim',
                  source = ["base.cpp",
                            "part.cpp",
                            "simulator.cpp"])

env.Program(target = 'example',
            source = 'example.cpp',
            LIBS = 'hdlsim',
            LIBPATH = '.')
