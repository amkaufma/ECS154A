//Andrew Kaufman & Frank Chung

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <iomanip>

using namespace std;

struct cacheline {
	string data[4];
	unsigned int tag;
	bool dirty;
	unsigned int lru;
};

struct memory {
	string block[4];
};

int main(int argc, char *argv[])
{
	cacheline *cache = new cacheline[40];
	memory *RAM = new memory[16384];

	//unsigned int setnum;

	for(int i = 0; i < 40; i++)
	{
		cache[i].data[0] = "00";
		cache[i].data[1] = "00";
		cache[i].data[2] = "00";
		cache[i].data[3] = "00";
		cache[i].tag = 16390;
		cache[i].dirty = false;
                cache[i].lru = 0;
	}

	for(int j = 0; j < 16384; j++)
	{
		RAM[j].block[0] = "00";
		RAM[j].block[1] = "00";
		RAM[j].block[2] = "00";
		RAM[j].block[3] = "00";
	}

	char *filename = argv[1];

	string operation;
	string address;
	string dataTemp;

	unsigned int addressTemp;
	unsigned int offset;
	unsigned int set;
        unsigned int index;
        unsigned int replace;
	unsigned int ramline;
	unsigned int tagTemp;
        
        bool miss;
        unsigned int largest;
        unsigned int count;

	ifstream infile;
	infile.open(filename);

	ofstream myfile;
	myfile.open("sa-out.txt");

	int hit;
	int dirtyout;
	
	while(infile)
	{
		getline(infile, operation);
		istringstream iss(operation);
                stringstream ss;
                iss >> address >> operation >> dataTemp;
                //cout << operation << endl;
                
                ss << hex << address;
                ss >> addressTemp;
                
                offset = addressTemp & 0x3;
                set = (addressTemp >> 2) & 0x7;
                tagTemp = (addressTemp >> 5) & 0x7ff;
                
                index = set * 5;
                miss = true;
                
                if(operation == "FF")
                {
                    for(int k = 0; k < 5; k++)
                    {
                        if(cache[index + k].tag == tagTemp)
                        {
                            miss = false;
                            index = index + k;
                        }
                    }
                    
                    if(miss)            //write miss
                    {
                        largest = 0;
                        for(int l = 0; l < 5; l++)
                        {
                            if(cache[index + l].lru >= largest)
                            {
                                largest = cache[index + l].lru;
                                replace = (index + l);
                            }
                            
                        }
                        
                        ramline = (cache[replace].tag) & 0x7ff;
                        ramline = (ramline << 3) | set;
                        
                        for(int m = 0; m < 4; m++)
                        {
                            RAM[ramline].block[m] = cache[replace].data[m];
                            cache[replace].data[m] = RAM[(addressTemp) >> 2].block[m];
                        }
                        
                        cache[replace].tag = tagTemp;
                        cache[replace].dirty = false;
                        
                        for(int n = 0; n < 5; n++)
                        {
                            if(cache[index + n].lru <= largest)
                            {
                                cache[index + n].lru = (cache[index + n].lru) + 1;
                            }
                        }
                        
                        cache[replace].lru = 0;
                        cache[replace].data[offset] = dataTemp;
                        cache[replace].dirty = true;

                    }//end write miss
                    else if(!miss)
                    {
                        cache[index].data[offset] = dataTemp;
                        cache[index].dirty = true;
                        
                        count = cache[index].lru;
                        for(int p = 0; p < 5; p++)
                        {
                            if((cache[(set * 5) + p].lru) <= count) // cache[index].lru)
                            {
                                cache[(set * 5) + p].lru = (cache[(set * 5) + p].lru) + 1;    //changed the brackets from containing [index]
                            }
                        }
                        
                        cache[index].lru = 0;
                    }//end write hit
                    

                    
                }
                else if(operation == "00")
                {
                    for(int q = 0; q < 5; q++)
                    {
                        if(cache[index + q].tag == tagTemp)
                        {
                            miss = false;
                            index = index + q;
                        }
                    }
                    
                    if(miss)            //read miss
                    {
                        hit = 0;
                        dirtyout = 0;
                        
                        largest = 0;
                        for(int r = 0; r < 5; r++)
                        {
                            if(cache[index + r].lru >= largest)
                            {
                                largest = cache[index + r].lru;
                                replace = (index + r);
                            }
                        }
                        
                        ramline = (cache[replace].tag) & 0x7ff;
                        ramline = (ramline << 3) | set;
                        
                        for(int s = 0; s < 4; s++)
                        {
                            RAM[ramline].block[s] = cache[replace].data[s];
                            cache[replace].data[s] = RAM[(addressTemp >> 2)].block[s];
                        }
                        
                        cache[replace].tag = tagTemp;
                        cache[replace].dirty = false;
                        
                        for(int t = 0; t < 5; t++)
                        {
                            if((cache[index + t].lru) <= largest)
                            {
                                cache[index + t].lru = (cache[index + t].lru) + 1;
                            }
                        }
                        
                        cache[replace].lru = 0;
                        
                        myfile << cache[replace].data[offset] << " " << hit << " " << dirtyout << endl;
                        
                    }//end read miss
                    else if(!miss)          //read hit
                    {
                        hit = 1;
                        if(cache[index].dirty)
                        {
                            dirtyout = 1;
                        }
                        else
                        {
                            dirtyout = 0;
                        }
                        
                        count = cache[index].lru;
                        for(int u = 0; u < 5; u++)
                        {
                            if(cache[(set * 5) + u].lru <= count) //cache[index].lru)
                            {
                                cache[(set * 5)+ u].lru = (cache[(set * 5) + u].lru) + 1;
                            }
                        }
                        
                        cache[index].lru = 0;
                        
                        myfile << cache[index].data[offset] << " " << hit << " " << dirtyout << endl;
                        
                    }
                    
                }
	}//end while
	
        myfile.close();
	return 0;
}
