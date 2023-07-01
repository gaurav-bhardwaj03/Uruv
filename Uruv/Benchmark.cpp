//
// Created by gaurav on 01/04/22.
//

#include "BPTree.h"
#include <random>
#include <thread>
#include <fstream>
#include <cassert>
#include <unistd.h>
#include <algorithm>

int64_t table_size;
int64_t DatasetSize;
int64_t threads;
double Read;
double Insert;
double Delete;
double RangeQuery;
int64_t RangeQuerySize;
int64_t Runtime = 10;
bool running = false;
std::atomic<size_t> ready_threads (0);
std::vector<uint64_t> tho;
std::vector<uint64_t> dataset;
std::vector<std::vector<uint64_t>> dataset_split(160);
std::ofstream fout;
BPTree<uint64_t, int64_t>* tree;

void prepare_table ();
void run_benchmark ();
void prepare_dataset ();
void read_from_files();

int main(int argc, char** argv)
{
    std::cout<<"Hello\n";
    fout.open("final_benchmarks_SSS.txt", std::ios::app);
	threads = atoi(argv[1]);
	Read = ((double)atoi(argv[2]))/100;
	Insert = ((double)atoi(argv[3]))/100;
	Delete = ((double)atoi(argv[4]))/100;
	RangeQuery = ((double)atoi(argv[5]))/100;
	RangeQuerySize = atol(argv[6]);
	table_size = atol(argv[7]);
	DatasetSize = atol(argv[8]);
    printf("Parameters\nThreads: %ld\nRead: %lf\nInsert: %lf\nDelete: %lf\nRangeQuery: %lf\nRangeQuerySize: %ld\n"
           "TableSize: %ld\nDatasetSize: %ld\n", threads, Read, Insert, Delete, RangeQuery, RangeQuerySize, table_size, DatasetSize);
    assert(((int64_t)(Read + Delete + Insert + RangeQuery)) == 1);
    tho.resize(threads, 0);
    tree = new BPTree<uint64_t, int64_t>(threads);

	if(DatasetSize == 500000000 || DatasetSize == 150000000){
		std::cout << "Reading the dataset from the files\n";
		read_from_files();
	}
	else {
		prepare_dataset();
		std::random_device rd;
		std::mt19937 gen(rd());
		std::shuffle(begin(dataset), end(dataset), gen);
	}
	std::cout<<"Dataset Prepared\n";
    prepare_table ();
    std::cout<<"Table Prepared\n";
    for(int i = 0; i < threads; i++)
        tree ->init_thread(i);
    run_benchmark ();
    fout.close();
}

void dataset_prep(int64_t tid, int64_t start, int64_t fin){
	for(int i = start; i < fin; i++){
		dataset_split[tid].push_back(i);
	}
}

void prepare_dataset(){
	int th = 160;
	std::vector<std::thread> dataset_threads(th);
	int64_t number_of_keys_per_thread = DatasetSize/th;
	for(int64_t i = 0; i < th; i++){
		int64_t fin = std::min((i + 1) * number_of_keys_per_thread, DatasetSize);
		dataset_threads[i] = std::thread(dataset_prep, i, i * (number_of_keys_per_thread), fin);
	}
	for(int64_t i = 0; i < th; i++){
		dataset_threads[i].join();
	}
	for(int i = 0; i < th; i++){
		dataset.insert(dataset.end(), dataset_split[i].begin(), dataset_split[i].end());
	}
}

