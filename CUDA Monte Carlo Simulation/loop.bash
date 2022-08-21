#!/bin/bash
for t in 1024 4096 16384 65536 262144 1048576 2097152 4194304
do
        for b in 8 32 128
        do
                /usr/local/apps/cuda/cuda-10.1/bin/nvcc -DNUMTRIALS=$t -DBLOCKSIZE=$b -o montecarlo  montecarlo.cu
                ./montecarlo
        done
done

# #!/bin/csh
# #number of array size:
# foreach t ( 1000 5000 10000 30000 100000 500000 1000000 2000000 4000000 8000000 )
# 	g++ -DARRAYSIZE=$t project4_extraCredit.cpp -o prog -lm -fopenmp
# 	./prog
# end

# #clear files
# rm -f project4.o prog
