# -*- python -*-

env = Environment()

env.Append(CCFLAGS = [
    "-I.",
    "-DMULTIASSIGN",
#    "-DDEBUG",
    "-ggdb",
    "-Wall",
    "-Werror",
    "-std=c++11",
    "-pedantic",
    "-O2",
    "-pthread",
    ""])
    
env.Append(LINKFLAGS = [
    "-pthread",
    ""])
    
env.SharedLibrary(target = 'hdl',
                  source = ["base.cpp",
                            "part.cpp"])

env.Program(target = 'example',
            source = 'example.cpp',
            LIBS = 'hdl',
            LIBPATH = '.')
