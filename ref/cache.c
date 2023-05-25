
#include <math.h>
#include <stdlib.h>
#include <stdint.h>
#include <cstdlib>
#include <iostream>
#include <fstream>
#include <sstream>


/* we want to have a struct of a line\block/
 * tag to identify
 * valid bit - 0 is empty, 1 if data exist in this line 
 * dirty- if we need to update low memory levels -1 , else -0
 *LRU number: an number between 0 to (ways_num -1 ). The oldest gets 0 (meaning its the lru)
 * */

typedef struct {
	unsigned long int tag;
	int lru_line;
	bool dirty;
	bool valid;
} line;

/* we want to have a struct of one way: a pointer to an arrays of lines
 * */
 
typedef struct {
	
	line *set_table;
	
} way;

/* we want to have a struct of a cache level
 * pointer to an arrays of ways
 * miss and hit counters to the current cache level
 * wr_alloc -1 if write allocate policy , else -0
 *l_cyc - number of cycle to reach the cache
 * */
typedef struct {
	
	way *ways;
	double miss_counter;
	double hit_counter;
	int block_size;
	int mem_cyc;
	int wr_alloc;
	int l_size;
	int ways_num;
	int l_cyc;
	int lines_num;
	bool is_L1;

} L;


L L_1;
L L_2;

void invalid_line(unsigned long int address, L* l_cache );
int get_way(unsigned long int tag, int set,L* l_cache );
int cache_init(int MemCyc, int BSize, int L1Size, int L2Size, int L1Assoc, int L2Assoc, int L1Cyc, int L2Cyc,  int WrAlloc);
void write(unsigned long int address);
void read(unsigned long int address);
void insert(unsigned long int address, L* l_cache);
void update_lru(L* l_cache, int set, int lru_way_idx);
bool search(unsigned long int address, L* l_cache);
int get_lru(L* l_cache, int set);
int get_set(unsigned long int address, int bsize, int sets_num );
unsigned long int get_tag(unsigned long int address, int bsize, int sets_num );
void GetStats(double *L1MissRate, double *L2MissRate,double *avgAccTime, int instruction_num);
void update_dirty_bit(unsigned long int address, L* l_cache );
unsigned long int build_address(unsigned long int org_tag, int set,L* l_cache );
bool check_dirty_bit(unsigned long int address, L* l_cache );
void update_lru_invalid(L* l_cache, int set, int lru_way_idx );


int cache_init(int MemCyc, int BSize, int L1Size, int L2Size, int L1Assoc, int L2Assoc, 
			int L1Cyc, int L2Cyc,  int WrAlloc){
	
	L_2.ways_num =pow(2,L2Assoc );
	L_1.ways_num =pow(2,L1Assoc );
	
	L_1.ways = (way *)malloc(sizeof(way)*L_1.ways_num);
	if(L_1.ways == NULL){
		return -1;
	}
	L_2.ways = (way *)malloc(sizeof(way)*L_2.ways_num);
	if(L_2.ways == NULL){
		return -1;
	}
	int L_1_total_lines = (pow(2,L1Size ))/(pow(2,BSize ));
	L_1.lines_num =  L_1_total_lines / (pow(2,L1Assoc ));
	int L_2_total_lines = (pow(2,L2Size ))/(pow(2,BSize ));
	L_2.lines_num =  L_2_total_lines / (pow(2,L2Assoc ));
	for (int i=0 ; i<L_1.ways_num; i++ ){
		L_1.ways[i].set_table = (line *)malloc(sizeof(line)*L_1.lines_num);
		if(L_1.ways[i].set_table == NULL){
			return -1;
		}	
		for(int j=0; j<L_1.lines_num; j++){
			L_1.ways[i].set_table[j].lru_line = 0;
			L_1.ways[i].set_table[j].dirty = 0;
			L_1.ways[i].set_table[j].valid = 0;
			L_1.ways[i].set_table[j].tag = 0;
		}
	

	}
	for (int i=0 ; i<L_2.ways_num; i++ ){
		L_2.ways[i].set_table= (line *)malloc(sizeof(line)*L_2.lines_num);
		if(L_2.ways[i].set_table == NULL){
			return -1;
		}
		for(int j=0; j<L_2.lines_num; j++){
			L_2.ways[i].set_table[j].lru_line = 0;
			L_2.ways[i].set_table[j].dirty = 0;
			L_2.ways[i].set_table[j].valid = 0;
			L_2.ways[i].set_table[j].tag = 0;
		}

	}
	
	L_1.miss_counter=0;
	L_1.hit_counter=0;
	L_1.block_size = BSize;
	L_1.mem_cyc= MemCyc;
	L_1.wr_alloc= WrAlloc;
	L_1.l_size=L1Size ;
	L_1.l_cyc = L1Cyc;
	L_1.is_L1 = true;
	L_2.miss_counter=0;
	L_2.hit_counter=0;
	L_2.block_size = BSize;
	L_2.mem_cyc= MemCyc;
	L_2.wr_alloc= WrAlloc;
	L_2.l_size=L2Size ;
	L_2.l_cyc = L2Cyc;
	L_2.is_L1 = false;
	return 0;
}

