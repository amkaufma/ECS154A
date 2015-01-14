//Andrew Kaufman & Frank Chung

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>
using namespace std;

struct cacheline{
	string data[4];
	unsigned int tag;
	bool dirty;
};

struct memory{
	string block[4];
};

int main(int argc, char *argv[])
{
	cacheline *cache = new cacheline[64];
	memory *RAM = new memory[16384];

	for(int k = 0; k < 64; k++) 
	{
		cache[k].data[0] = "00";
		cache[k].data[1] = "00";
		cache[k].data[2] = "00";
		cache[k].data[3] = "00";
		cache[k].tag = 16385;
		cache[k].dirty = false;
	}

	for(int l = 0; l < 16384; l++)	
	{
		RAM[l].block[0] = "00";
		RAM[l].block[1] = "00";
		RAM[l].block[2] = "00";
		RAM[l].block[3] = "00";
	}


	char *filename = argv[1];
	string operation;
	string address;
	string dataTemp;
	unsigned int addressTemp;
	unsigned int offset;
	unsigned int line;
	unsigned int ramline;
	unsigned int tagTemp;
	
	ifstream infile;
	infile.open(filename);

	ofstream myfile;
	myfile.open("dm-out.txt");

	int hit;
	int dirtyout;
	
	while(infile){
		getline(infile, operation);
		istringstream iss(operation);
		stringstream ss;
		stringstream ss2;
		iss >> address >> operation >> dataTemp;						//break up parts of file input	
		
		ss << hex << address;
		ss >> addressTemp;

		offset = addressTemp & 0x3;								//bitwise and to get offset bits
		line = (addressTemp >> 2) & 0x3f;							//shift right two, then bitwise and to get bits for line number
		tagTemp = (addressTemp >> 8) & 0xff;							//shift right eight, then bitwise and to get bits for tag
	
		if(operation == "FF")									//write operation
		{
			if(tagTemp == cache[line].tag)
			{
				cache[line].data[offset] = dataTemp;
				cache[line].dirty = true;
			}		
			else if(tagTemp != cache[line].tag)
			{
				ramline = (cache[line].tag) & 0xff;					//bitwise and to set tag bits
				ramline = (ramline << 6) | line;					//shift left 6, then bitwise or to keep tag bits and set line number bits
				
				for(int i = 0; i < 4; i++)
				{
					RAM[ramline].block[i] = cache[line].data[i];			//evict cacheline to main memory
					cache[line].data[i] = RAM[(addressTemp >> 2)].block[i];		//bring data with right tag back into cache
				}
				cache[line].tag = tagTemp;	 					//update tag in cache;
				cache[line].dirty = false;						//cache represents main memory, so not dirty

				cache[line].data[offset] = dataTemp;
				cache[line].dirty = true;
			}	
		}
		else if(operation == "00")								//read operation
		{

			if(cache[line].tag == tagTemp)
			{
				hit = 1;
				if(cache[line].dirty)
				{
					dirtyout = 1;
				}
				else
				{
					dirtyout = 0;
				}

				myfile <<  cache[line].data[offset] << " " << hit << " " << dirtyout << endl;	
			}
			else if(cache[line].tag != tagTemp)
			{
				hit = 0;
				dirtyout = 0;
				
				ramline = (cache[line].tag) & 0xff; 		
				ramline = (ramline << 6) | line;

				for(int j = 0; j < 4; j++)
				{
					RAM[ramline].block[j] = cache[line].data[j];
					cache[line].data[j] = RAM[(addressTemp >> 2)].block[j];
				}

				cache[line].tag = tagTemp;
				cache[line].dirty = false;

				myfile << cache[line].data[offset] << " " << hit << " " << dirtyout << endl;
			}
		}
	}//while
	
	myfile.close();

	return 0;
}

