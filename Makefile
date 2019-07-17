# set the compiler
CXX      := mpic++
CXXFLAGS := -std=c++11 -O3 -Isrc/
LDFLAGS  := -L/usr/lib -L/usr/lib64 -L/usr/lib64/mpich/lib
#LIBS     := -lmpichcxx
LIBS	 := -lmpi -lboost# this is for HAL

# set the main directories
distr_util := src/distr_utility
apso       := src/async_pso
spso       := src/sync_pso
particles  := src/particle

# get the cpp and h/hpp/hxx files
distr_cpp   := $(wildcard $(distr_util)/*.cpp)
apso_cpp    := $(wildcard $(apso)/*.cpp)
spso_cpp    := $(wildcard $(spso)/*.cpp)
parts       := $(wildcard $(particles)/*.cpp)
src1        := src/main.cpp $(distr_cpp) $(apso_cpp) $(spso_cpp) $(parts)
distr_h     := $(wildcard $(distr_util)/*.h*)
pso_h       := $(wildcard $(apso)/*.h* $(spso)/*.h* $(particles)/*.h*)
hdr1        := $(distr_h) $(pso_h)

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