void write(unsigned long int address){
	int wr_alloc = L_1.wr_alloc;
	bool data_exist_l1 = search(address, &L_1);
	if (data_exist_l1){
		L_1.hit_counter++;
		int set = get_set(address,L_1.block_size ,L_1.lines_num);
		unsigned long int tag = get_tag(address,L_1.block_size ,L_1.lines_num); 
		int way_idx = get_way( tag, set,&L_1 );
		update_lru(&L_1,  set,  way_idx );
		update_dirty_bit(address, &L_1);
		printf("write L1 HIT\n");
		return;
	}
	//the data doesnt exist in L1:
	L_1.miss_counter++;
	printf("write L1 MISS\n");
	bool data_exist_l2 = search(address, &L_2);
	if (data_exist_l2){
		L_2.hit_counter++;
		int set = get_set(address,L_2.block_size ,L_2.lines_num);
		unsigned long int tag = get_tag(address,L_2.block_size ,L_2.lines_num); 
		int way_idx = get_way( tag, set,&L_2 );
		update_lru(&L_2,  set,  way_idx );
		update_dirty_bit(address, &L_2);
		//if is Write allocate - we should immidiatly update L1
		if(wr_alloc){
			insert(address,&L_1);
			update_dirty_bit(address, &L_1);
		}
		
		printf("write L2 HIT\n");
		return;
	}
	L_2.miss_counter++;
	
	printf("write L2 MISS\n");
	if(wr_alloc){
		insert(address,&L_2);
		insert(address,&L_1);
		update_dirty_bit(address, &L_1);
	}
}

/* read function - if the memory instruction is 'r'
 * */
void read(unsigned long int address){
	bool data_exist_l1 = search(address, &L_1);
	if (data_exist_l1){
		L_1.hit_counter++;
		int set = get_set(address,L_1.block_size ,L_1.lines_num);
		unsigned long int tag = get_tag(address,L_1.block_size ,L_1.lines_num); 
		int way_idx = get_way( tag, set,&L_1 );
		update_lru(&L_1,  set,  way_idx );
		return;
	}
	L_1.miss_counter++;
	bool data_exist_l2 = search(address, &L_2);
	if (data_exist_l2){
		L_2.hit_counter++;
		int set = get_set(address,L_2.block_size ,L_2.lines_num);
		unsigned long int tag = get_tag(address,L_2.block_size ,L_2.lines_num); 
		int way_idx = get_way( tag, set,&L_2 );
		update_lru(&L_2,  set,  way_idx );
		insert(address,&L_1);
		return;
	}
	L_2.miss_counter++;
	insert(address,&L_2);
	insert(address,&L_1);
}
int get_way(unsigned long int tag, int set,L* l_cache ){
	for (int i=0; i< l_cache->ways_num; i++){
		if(l_cache->ways[i].set_table[set].tag == tag){
			return i;
		}
	}
	return -1;
}
void update_dirty_bit(unsigned long int address, L* l_cache ){
	int set = get_set(address,l_cache->block_size ,l_cache->lines_num);
	unsigned long int tag = get_tag(address,l_cache->block_size ,l_cache->lines_num); 
	int way = get_way( tag, set, l_cache );
	l_cache->ways[way].set_table[set].dirty =1; //update dirty bit

}

bool check_dirty_bit(unsigned long int address, L* l_cache ){
	int set = get_set(address,l_cache->block_size ,l_cache->lines_num);
	unsigned long int tag = get_tag(address,l_cache->block_size ,l_cache->lines_num); 
	int way = get_way( tag, set, l_cache );
	return l_cache->ways[way].set_table[set].dirty; //update dirty bit

}
//if we ovveride data in l2 we need to invalid the data in l1 because the inclusive law
void invalid_line(unsigned long int address, L* l_cache ){
	bool data_exist_l1 = search(address, &L_1);
	if (data_exist_l1){
		int set = get_set(address,l_cache->block_size ,l_cache->lines_num);
		unsigned long int tag = get_tag(address,l_cache->block_size ,l_cache->lines_num); 
		int way = get_way( tag, set, l_cache );
		l_cache->ways[way].set_table[set].valid =0; //update valid bit
		l_cache->ways[way].set_table[set].dirty = 0;
		update_lru_invalid(l_cache, set,way );
		
	}
	return;

}
void insert(unsigned long int address, L* l_cache ){
	int set = get_set(address,l_cache->block_size ,l_cache->lines_num);
	unsigned long int tag = get_tag(address,l_cache->block_size ,l_cache->lines_num); 
	//******* we want to check if the address is already inside, if so only update lru
	bool exists = search(address,l_cache);
	if(exists){
		int way = get_way(tag,set,l_cache);
		update_lru(l_cache,  set,  way );
		return;
	}
	//*****************************
	int lru_way_idx = get_lru(l_cache, set);
	//if the line exist in L_1 and dirty - we need to update L2 with the data
	if(l_cache->is_L1 && l_cache->ways[lru_way_idx].set_table[set].dirty){
		unsigned long int org_tag = l_cache->ways[lru_way_idx].set_table[set].tag;
		//we need to sent the original address that was saved
		unsigned long int org_address = build_address(org_tag,set, &L_1);
		insert(org_address, &L_2);
	}
	else if((l_cache->is_L1 == false) && (l_cache->ways[lru_way_idx].set_table[set].valid==1)) {
		unsigned long int org_tag = l_cache->ways[lru_way_idx].set_table[set].tag;
		//we need to sent the original address that was saved
		unsigned long int org_address = build_address(org_tag,set, &L_2);
		invalid_line(org_address, &L_1);
	}
	l_cache->ways[lru_way_idx].set_table[set].dirty=0;
	l_cache->ways[lru_way_idx].set_table[set].tag = tag; //update tag
	l_cache->ways[lru_way_idx].set_table[set].valid =1; //update valid bit
	update_lru(l_cache,  set,  lru_way_idx );
	
}

