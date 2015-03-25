env = Environment()
env.Append(CCFLAGS = [
        "-I.",
        "-DMULTIASSIGN",
#        "-DDEBUG",
#        "-pg",
#        "-ggdb",
        "-fopenmp",
        "-Wall",
        "-Werror",
        "-std=c++11",
        "-O2"
        ""])
env.Append(LINKFLAGS = [
        "-fopenmp",
#        "-pg",
        ""])
env.Program(target = 'main',
            source = ["main.cpp",
                      "base.cpp",
                      "part.cpp"])
