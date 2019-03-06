# TDSL
A C++ implementation of the paper "Transactional Data Structure Libraries": http://webee.technion.ac.il/~idish/ftp/TransactionalLibrariesPLDI16.pdf

## Running Comparison With Java Implementation
We have implemented the experiments described in TDSL for both our C++ implementation and the Java implementation (see experiments.cpp and Experiments.java). These are the steps to perform in order to run this comparison:
1. Checkout the Java implementation (https://github.com/HagarMeir/transactionLib) and copy Experiments.java to src/main/java/. The Java experiments can now be run using the command: "mvn exec:java -Dexec.mainClass="Experiments" -Dexec.args="<WORKLOAD_TYPE> <NUM_THREADS>" from the checkout folder. 
2. Build the C++ implementation by running "cmake" and then "make tdsl-test". You can then run the test using the command "./tdsl-test <WORKLOAD_TYPE> <NUM_THREADS>". 
3. If you wish to run the experiments the way we did, you can use the scripts inside run_experiments. (i) run_experiments_cpp.py runs the C++ experiments given a tdsl-test path; (ii) run_experiments_java.py runs the Java experiments using maven - you must run it from the Java checkout folder; (iii) plot_comparison.py expects to get two results of the aforementioned scripts and plots a graph that compares them.

Workload types:
0 = READ_ONLY
1 = MIXED
2 = UPDATE_ONLY

Example of running the experiments and drawing a comparison graph:
1. python run_experiments_cpp.py tdsl-test 1 results_cpp
2. cd ../transactionLib; python run_experiments_java.py 1 results_java
3. python plot_comparison.py results_cpp results_java
