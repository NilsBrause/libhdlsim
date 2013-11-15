env = Environment()
env.Append(CCFLAGS = ["-I.",
                      "-std=c++11",
                      "-Wall",
                      "-Werror",
                      "-fopenmp"])
env.Append(LINKFLAGS = ["-fopenmp"])
env.Program(target = 'main',
            source = ["main.cpp",
                      "base.cpp",
                      "process.cpp"])
