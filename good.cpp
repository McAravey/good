#include <stdlib.h>
#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>
#include <sstream>
#include <cstring>
using namespace std;

//#define windows

int size = 32768;//1048576;

struct State
{
	int x;
	int y;
	int z;
	int w;

	string toString()
	{
		ostringstream ss;
		ss << "X: " << x
			<< " Y: " << y
			<< " Z: " << z
			<< " W: " << w;
		return ss.str();
	}
};

class Hash
{
public:
	Hash(string dictionary)
	{ 
		std::ifstream infile(dictionary.c_str());
		string word;

		cout << "Initializing memory hash..." << endl;
		while (getline(infile, word))
		{
			wordHashes.push_back(hashString(word));
		}
		cout << "Completed initializing memory hash." << endl;

		numOfWords = wordHashes.size();
		//hashTable = new bool[size];
	}

	Hash(int inputSize)
	{ 
		cout << "Initializing memory hash..." << endl;
		for (int i = 0; i < inputSize; i++)
		{
			wordHashes.push_back(rand());
		}
		cout << "Completed initializing memory hash." << endl;

		numOfWords = inputSize;
		//hashTable = new bool[size];
	}
	~Hash()
	{
		//delete hashTable;
	}

	int getCollisions(State& state, bool* hashTable)
	{
		cout << "Calling memset" << endl;
		memset(hashTable,0,sizeof(bool)*size);
		cout << "Called memset" << endl;

		int totalCollisions = 0; //Track the total number of collisions for finding the best
		int initialHash = 0;

		//Treat the vector as an array for fast indexing.
		//Discovered this performance hit using Visual Studio
		//   performance analysis tools.
		int* p = &wordHashes[0];


		for(int i = 0; i < numOfWords; ++i)
		{
			initialHash = *p++; //Get the next number to hash.
			unsigned protectedHash = protectionHash(initialHash, state); //Apply the extra protective hash on top of the initial hash.
			int index = indexFor(protectedHash, size); //Get the index for the hash that matches the size of the current hash table
			if (hashTable[index] == true) //If there is a collision
				totalCollisions++;
			else
				hashTable[index] = true;
		}
		return totalCollisions;
	}

	int getSize()
	{
		return numOfWords;
	}
private:
	vector<int> wordHashes;
	int numOfWords;
	//bool* hashTable;


	int hashString(string s)
	{
		int hash = 0;
		int n = s.size() - 1;
		for (int i = 0; i <= n; i++)
		{
			//hash += s[i] * pow(31, n - i);
			hash = s[i] + 31 * hash;
		}
		return hash;
	}

	/*
	* Gets the index into the hash table. Note: This function
	* has been inlined for some crazy performance improvements.
	*/
	inline int indexFor(unsigned h, int length)
	{
		return h & (length - 1);
	}

	/*
	* Apply a protective hash to the original hash using the values x y z and w.
	* Note: This function has been inlined for some crazy performance improvements.
	*/
	inline unsigned protectionHash(unsigned hash, State& state)
	{
		hash += ~(hash << state.x);//9
		hash ^= (hash >> state.y);//14
		hash += (hash << state.z);//4
		hash ^= (hash >> state.w);//10
		return hash;
	}
};

class SimulateAnnealing
{
public:
	SimulateAnnealing(Hash& hashValue, int limit = 30) : currentHash(hashValue)
	{
		upperLimit = limit;
		cout << "Allocating" << endl;
		hashTable = new bool[size];
		cout << "Allocated" << endl;
	} 
	~SimulateAnnealing()
	{
		delete hashTable;
	}

	/*
	* Executes the simulated annealing algorithm.
	* Returns the best state.
	*/
	State simulate()
	{
		currentEnergy = currentHash.getCollisions(currentState, hashTable);
		temperature = 1000000;//currentEnergy;
		int runLimit = 400;
		int alterationLimit = 40;

		while (temperature > 1)
		{
			int successfulAlterations = anneal(runLimit, alterationLimit);
			if (successfulAlterations == 0)
			{
				cout << "No alterations, exiting loop." << endl;
				break;
			}
			temperature *= 0.95;

			cout << "Temperature: " << temperature << endl;
		}
		cout << endl 
			<< "======================================" << endl
			<< " And the winning results are" << endl
			<< "======================================" << endl;
		printResults();
		return currentState;
	}

	void simulateSilent()
	{
		simulate();
	}
private:
	Hash& currentHash;
	State currentState;
	int currentEnergy;
	double temperature;
	int upperLimit;
	bool* hashTable;

