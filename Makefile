# set the compiler
CXX      := mpic++
CXXFLAGS := -std=c++11 -O3 -Isrc/
LDFLAGS  := -L/usr/lib -L/usr/lib64 -L/usr/lib64/mpich/lib
LIBS     := -lmpichcxx

# set the main directories
distr_util := src/distr_utility
pso        := src/async_pso
compare    := src/compare_pso

# get the cpp and h/hpp/hxx files
distr_cpp   := $(wildcard $(distr_util)/*.cpp)
pso_cpp     := $(wildcard $(pso)/*.cpp)
src1        := src/main.cpp $(distr_cpp) $(pso_cpp)
compare_cpp := $(wildcard $(compare)/*.cpp)
src2        := $(src1) $(compare_cpp)
distr_h     := $(wildcard $(distr_util)/*.h*)
pso_h       := $(wildcard $(pso)/*.h*)
hdr1        := $(distr_h) $(pso_h)
compare_h   := $(wildcard $(compare)/*.h*)

# specify the object files
obj1 := $(patsubst %.cpp, %.o, $(src1))
obj2 := $(patsubst %.cpp, %.o, $(src2))

# specify the possible binaries
APSO_Test := apso_test

# try to compile some stuff
test: $(obj1)
	$(CXX) $(LDFLAGS) $(LIBS) -o $(APSO_Test) $^

%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

clean:
	rm -f $(obj1) $(obj2)
	rm -f $(APSO_Test)