void run (int64_t tid) {
    tree ->init_thread(tid);
    ttid = tid;
    std::uniform_real_distribution<double> ratio_dis (0, 1);
    std::uniform_int_distribution<int64_t> index_dis(0, dataset.size() - 1);
    ready_threads++;
    std::random_device rd;
    std::mt19937 gen(rd());
    int64_t j = index_dis(gen);
    uint64_t dataset_prep_threads=0;
    uint64_t rq_put = 0;
    while (!running);
    while (running) {
        double d = (double) ratio_dis (gen);
        if (d <= Read) {
            tree -> search((uint64_t)dataset[j]);   // read
        } else if (d <= Read + Delete) {
            tree -> remove(dataset[j], tid);  // delete
        } else if (d <= Read + Delete + Insert) {
            tree -> insert(dataset[j], j, tid);  // insert
        }
        else{
            tree -> range_query(dataset[j], dataset[j] + RangeQuerySize);
        }
        j = (j + 1)%dataset.size();
        tho[tid]++;
    }
}

void tree_preparation(int64_t start, int64_t fin){
    for(int64_t i = start; i < fin ; i++)
        tree->insert(dataset[i], 10, 0, -1);
}

void prepare_table () {
	int th = 160;
	std::vector<std::thread> tree_prep_threads(th);
	int64_t number_of_keys_per_thread = table_size/th;
	for(int64_t i = 0; i < th; i++){
		int64_t fin = std::min((i + 1) * number_of_keys_per_thread, table_size);
		tree_prep_threads[i] = std::thread(tree_preparation, i * (number_of_keys_per_thread), fin);
	}
	for(int64_t i = 0; i < th; i++){
		tree_prep_threads[i].join();
	}
}

void run_benchmark () {
    std::thread t[threads];
    int64_t numa1 = 0, numa2 = 40, numa3 = 80, numa4 = 120;
    for (int64_t i = 0; i < threads; i++) {
        tho[i] = 0;
        t[i] = std::thread (run, i);
        cpu_set_t cpuset;
        int64_t cpu_pin;
        if(i < 80){
            if(i % 2 == 0){
                cpu_pin = numa1;
                numa1++;
            }
            else{
                cpu_pin = numa2;
                numa2++;
            }
        }
        else{
            if(i % 2 == 0){
                cpu_pin = numa3;
                numa3++;
            }
            else{
                cpu_pin = numa4;
                numa4++;
            }
        }
        CPU_ZERO(&cpuset);
        CPU_SET(cpu_pin, &cpuset);
        pthread_setaffinity_np (t[i].native_handle (),
                                sizeof (cpu_set_t), &cpuset);
    }
    while (ready_threads < threads) sleep (1);
    running = true;
    sleep(Runtime);
    running = false;
    for (int64_t i = 0; i < threads; ++i)
        t[i].join ();
    uint64_t throughput = 0;
    for (int64_t i = 0; i < threads; i++)
        throughput += tho[i];
	std::cout<< threads << " " << Read <<    " "   << Insert << " "  << Delete << " "  << RangeQuery << " " << RangeQuerySize << " " << table_size << " " << DatasetSize << " "
			 << throughput / Runtime << "\n";
	fout<< threads << " " << Read <<    " "   << Insert << " "  << Delete << " "  << RangeQuery << " " << RangeQuerySize << " " << table_size << " " << DatasetSize << " " <<
		throughput / Runtime << "\n";
}

void file_reader(int64_t tid){
	std::ifstream reader;
	reader.open("../DatasetGen/Data/" + std::to_string(DatasetSize/1000000)+ "M/Data_Split_" + std::to_string(tid), std::ios::in);
	std::string number;
	while(reader >> number){
		dataset_split[tid].push_back(std::stoll(number));
	}
	reader.close();
}

void read_from_files(){
	int th = 160;
	std::vector<std::thread> reader_threads(th);
	int64_t number_of_keys_per_thread = DatasetSize/th;
	for(int64_t i = 0; i < th; i++){
		int64_t fin = std::min((i + 1) * number_of_keys_per_thread, DatasetSize);
		reader_threads[i] = std::thread(file_reader, i);
	}
	for(int64_t i = 0; i < th; i++){
		reader_threads[i].join();
	}
	for(int i = 0; i < th; i++){
		dataset.insert(dataset.end(), dataset_split[i].begin(), dataset_split[i].end());
	}
}