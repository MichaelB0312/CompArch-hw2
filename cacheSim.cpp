/* 046267 Computer Architecture - Winter 20/21 - HW #2 */

#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>
#include <maths>

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
	bool valid_bit;
	bool dirty_bit;
} way;
	
typedef struct {
	way* ways; 
} level_row;
	
typedef struct {
	unsigned block_size;
	unsigned set_size;
	unsigned offset_size;
	unsigned tag_size;
	
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
	
	

	if(init_cache(BSize,L1Size, L2Size, L1Assoc, L2Assoc, L1Cyc, L2Cyc, WrAlloc) == -1) 
		return -1;

	int instr_num = 0; //count number of input instructions
	double time_access = 0.0;//total missing time
	int	L1_miss_num = 0, L2_miss_num = 0;
	
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
		// converts a hexadecimal string representation of an unsigned integer into a value
		num = strtoul(cutAddress.c_str(), NULL, 16);

		// DEBUG - remove this line
		cout << " (dec) " << num << endl;

		
		//treating read case:
		if (operation == 'r') {
			read_func(&time_access,&L1_miss_num,&L2_miss_num, num, MemCyc);
		}
		// treating write case
		else if (operation == 'w') {
			write_func(&time_access,&L1_miss_num,&L2_miss_num, num);
		}
		else
		{
			return -1;
		}

		instr_num++;
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
	
	// origin sizes are in log2, so convert it
	BSize_pow = pow(2,BSize);
	L1Size = pow(2,L1Size);
	L2Size = pow(2,L2Size);
	L1Assoc = pow(2,L1Assoc);
	L2Assoc = pow(2,L2Assoc);	
	
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

	if((BSize_pow << 30) != 0) return -1; //check if divides into 4
	if((L1Size << 30) != 0) return -1;//check if divides into 4
	if((L2Size << 30) != 0) return -1;//check if divides into 4
	L1->block_size = BSize;
	L2->block_size = BSize;
	if(L1Size%BSize_pow != 0) return -1; //check if no leftovers
	if(L2Size%BSize_pow != 0) return -1; //check if no leftovers
	int num_rows1_per_way = (L1Size/BSize_pow)/L1Assoc;
	int num_rows2_per_way = (L2Size/BSize_pow)/L2Assoc;
		
	if((L1Assoc << 31) != 0) return -1; //check if divides into 2
	L1->num_ways = L1Assoc;
	if((L2Assoc << 31) != 0) return -1; //check if divides into 2
	L2->num_ways = L2Assoc;
		
	if(num_rows1%L1Assoc != 0) return -1;
	L1->level_rows = new level_row[num_rows1_per_way];
	if(num_rows2%L2Assoc != 0) return -1;
	L2->level_rows = new level_row[num_rows2_per_way];
	if(!(L1->level_rows) || !(L2->level_rows)) return -1;
	
	L1->set_size = log2(num_rows1_per_way);
	L1->offset_size = BSize;
	L1->tag_size = 32 - (BSize + log2(num_rows1_per_way));
		
	for( int i=0; i<num_rows1_per_way ; i++){
		(L1->level_rows[i]).ways = new way[L1Assoc];
		if(!((L1->level_rows[i]).ways)) return -1;
		memset((L1->level_rows[i]).ways, 0, sizeof(way)*L1Assoc);
	}
	
	L2->set_size = log2(num_rows2_per_way);
	L2->offset_size = BSize;
	L2->tag_size = 32 - (BSize + log2(num_rows2_per_way));
	
	for( int i=0; i<num_rows2_per_way ; i++){
		(L2->level_rows[i]).ways = new way[L1Assoc];
		if (!((L2->level_rows[i]).ways)) return -1;
		memset((L2->level_rows[i]).ways, 0, sizeof(way) * L1Assoc);
	}
		
	return 0;
}


