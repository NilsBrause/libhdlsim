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
    "-O2",
#    "-fno-inline",
    "-pthread",
    ""])
env.Append(LINKFLAGS = [
    "-pthread",
#    "-pg",
    ""])
env.Program(target = 'main',
            source = ["main.cpp",
                      "base.cpp",
                      "part.cpp"])
