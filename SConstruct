env = Environment()

env.Append(CCFLAGS = [
    "-I.",
    "-DMULTIASSIGN",
#    "-DHDL_MT",
#    "-DDEBUG",
#    "-pg",
    "-ggdb",
    "-Wall",
    "-Werror",
    "-std=c++11",
    "-pedantic",
    "-O2",
#    "-fno-inline",
    "-pthread",
    ""])
    
env.Append(LINKFLAGS = [
    "-pthread",
#    "-pg",
    ""])
    
env.SharedLibrary(target = 'hdl',
                  source = ["base.cpp",
                            "part.cpp"])

env.Program(target = 'example',
            source = 'example.cpp',
            LIBS = 'hdl',
            LIBPATH = '.')