/*
 * read_func - doing the reading opearation, return 0 on success.
 * param[out] time_access- aggregating time access of the current line instruction
 * param[out] L2,L1_miss_num - increase it if there is a miss in L2,L1 cache
 * 
*/
int read_func(double *time_access,int *L1_miss_num,int *L2_miss_num, unsigned long int num, unsigned MemCyc) {
	unsigned adr_mask = -1;
	adr_mask = adr_mask >> (32 - L1->offset_size);
	unsigned adr_offset = num & adr_mask ;
	//set of address L1
	unsigned num_shift = num >> L1->offset_size;
	adr_mask = -1;
	adr_mask = adr_mask >> (32 - L1->set_size);
	unsigned adr_set_L1 = num_shift & adr_mask;
	//set of address L2
	adr_mask = -1;
	adr_mask = adr_mask >> (32 - L2->set_size);
	unsigned adr_set_L2 = num_shift & adr_mask;

	//tag address L1 & L2
	unsigned num_shift_tag1 = num_shift >> L1->set_size;
	unsigned num_shift_tag2 = num_shift >> L2->set_size;
	unsigned adr_mask_tag1 = -1;
	unsigned adr_mask_tag2 = -1;
	adr_mask_tag1 = adr_mask >> (32 - L1->tag_size);
	adr_mask_tag2 = adr_mask >> (32 - L2->tag_size);
	unsigned adr_tag_L1 = num_shift_tag1 & adr_mask_tag1;
	unsigned adr_tag_L2 = num_shift_tag2 & adr_mask_tag2;
	
	//check tag of L1
	int level_num = 1;
	for (int i=0; i < L1->num_ways ; i++){
		if ( (adr_tag_L1 == (L1->level_rows[adr_set_L1]).ways[i].tag) &&
				((L1->level_rows[adr_set_L1]).ways[i].valid_bit == 1) ){
			//hit read:
			// update LRU
			// update time access
			if ( hit(adr_set_L1, i, level_num, &time_access) != 0)
				return -1;
			return 0;
		}
	}
	//check tag of L2
	level_num = 2;
	for (int i=0; i < L2->num_ways ; i++){
		if ( (adr_tag_L2 == (L2->level_rows[adr_set_L2]).ways[i].tag) &&
			((L2->level_rows[adr_set_L2]).ways[i].valid_bit == 1) ) {
			//hit read:
			// update LRU
			// update time access
			if ( hit(adr_set_L2, i, level_num, &time_access) != 0)
				return -1;
			return 0;
		}
	}
	
	//read MISS:
	// to memory
	// bring to L2 - update tag
	level_num = 2;
	bool flag_updated_L2 = false;
	*time_access += MemCyc + L2->time_access;
	for (int i=0; i < L2->num_ways ; i++){ // check if empty space
		if (L2->level_rows[adr_set_L2]).ways[i].valid_bit == false){
			(L2->level_rows[adr_set_L2]).ways[i].tag = adr_tag_L2;
			L2->level_rows[adr_set_L2]).ways[i].valid_bit = true;
			update_LRU(level_num, adr_set_L2, i);
			flag_updated = true;
			break;
		} 
	}
	if (flag_updated_L2 == false){ // if no empty, find smallest LRU
		for (int i=0; i < L2->num_ways ; i++){
			if ( L2->level_rows[adr_set_L2]).ways[i].LRU_state == 0){
				// check if dirty_bit
				if ( L2->level_rows[adr_set_L2]).ways[i].dirty_bit == true ){
					*time_access += MemCyc;
					L2->level_rows[adr_set_L2]).ways[i].dirty_bit = false;
				}
				// update valid bit to 1
				L2->level_rows[adr_set_L2]).ways[i].valid_bit = true;
				// is the line P in L1 -- if yes, delete from L1
				for( int s=0 ; s < pow(2,L1->set_size);s++){ //check for each set
					for (int w=0 ; w < L1->num_ways; w++){ // check for each way in each set
						//check if same tag
						if( (L1->level_rows[s]).ways[w].tag == (L2->level_rows[adr_set_L2]).ways[i].tag ){
							// if same tag, check if dirty
							if ( L1->level_rows[s]).ways[w].dirty_bit == true ){
								*time_access += MemCyc;
								(L1->level_rows[s]).ways[w].dirty_bit = false;
							}
							// make line invalid in L1
							(L1->level_rows[s]).ways[w].valid_bit == false;
							// update LRU - make it the OLDEST, because it is invalid
							update_LRU(1, s, w);
						}
					}
				}
				//update tag
				(L2->level_rows[adr_set_L2]).ways[i].tag = adr_tag_L2;
				update_LRU(level_num, adr_set_L2, i);
				flag_updated = true;
				break;
			}
		}
	}
	
	
	// bring to L1 - update tag
	level_num = 1;
	bool flag_updated_L1 = false;
	*time_access += L1->time_access;
	for (int i=0; i < L1->num_ways ; i++){
		if (L1->level_rows[adr_set_L1]).ways[i].valid_bit == false){
			// no need to override
			(L1->level_rows[adr_set_L1]).ways[i].tag = adr_tag_L1;
			L1->level_rows[adr_set_L1]).ways[i].valid_bit = true;
			update_LRU(level_num, adr_set_L1, i);
			flag_updated = true;
			break;
		} 
	}
	if (flag_updated_L1 == false){ // if no empty, find smallest LRU
		for (int i=0; i < L1->num_ways ; i++){
			if ( L1->level_rows[adr_set_L1]).ways[i].LRU_state == 0){
				// override line in L1
				//check if dirty 
				if ( L1->level_rows[adr_set_L1]).ways[i].dirty_bit == true ){
					*time_access += MemCyc;
					(L1->level_rows[adr_set_L1]).ways[i].dirty_bit = false;
					//find in L2 - mark dirty & update LRU
					for( int s=0 ; s < pow(2,L2->set_size);s++){ //check for each set
						for (int w=0 ; w < L2->num_ways; w++){ // check for each way in each set
							//check if same tag
							if( (L2->level_rows[s]).ways[w].tag == (L1->level_rows[adr_set_L1]).ways[i].tag ){
								// if same tag, mark as dirty
								(L2->level_rows[s]).ways[w].dirty_bit = true;
								//(L2->level_rows[s]).ways[w].valid_bit == true; // NOT SURE IF NEEDED!
								// update LRU
								update_LRU(2, s, w);
							}
						}
					}
				}
				(L1->level_rows[adr_set_L1]).ways[i].tag = adr_tag_L1;
				update_LRU(level_num, adr_set_L1, i);
				flag_updated = true;
				break;
			}
		}
	}
	//update time
	
	return 0;
}
	
	
int hit(unsigned adr_set, unsigned way_num, unsigned level_num, double *time_access){
	//hit read:
	// update LRU
	update_LRU(level_num, adr_set, way_num);
	// update time access
	if (level_num == 1){		
		*time_access += L1->time_access;
	} else if (level_num == 2){
		*time_access += L2->time_access;
	}
	return 0;
}