unsigned long int build_address(unsigned long int org_tag, int set,L* l_cache ){
	int set_size = (int)log2(l_cache->lines_num);
	unsigned long int tag = org_tag << set_size;
	unsigned long int  address = (tag | set) << l_cache->block_size;
	
	return address;
}

void update_lru(L* l_cache, int set, int lru_way_idx ){
	int lru_way_value = l_cache->ways[lru_way_idx].set_table[set].lru_line;
	
	for (int i=0; i< l_cache->ways_num; i++){
		if( i==lru_way_idx ){
			l_cache->ways[i].set_table[set].lru_line = l_cache->ways_num -1 ;
		}
		else if (lru_way_value < l_cache->ways[i].set_table[set].lru_line ){
			l_cache->ways[i].set_table[set].lru_line--;
		}
	
	}
}

void update_lru_invalid(L* l_cache, int set, int way ){
	int lru_way_value = l_cache->ways[way].set_table[set].lru_line;
	
	for (int i=0; i< l_cache->ways_num; i++){
		if( i==way ){
			l_cache->ways[i].set_table[set].lru_line = 0 ;
		}
		else if (lru_way_value > l_cache->ways[i].set_table[set].lru_line ){
			if(l_cache->ways[i].set_table[set].valid){
				l_cache->ways[i].set_table[set].lru_line++;
			}
		}
	
	}
}

//find which way for this set is the lru
int get_lru(L* l_cache, int set){
	for (int i=0; i< l_cache->ways_num; i++){
		if(l_cache->ways[i].set_table[set].lru_line == 0){
			return i;
				
		}
	}
	return -1;
}
/* search function - just searching if we have this data in L1/L2
 * */
bool search(unsigned long int address, L* l_cache){
	int set = get_set(address,l_cache->block_size ,l_cache->lines_num);
	unsigned long int tag = get_tag(address,l_cache->block_size ,l_cache->lines_num);
	for (int i=0; i< l_cache->ways_num; i++){
		if(l_cache->ways[i].set_table[set].valid == 1){
			if (l_cache->ways[i].set_table[set].tag == tag){
				return true;
			}
		}
	}
	return false;
}


int get_set(unsigned long int address, int bsize, int sets_num ){
	
	int shift =bsize;
	unsigned mask = (1 << (int)log2(sets_num)) - 1;
	if(sets_num == 1){
		return 0;
	}
	
	int set = (address >> shift) & mask;
	
	return set;
}

unsigned long int get_tag(unsigned long int address, int bsize, int sets_num ){
	
	int shift =bsize+ (int)log2(sets_num); 
	unsigned long int tag = (address >> shift );
	return tag;	
}

void GetStats(double *L1MissRate, double *L2MissRate,double *avgAccTime, int instruction_num){
	*(L1MissRate) = L_1.miss_counter / (L_1.miss_counter +L_1.hit_counter);
	*(L2MissRate) =	L_2.miss_counter / (L_2.miss_counter +L_2.hit_counter);	
	// tot_time = hit_l1 * l1_cyc + miss_l1 *(cyc_1 +cyc_2) + miss*l2 *( cyc_memory)
	double total_time= (L_1.l_cyc *L_1.hit_counter) + (L_1.miss_counter * (L_1.l_cyc +L_2.l_cyc)) + (L_2.miss_counter *L_2.mem_cyc) ;
	*(avgAccTime) = total_time / (double)instruction_num;
	
	for (int i=0 ; i<L_1.ways_num; i++ ){
		free(L_1.ways[i].set_table);
	}
	
	free(L_1.ways);
	
	for (int i=0 ; i<L_2.ways_num; i++ ){
		free(L_2.ways[i].set_table);
	}
	
	free(L_2.ways);
}