	/*
	* Pick a new state to work with.
	*/
	void pickAlteration(State& alterations)
	{
		alterations.x = rand() % upperLimit;
		alterations.y = rand() % upperLimit;
		alterations.z = rand() % upperLimit;
		alterations.w = rand() % upperLimit;
	}

	void printResults()
	{
		cout << currentState.toString()
			<< " With an energy of: " << currentEnergy << endl;
	}

	/*
	* Accept the new values as the current values.
	* Commits changes.
	*/
	void alterList(State& alteredState, int newEnergy)
	{
		currentEnergy = newEnergy;
		currentState = alteredState;
	}

	/*
	* Tell if the new energy difference is acceptable or not.
	* The Boltzman factor allows for a certain tolerance of
	* regression.
	*/
	bool oracle(int energyDiff)
	{
		if (energyDiff < 0)
			return true;

		if (exp((-energyDiff)/temperature) > (rand() % 2))
			return true;

		return false;
	}

	/*
	* Returns the number of successful alterations made.
	*/
	int anneal(int runLimit, int alterationLimit)
	{
		int alterationCount = 0;
		State alteration;
		for (int i = 0; i <= runLimit; i++)
		{
			pickAlteration(alteration);
			int newEnergy = currentHash.getCollisions(alteration, hashTable);
			int energyDiff = newEnergy - currentEnergy;
			if (oracle(energyDiff))
			{
				alterList(alteration, newEnergy); //Commit changes
				alterationCount++;
			}
			if (alterationCount >= alterationLimit)
				break;
		}
		return alterationCount;
	}
};

void handleInput(int argc, string argv[])
{
	//upperlimit, hashSize, temperature decrease ratio, initial temperature
}

//Hash* commonHash;

void* work(void* data)
{
	Hash* currentHash = (Hash*)data;
//	cout << "Hash size: " << currentHash->getSize() << endl;
	SimulateAnnealing sa(*currentHash, 30);
	cout << "Starting simulation" << endl;
	sa.simulate();
}


vector<pthread_t> runSimulation(Hash* currentHash)
{
	int threadCount = 1;
	vector<pthread_t> threads;

	for (int i = 0; i < threadCount; i++)
	{      
		void* hashData = (void*)currentHash;
		pthread_t thread;
		int result = pthread_create(&thread, NULL, work, hashData);
		if (result != 0)
			cout << "Error creating producer." << endl;

		threads.push_back(thread);
	}
//
//#ifdef windows
//	SimulateAnnealing sa(*currentHash, 30);
//	State res = sa.simulate();
//#endif
}

void waitAll(vector<pthread_t> threads)
{
	bool success = true;
	
	for (int i = 0; i < threads.size(); i++)
	{
		pthread_t thread = threads[i];
		int result = pthread_join(thread, NULL);
		if (result != 0) //Failure
			success = false;
	}
}

//try 4 numbers between 50-99}
int main(int argc, char* argv[])
{

	//Hash* hash1 = new Hash("bigdictionary.txt");
	Hash* hash1 = new Hash(32768);
//	Hash* hash2 = new Hash(32768);
//	Hash* hash3 = new Hash(32768);
	
	vector<pthread_t> res1 = runSimulation(hash1);
//	vector<pthread_t> res2 = runSimulation(hash2);
//	vector<pthread_t> res3 = runSimulation(hash3);
	
//	cout << "Start inserting" << endl;
//	vector<pthread_t> res;
//	res.insert(res.end(), res1.begin(), res1.end());
//	res.insert(res.end(), res2.begin(), res2.end());
//	res.insert(res.end(), res3.begin(), res3.end());
//	res2.insert(res2.end(), res3.begin(), res3.end());
//	res1.insert(res1.end(), res2.begin(), res2.end());
//	res.insert(res.end(), res3.begin(), res3.end());
	
//	cout << "Done inserting" << endl;
	


	//bool* hashTable = new bool[size];
	//State baseState;
	//baseState.x = 9;
	//baseState.y = 14;
	//baseState.z = 4;
	//baseState.w = 10;
	//cout << "BASE (x=9, y=14, z=4, w=10): " << commonHash->getCollisions(baseState, hashTable) << endl << endl;

   waitAll(res1);
//   waitAll(res2);
//   waitAll(res3);

	delete hash1;
//	delete hash2;
//	delete hash3;
	return 0;
}
