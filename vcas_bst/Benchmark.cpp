#include "vcas_bst_impl.h"
#include "record_manager.h"
#include <random>
#include <thread>
#include <fstream>
#include <cassert>
#include <unistd.h>
#include <algorithm>
#include <vector>
using namespace vcas_bst_ns;

int64_t table_size;
int64_t DatasetSize;
int64_t threads;
double Read;
double Insert;
double Delete;
double RangeQuery;
int64_t RangeQuerySize;
int64_t Runtime = 1;
bool running = false;
std::atomic<size_t> ready_threads (0);
std::vector<uint64_t> tho;
std::vector<uint64_t> dataset;
int dataset_prep_threads;
std::ofstream fout;

#define test_type int64_t
#define RECLAIM reclaimer_debra<test_type>
#define ALLOC allocator_new_segregated<test_type>
#define POOL pool_none<test_type>
#define SIGQUIT 3
const test_type NO_VALUE = -1;
const test_type KEY_MIN = numeric_limits<int64_t>::min()+1;
const test_type KEY_MAX = numeric_limits<int64_t>::max()-1; // must be less than max(), because the snap collector needs a reserved key larger than this!
const int64_t TOTAL_THREADS = 200;

#define MEMMGMT_T record_manager<RECLAIM, ALLOC, POOL, Node<test_type, test_type> >
#define DS_DECLARATION vcas_bst<test_type, test_type, less<test_type>, MEMMGMT_T>
#define DS_CONSTRUCTOR new DS_DECLARATION(KEY_MAX, NO_VALUE, TOTAL_THREADS, SIGQUIT)
DS_DECLARATION *Tree;


void prepare_table ();
void run_benchmark ();
void prepare_dataset ();
void read_from_files();

int main(int argc, char** argv)
{
	fout.open("final_benchmarks.txt", std::ios::app);
	Tree = DS_CONSTRUCTOR;
	for(int i = 0; i < TOTAL_THREADS; i++){
		Tree -> initThread(i);
	}
	for(int i = 0; i < TOTAL_THREADS; i++){
		Tree -> deinitThread(i);
	}
	for(int i = 0; i < TOTAL_THREADS; i++){
		Tree -> initThread(i);
	}
	if(argc != 10)
	{
		std::cout<<"Invalid Arguments \n";
		return 0;
	}
	threads = atoi(argv[1]);
	Read = ((double)atoi(argv[2]))/100;
	Insert = ((double)atoi(argv[3]))/100;
	Delete = ((double)atoi(argv[4]))/100;
	RangeQuery = ((double)atoi(argv[5]))/100;
	RangeQuerySize = atol(argv[6]);
	table_size = atol(argv[7]);
	DatasetSize = atol(argv[8]);
	dataset_prep_threads = atoi(argv[9]);
	dataset = std::vector<uint64_t>(DatasetSize);
	if(threads == 0){
		std::cout<<"Error:: Threads cannot be Zero\n";
		return 0;
	}
	if(DatasetSize == 0)
	{
		std::cout<<"Error:: Dataset size cannot be zero\n";
		return 0;
	}
	printf("Parameters\nThreads: %ld\nRead: %lf\nInsert: %lf\nDelete: %lf\nRangeQuery: %lf\nRangeQuerySize: %ld\n"
		   "TableSize: %ld\nDatasetSize: %ld\n", threads, Read, Insert, Delete, RangeQuery, RangeQuerySize, table_size, DatasetSize);
	assert(((int64_t)(Read + Delete + Insert + RangeQuery)) == 1);
	tho.resize(threads, 0);
	read_from_files();
	std::cout<<"Dataset Prepared\n";
	if(table_size > 0)
		prepare_table ();
	std::cout<<"Table Prepared\n";
	run_benchmark ();
	fout.close();
}

void run (int64_t tid) {
	std::uniform_real_distribution<double> ratio_dis (0, 1);
	std::uniform_int_distribution<int64_t> index_dis(0, dataset.size() - 1);
	ready_threads++;
	std::random_device rd;
	std::mt19937 gen(rd());
	int64_t j = index_dis(gen);
	int64_t *keys = new int64_t[(uint64_t)RangeQuerySize];
	int64_t *values = new int64_t [(uint64_t)RangeQuerySize];
	while (!running);
	while (running) {
		double d = (double) ratio_dis (gen);
		if (d <= Read) {
			Tree -> find(tid + 1, (uint64_t)dataset[j]);   // read
		} else if (d <= Read + Delete) {
			Tree -> erase(tid + 1, dataset[j]);  // delete
		} else if (d <= Read + Delete + Insert) {
			Tree -> insert(tid + 1, dataset[j], j);  // insert
		}
		else{
			Tree ->rangeQuery(tid + 1, dataset[j], dataset[j] + RangeQuerySize, keys, values);
		}
		tho[tid]++;
		j = (j + 1)%dataset.size();
	}
}

void tree_preparation(int64_t tid, int64_t start, int64_t fin){
	for(int64_t i = start; i < fin ; i++) {
		Tree->insert(tid + 1, dataset[i], 10);
	}
}

void prepare_table () {
	std::vector<std::thread> tree_prep_threads(dataset_prep_threads);
	int64_t number_of_keys_per_thread = table_size/dataset_prep_threads;
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		int64_t fin = std::min((i + 1) * number_of_keys_per_thread, table_size);
		tree_prep_threads[i] = std::thread(tree_preparation, i, i * (number_of_keys_per_thread), fin);
	}
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		tree_prep_threads[i].join();
	}
}

void run_benchmark () {
	std::thread t[threads];
	//Thread pinning logic. Modify it according to your machine.
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

void file_reader(int64_t tid, int64_t start){
	std::ifstream reader;
	reader.open("../DatasetGen/Data/" + std::to_string(DatasetSize/1000000)+ "M/Data_Split_" + std::to_string(tid), std::ios::in);
	if(!reader.is_open())
	{
		std::cout<<"Error:: Dataset does not exist\n";
		exit(1);
	}
	std::string number;
	int64_t index = start;
	while(reader >> number){
		dataset[index] = std::stoll(number);
		index++;
	}
	reader.close();
}

void read_from_files(){
	std::vector<std::thread> reader_threads(dataset_prep_threads);
	int64_t number_of_keys_per_thread = DatasetSize/dataset_prep_threads;
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		int64_t begin = i * number_of_keys_per_thread;
		int64_t fin = std::min((i + 1) * number_of_keys_per_thread, DatasetSize);
		reader_threads[i] = std::thread(file_reader, i, begin);
	}
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		reader_threads[i].join();
	}
}