#include <iostream>
#include <algorithm>
#include <vector>
#include <thread>
#include <random>
#include <fstream>
#include <cassert>

void prepare_dataset();
void write_to_files();
void read_from_files();
std::vector<uint64_t> dataset;
int64_t dataset_prep_threads;
int64_t DatasetSize;

int main(int argc, char** argv) {
	if(argc <= 2){
		std::cout<<"Invalid Arguments\n";
		return 0;
	}
	dataset_prep_threads = atoi(argv[1]);
	DatasetSize = atoi(argv[2]);
	assert(DatasetSize % 1000000 == 0);
	dataset = std::vector<uint64_t> (DatasetSize);
	prepare_dataset();
	std::random_device rd;
	std::mt19937 gen(rd());
	std::shuffle(begin(dataset), end(dataset), gen);
	std::cout << "A randomised dataset of size : " << dataset.size() << " has been created"
			  << std::endl;
	std::cout << "Beginning to write it to disk\n";
	write_to_files();
	dataset = std::vector<uint64_t> (DatasetSize);
	std::cout << "Reading the files from disk to confirm it read the entire dataset\n";
	read_from_files();
	std::cout << dataset.size() << std::endl;
}

void dataset_prep(int64_t tid, int64_t start, int64_t fin){
	for(int64_t i = start; i < fin; i++){
		dataset[i] = i;
	}
}

void prepare_dataset(){
	std::vector<std::thread> dataset_threads(dataset_prep_threads);
	int64_t number_of_keys_per_thread = DatasetSize/dataset_prep_threads;
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		int64_t fin = std::min((i + 1) * number_of_keys_per_thread, DatasetSize);
		dataset_threads[i] = std::thread(dataset_prep, i, i * (number_of_keys_per_thread), fin);
	}
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		dataset_threads[i].join();
	}
}

void file_writer(int64_t tid, int64_t start, int64_t fin){
	std::ofstream out;
	out.open("Data/" + std::to_string(DatasetSize/1000000)+ "M/Data_Split_" + std::to_string(tid), std::ios::out);
	if(!out.is_open())
	{
		std::cout<<"Error:: Dataset directory does not exists \n";
		exit(1);
	}
	for(int64_t i = start; i < fin; i++){
		out << dataset[i] << "\n";
	}
	out.close();
}

void write_to_files(){
	std::vector<std::thread> writer_threads(dataset_prep_threads);
	int64_t number_of_keys_per_thread = DatasetSize/dataset_prep_threads;
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		int64_t fin = std::min((i + 1) * number_of_keys_per_thread, DatasetSize);
		writer_threads[i] = std::thread(file_writer, i, i * (number_of_keys_per_thread), fin);
	}
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		writer_threads[i].join();
	}
}

void file_reader(int64_t tid, int64_t start, int64_t fin){
	std::ifstream reader;
	reader.open("Data/" + std::to_string(DatasetSize/1000000)+ "M/Data_Split_" + std::to_string(tid), std::ios::in);
	std::string number;
	int64_t index = start;
	while(reader >> number){
		dataset[index] = std::stoll(number);
	}
	reader.close();
}

void read_from_files(){
	std::vector<std::thread> reader_threads(dataset_prep_threads);
	int64_t number_of_keys_per_thread = DatasetSize/dataset_prep_threads;
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		int64_t fin = std::min((i + 1) * number_of_keys_per_thread, DatasetSize);
		reader_threads[i] = std::thread(file_reader, i, i * (number_of_keys_per_thread), fin);
	}
	for(int64_t i = 0; i < dataset_prep_threads; i++){
		reader_threads[i].join();
	}
}