This is the C++ artifact for DISC 2022's blind submission. The project
contains three subdirectories - _Uruv_, _vcas_bst_, and _OpenBwTree_. These directories
contain code that benchmarks the trees Uruv, VCAS-BST and OpenBw-Tree
respectively. Please download the source code inside a directory named 
`home/DISC_22_Artifact`. If you would like to use a custom directory instead, replace any occurence
of the aforementioned directory in this source code with your custom
directory. Please read the entire document carefully for a successful
execution of the benchmarks. 

## 1. Dataset Generation
Before the benchmarks, described below, are run, randomised datasets 
that can be read from disk need to be generated. `DatasetGen/dataset_generator.cpp`
contains logic to generate random integer datasets and store them 
on disk. _**You can only create dataset sizes that are an integer multiple of 1 million**_.
Let us say you want to create a dataset of size 500 million. Do the following,
1. Create a new subdirectory named `500M` inside `DatasetGen/Data`. If you were creating a dataset of size 100 million instead, you would name
the subdirectory `100M`. If `DatasetGen/Data` does not exist, create it.
2. Set the variable _DatasetSize_ to `500000000` on _line 14_ in `dataset_generator.cpp`.
3. Set how many files you want this dataset to be split into for multi-threaded 
reading while benchmarking. This can be done by setting the variable _dataset_prep_threads_ on _line 12_
in `dataset_generator.cpp`. If you set this value to 1, benchmarking larger
datasets will take a lot of time. It is highly recommended to set this
to the maximum number of logical threads in your system. 
4. cd `home/DISC_22_Artifact/DatasetGen`
5. `cmake -S ./`
6. `make`
7. `./build/DatasetGen`

## 2. Running the benchmarks
The instructions mentioned in this section, apply to all three subdirectories - 
_Uruv_, _vcas_bst_ and _OpenBwTree_. Each subdirectory in this artifact contains 
their own CMakeLists which needs to be built independently. They 
contain a `Benchmark.cpp` file containing logic that benchmarks the
data structure and a script `run_benchmarks.sh` that runs various
workloads. Before we get into building and running the benchmarks, 
we digress a little. `Benchmark.cpp` contains a variable
_dataset_prep_threads_. The value of this variable needs to be set
equal to the value set in step 3 of Section 1, otherwise the benchmarks
will fail to execute. Additionally, only execute these benchmarks
with dataset sizes stored on disk.

Coming back to building and running the benchmarks, the script `run_benchmarks.sh`
builds the code and runs the executable for you. However, if you
would like to build the code on your own instead, do the following,
1. `cmake -S ./`
2. `make`
3. The executable will be present in the `build` directory.

You can run various workloads from the paper by using the command
`bash run_benchmarks.sh`. A file `final_benchmarks.txt` will be created
containing the result of each trial in the benchmark. Every line in 
the file is of the format,

`number_of_threads` _WHITESPACE_ `read_percentage` _WHITESPACE_ `insert_percentage` _WHITESPACE_ 
`delete_percentage` _WHITESPACE_ `range_query_percentage` _WHITESPACE_ 
`range_query_size` _WHITESPACE_ `prefilling_size` _WHITESPACE_ `dataset_size`

Each workload runs 10 times and so you'll find 10 lines per workload.



