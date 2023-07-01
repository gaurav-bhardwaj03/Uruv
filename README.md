The project contains three subdirectories - _Uruv_, _vcas_bst_, and _OpenBwTree_. These directories
contain code that benchmarks the trees Uruv, VCAS-BST and OpenBw-Tree
respectively. Your source code would have extracted into a folder 
called `Uruv-main` by default. Please read the entire document properly
at least once before executing commands in it. It is critical to gain
a proper understanding of the code organisation and the benchmarking
of every data structure present.

## 1. Dataset Generation
Before you build and run the benchmarks, randomised datasets 
that can be read from disk need to be generated. `DatasetGen/dataset_generator.cpp`
contains logic to generate random integer datasets and store them 
on disk. _**You can only create dataset sizes that are an integer multiple of 1 million**_.
Let us say you want to create a dataset of size _500 million_. Do the following,
1. Create a subdirectory `DatasetGen/Data` if it does not exist. 
2. Create another subdirectory named `500M` inside `DatasetGen/Data` if it
does not exist. If it exists, empty out that folder. (NOTE, if you were 
creating a dataset of size 100 million instead, you would name 
the subdirectory `100M`)
3. Go back to `Uruv-main/DatasetGen`
4. Run `cmake -S ./`
5. Run `make`
6. The executable `/build/DatasetGen` is created. To execute it you 
need to pass two command line arguments to it. 
7. The first argument sets the value of the variable _dataset_prep_threads_
and is required to quickly read the dataset from disk. This argument 
tells the generator to split the dataset into _dataset_prep_threads_ 
number of files for efficient reading. Set it to the
maximum number of logical threads in your system. Let us say the 
maximum number of threads are 80.
8. The second argument is the dataset size which is 500000000 (since
these instructions are for creating a dataset of size 500M)
9. Run `./build/DatasetGen 80 500000000` to generate your dataset.

## 2. Building and running the benchmarks
The instructions mentioned in this section, apply to all three subdirectories - 
_Uruv_, _vcas_bst_ and _OpenBwTree_. Each subdirectory contains 
their own CMakeLists which needs to be built independently. They 
contain a `Benchmark.cpp` file containing logic that benchmarks the
data structure and a script `run_benchmarks.sh` that runs various
workloads from the paper.

To build the code and understand how to run and use the benchmark, 
do the following,
1. Run `cmake -S ./`
2. Run `make`
3. The executable will be present in the `build` directory.
4. The executable will take in 9 command line arguments. 
5. The first argument is the number of threads accessing the data structure
concurrently.
6. The second is the read percentage of your workload. The value lies
in between 0 and 100.
7. The third argument is the insert percentage.
8. The fourth is the delete percentage.
9. The fifth is the range query percentage. NOTE, the values passed 
for arguments 2-5 should sum to 100 otherwise the benchmark will fail.
10. The sixth argument is the range query's range size -- how large a
range you want to query.
11. The seventh argument is the prefill size. How large you want
your data structure to be _before_ the benchmark starts. In the paper, 
we prefill it with a 100M keys.
12. The eighth argument is the dataset size. This is the universe of keys
from which the prefilling happens and the benchmark picks keys. In
the paper, we set it to 500M.
13. The ninth argument is the value for _dataset_prep_threads_ as
explained in step 7 in section 1. In the paper, we set it to 80, but 
you should set it to the maximum logical threads in your system.
14. NOTE, arguments 8 and 9 need to be set to the same values you used
earlier (steps 7 and 8 in section 1) for datasets you generated. Not doing so, will result in a
failure in executing the benchmark.

A file `final_benchmarks.txt` will be created
containing the result of each trial in the benchmark. Every line in
the file is of the format,

`number_of_threads` _WHITESPACE_ `read_percentage` _WHITESPACE_ `insert_percentage` _WHITESPACE_
`delete_percentage` _WHITESPACE_ `range_query_percentage` _WHITESPACE_
`range_query_size` _WHITESPACE_ `prefilling_size` _WHITESPACE_ `dataset_size`
_WHITESPACE_ `throughput_per_second`

## 3. Running the workloads from the paper
Each subdirectory - _Uruv_, *OpenBwTree* and _vcas_bst_ contains `run_benchmarks.sh` 
which builds the code and runs the executable for you. It runs all 
workloads from the paper for that data structure and can be run using the command
`bash run_benchmarks.sh`. NOTE, the default script expects a dataset
of size 500 million split across 80 files to _**already be generated**_. If
you haven't generated the dataset yet, please follow Section 1 for
instructions on how to do so. Change the value of 80 for every workload 
in the script to the max logical threads in your system for best 
performance.

Each workload in `run_benchmarks.sh` runs 10 times and so you'll
find 10 lines per workload in the file `final_benchmarks.txt`. NOTE,
empty out `final_benchmarks.txt` before running the script provided
otherwise old data can be present, polluting the results.

To take the average of each of these 10 runs per workload, consider
the last 7 and take the mean of the throughputs ignoring
outliers.