void update_LRU(unsigned level_num, unsigned adr_set, unsigned curr_way){
	if (level_num == 1){
		if ( L1->level_rows[adr_set]).ways[curr_way].valid_bit == false ){
			L1->level_rows[adr_set]).ways[curr_way].LRU_state = 0;
			return;
		}
		int counter = L1->level_rows[adr_set]).ways[curr_way].LRU_state;
		L1->level_rows[adr_set]).ways[curr_way].LRU_state = L1->num_ways - 1; // biggest LRU state = newest
		for (int i=0; i < L1->num_ways ; i++){
			// update LRU
			if ( (i != curr_way) && ((L1->level_rows[adr_set]).ways[i].LRU_state > counter)){
				L1->level_rows[adr_set]).ways[i].LRU_state--;
			}
		}
	} else if (level_num == 2){
		if ( L2->level_rows[adr_set]).ways[curr_way].valid_bit == false ){
			L2->level_rows[adr_set]).ways[curr_way].LRU_state = 0;
			return;
		}
		int counter = L2->level_rows[adr_set]).ways[curr_way].LRU_state;
		L2->level_rows[adr_set]).ways[curr_way].LRU_state = L2->num_ways - 1;
		for (int i=0; i < L2->num_ways ; i++){
			// update LRU
			if ( (i != curr_way) && (L2->level_rows[adr_set]).ways[i].LRU_state > counter){
				L2->level_rows[adr_set]).ways[i].LRU_state--;
			}
		}
	}
	return;
}
