/* 046267 Computer Architecture - Winter 20/21 - HW #2 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>

using std::FILE;
using std::string;
using std::cout;
using std::endl;
using std::cerr;
using std::ifstream;
using std::stringstream;	
	
typedef struct {
	unsigned tag;
	unsigned LRU_state;
	unsigned valid_bit;
	unsigned dirty_bit;
} way;
	
typedef struct {
	unsigned set;
	unsigned offset;
	way* ways; 
} level_row;
	
typedef struct {
	unsigned block_size;
	unsigned num_ways;
	unsigned time_access;
	bool is_write_allocate;
	
	level_row* level_rows;
} one_level;
	
one_level *L1;
one_level *L2;
	

int main(int argc, char **argv) {

	if (argc < 19) {
		cerr << "Not enough arguments" << endl;
		return 0;
	}

	// Get input arguments

	// File
	// Assuming it is the first argument
	char* fileString = argv[1];
	ifstream file(fileString); //input file stream
	string line;
	if (!file || !file.good()) {
		// File doesn't exist or some other error
		cerr << "File not found" << endl;
		return 0;
	}

	unsigned MemCyc = 0, BSize = 0, L1Size = 0, L2Size = 0, L1Assoc = 0,
			L2Assoc = 0, L1Cyc = 0, L2Cyc = 0, WrAlloc = 0;

	for (int i = 2; i < 19; i += 2) {
		string s(argv[i]);
		if (s == "--mem-cyc") {
			MemCyc = atoi(argv[i + 1]);
		} else if (s == "--bsize") {
			BSize = atoi(argv[i + 1]);
		} else if (s == "--l1-size") {
			L1Size = atoi(argv[i + 1]);
		} else if (s == "--l2-size") {
			L2Size = atoi(argv[i + 1]);
		} else if (s == "--l1-cyc") {
			L1Cyc = atoi(argv[i + 1]);
		} else if (s == "--l2-cyc") {
			L2Cyc = atoi(argv[i + 1]);
		} else if (s == "--l1-assoc") {
			L1Assoc = atoi(argv[i + 1]);
		} else if (s == "--l2-assoc") {
			L2Assoc = atoi(argv[i + 1]);
		} else if (s == "--wr-alloc") {
			WrAlloc = atoi(argv[i + 1]);
		} else {
			cerr << "Error in arguments" << endl;
			return 0;
		}
	}

	unsigned BSize_pow = pow(2,BSize);
	unsigned L1Size_pow = pow(2,L1Size);
	unsigned L2Size_pow = pow(2,L2Size);
	unsigned L1Assoc_pow = pow(2,L1Assoc);
	unsigned L2Assoc_pow = pow(2,L2Assoc);	
	unsigned L2Assoc_pow = pow(2,L2Assoc);	
	if(init_cache(BSize,L1Size, L2Size, L1Assoc, L2Assoc, L1Cyc, L2Cyc, WrAlloc) == -1) 
		return -1;


	
	while (getline(file, line)) {

		stringstream ss(line);
		string address;
		char operation = 0; // read (R) or write (W)
		if (!(ss >> operation >> address)) {
			// Operation appears in an Invalid format
			cout << "Command Format error" << endl;
			return 0;
		}

		// DEBUG - remove this line
		cout << "operation: " << operation;

		string cutAddress = address.substr(2); // Removing the "0x" part of the address

		// DEBUG - remove this line
		cout << ", address (hex)" << cutAddress;

		unsigned long int num = 0;
		num = strtoul(cutAddress.c_str(), NULL, 16);

		// DEBUG - remove this line
		cout << " (dec) " << num << endl;

	}

	double L1MissRate;
	double L2MissRate;
	double avgAccTime;

	printf("L1miss=%.03f ", L1MissRate);
	printf("L2miss=%.03f ", L2MissRate);
	printf("AccTimeAvg=%.03f\n", avgAccTime);

	return 0;
}


int init_cache(unsigned BSize,unsigned L1Size, unsigned L2Size, unsigned L1Assoc,
			unsigned L2Assoc,unsigned L1Cyc,unsigned L2Cyc,unsigned WrAlloc){
	L1 = new one_level;
	L2 = new one_level;
	if(!L1 || !L2) return -1;
	
	L1->time_access = L1Cyc;
	L2->time_access = L2Cyc;
		
	if(WrAlloc == 0){
		L1->is_write_allocate = false;
		L2->is_write_allocate = false;
	} else if (WrAlloc == 1){
		L1->is_write_allocate = true;
		L2->is_write_allocate = true;
	} else {
		return -1;
	}

	if((BSize << 30) != 0) return -1; //check if divides into 4
	if((L1Size << 30) != 0) return -1;//check if divides into 4
	if((L2Size << 30) != 0) return -1;//check if divides into 4
	L1->block_size = BSize;
	L2->block_size = BSize;
	if(L1Size%BSize_pow != 0) return -1; //check if no leftovers
	if(L2Size_pow%BSize != 0) return -1; //check if no leftovers
	int num_rows1_per_way = (L1Size/BSize)/L1Assoc;
	int num_rows2_per_way = (L2Size_pow/BSize)/L2Assoc;
		
	if((L1Assoc << 31) != 0) return -1; //check if divides into 2
	L1->num_ways = L1Assoc;
	if((L2Assoc << 31) != 0) return -1; //check if divides into 2
	L2->num_ways = L2Assoc;
		
	if(num_rows1%L1Assoc != 0) return -1;
	L1->level_rows = new level_row[num_rows1_per_way];
	if(num_rows2%L2Assoc != 0) return -1;
	L2->level_rows = new level_row[num_rows2_per_way];
	if(!(L1->level_rows) || !(L2->level_rows)) return -1;
	
	for( int i=0; i<num_rows1_per_way ; i++){
		L1->level_rows[i]->set = 0;
		L1->level_rows[i]->offset = 0;
		L1->level_rows[i]->ways = new way[L1Assoc];
		if(!(L1->level_rows[i]->ways)) return -1;
		memset(L1->level_rows[i]->ways, 0, sizeof(way)*L1Assoc);
	}
	for( int i=0; i<num_rows2_per_way ; i++){
		L2->level_rows[i]->set = 0;
		L2->level_rows[i]->offset = 0;
		L2->level_rows[i]->ways = new way[L2Assoc];
		if(!(L2->level_rows[i]->ways)) return -1;
		memset(L2->level_rows[i]->ways, 0, sizeof(way)*L2Assoc);
	}
		
	return 0;
}
	
