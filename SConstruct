env = Environment()
env.Append(CCFLAGS = ["-I.",
#                      "-DDEBUG",
                      "-ggdb",
                      "-fopenmp",
                      "-Wall",
                      "-Werror",
                      "-std=c++11"])
env.Append(LINKFLAGS = ["-fopenmp"])
env.Program(target = 'main',
            source = ["main.cpp",
                      "base.cpp",
                      "process.cpp"])
