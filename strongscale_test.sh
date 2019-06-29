#/bin/bash

for i in 1 2 4 8 16 32 48
do

mpiexec -np ${i} ./apso_test > out${i}.txt

done
