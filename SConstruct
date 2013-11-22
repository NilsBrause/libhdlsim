env = Environment()
env.Append(CCFLAGS = ["-I.",
#                      "-DDEBUG",
                      "-ggdb",
                      "-std=c++11",
                      "-Wall",
                      "-Werror",
                      "-fopenmp"])
env.Append(LINKFLAGS = ["-fopenmp"])
env.Program(target = 'main',
            source = ["main.cpp",
                      "base.cpp",
                      "process.cpp"])
