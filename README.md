This is the C++ artifact for DISC 2022's blind submission. The project
contains three subdirectories - _Uruv_, _vcas_bst_, and _OpenBwTree_. These directories
contain code that benchmarks the trees Uruv, VCAS-BST and OpenBw-Tree
respectively. Your source code would have extracted into a folder 
called `Uruv-main` by default. Please read the entire document 
carefully for a successful execution of the benchmarks.

## 1. Dataset Generation
Before you build and run the benchmarks, randomised datasets 
that can be read from disk need to be generated. `DatasetGen/dataset_generator.cpp`
contains logic to generate random integer datasets and store them 
on disk. _**You can only create dataset sizes that are an integer multiple of 1 million**_.
Let us say you want to create a dataset of size 500 million. Do the following,
1. Create a subdirectory `DatasetGen/Data` if it does not exist. 
Create a new subdirectory named `500M` inside `DatasetGen/Data`. If
you were creating a dataset of size 100 million instead, you would name 
the subdirectory `100M`.
2. Set how many files you want this dataset to be split into for multi-threaded 
reading while benchmarking. This can be done by setting the variable _dataset_prep_threads_ on _line 12_
in `dataset_generator.cpp`. If you set this value to 1, benchmarking larger
datasets will take a lot of time. It is highly recommended to set this
to the maximum number of logical threads in your system. 
3. Go back to `Uruv-main/DatasetGen`
4. `cmake -S ./`
5. `make`
6. The executable `/build/DatasetGen` is created. To run it you need
to pass two command line arguments to it. 
7. The first argument sets the value of the variable _dataset_prep_threads_
and is required to quickly read the dataset from disk. Set it to the
maximum number of logical threads in your system. Let us say the 
maximum number of threads are 40.
8. The second argument is the dataset size which is 500000000.
9. Run `./build/DatasetGen 500000000 40` to generate your dataset.

## 2. Building and running the benchmarks
The instructions mentioned in this section, apply to all three subdirectories - 
_Uruv_, _vcas_bst_ and _OpenBwTree_. Each subdirectory contains 
their own CMakeLists which needs to be built independently. They 
contain a `Benchmark.cpp` file containing logic that benchmarks the
data structure and a script `run_benchmarks.sh` that runs various
workloads. Before we get into building and running the benchmarks, 
we digress a little. `Benchmark.cpp` contains a variable
_dataset_prep_threads_. The value of this variable needs to be set
equal to the value set in step 7 of Section 1, otherwise the benchmarks
will fail to execute. Additionally, it is critical that you 
execute these benchmarks with datasets that are stored on disk by 
following the steps in Section 1.

Coming back to building and running the benchmarks, the script `run_benchmarks.sh`
builds the code and runs the executable for you. You can run various workloads from the paper by using the command
`bash run_benchmarks.sh`. A file `final_benchmarks.txt` will be created
containing the result of each trial in the benchmark. Every line in 
the file is of the format,

`number_of_threads` _WHITESPACE_ `read_percentage` _WHITESPACE_ `insert_percentage` _WHITESPACE_ 
`delete_percentage` _WHITESPACE_ `range_query_percentage` _WHITESPACE_ 
`range_query_size` _WHITESPACE_ `prefilling_size` _WHITESPACE_ `dataset_size` 
_WHITESPACE_ `throughput per second`

Each workload runs 10 times and so you'll find 10 lines per workload.
However, if you would like to build the code on your own instead, 
do the following,
1. `cmake -S ./`
2. `make`
3. The executable will be present in the `build` directory.
4. The executable will take in 8 command line arguments. 
5. The first argument is the number of threads accessing the data structure
concurrently.
6. The second is the read percentage of your workload. The value lies
in between 0 and 100.
7. The third argument is the insert percentage.
8. The fourth is the delete percentage.
9. The fifth is the range query percentage. Note the values passed 
for arguments 2-5 should sum to 100 otherwise the benchmark will fail.
10. The sixth argument is the range query's range size -- how large a
range do you want to query.
11. The seventh argument is the prefill size. How large do you want
your data structure to be _before_ the benchmark starts. In the paper, 
we prefill it with a 100M keys.
12. The final argument is the dataset size. This is the universe of keys
from which the prefilling happens and the benchmark picks keys.



