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

#define windows

int size = 1048576;

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
		while (infile >> word)
		{
			wordHashes.push_back(hashString(word));
		}
		cout << "Completed initializing memory hash." << endl;

		numOfWords = wordHashes.size();
		hashTable = new bool[size];
	}
	~Hash()
	{
		delete hashTable;
	}

	int getCollisions(State& state)
	{
		memset(hashTable,0,sizeof(bool)*size);

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
private:
	vector<int> wordHashes;
	int numOfWords;
	bool* hashTable;


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
	} 

	/*
	* Executes the simulated annealing algorithm.
	* Returns the best state.
	*/
	State simulate()
	{
		currentEnergy = currentHash.getCollisions(currentState);
		temperature = currentEnergy;
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
			temperature *= 0.9;

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
		int alterationCount = 10;
		State alteration;
		for (int i = 0; i <= runLimit; i++)
		{
			pickAlteration(alteration);
			int newEnergy = currentHash.getCollisions(alteration);
			int energyDiff = newEnergy - currentEnergy;
			if (oracle(energyDiff))
			{
				alterList(alteration, newEnergy); //Commit changes
				alterationCount++;
			}
			if (alterationCount >= alterationLimit)
				break;
		}
		return alterationLimit;
	}
};

void handleInput(int argc, string argv[])
{
	//upperlimit, hashSize, temperature decrease ratio, initial temperature
}

Hash* commonHash;

void* work(void* data)
{
	SimulateAnnealing sa(*commonHash, 30);
	sa.simulate();
}

//try 4 numbers between 50-99}
int main(int argc, char* argv[])
{
	int threadCount = 10;
	//Hash* h = new Hash("bigdictionary.txt");
	commonHash = new Hash("bigdictionary.txt");

#ifndef windows
	vector<pthread_t> threads;
	//SimulateAnnealing sa(*h, 30);
	//State res = sa.simulate();

	for (int i = 0; i < threadCount; i++)
	{      
		pthread_t thread;
		int result = pthread_create(&thread, NULL, work, NULL);
		if (result != 0)
			cout << "Error creating producer." << endl;

		threads.push_back(thread);
	}

	bool success = true;

	for (int i = 0; i < threads.size(); i++)
	{
		pthread_t thread = threads[i];
		int result = pthread_join(thread, NULL);
		if (result != 0) //Failure
			success = false;
	}
#endif

	delete commonHash;
	return 0;
}
