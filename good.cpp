/*******************************
 * Exploration 5 - Good
 *
 * Samuel Mcaravey - mca12004
 * 
 *
 *******************************
 *            LOG 
 *******************************
 * 
 * 
 * 
 * 
 * 
 * 6-27-2013
 *    -Finished creating the C++ test harness.
 *    -Ran the exact numbers through an equivilent 
 *        Java program for verification.
 *    -Discovered that energy is the number of collisions!
 *        "It is also necessary
 *			for you to be able to generate a number
 *			that tells how well any given sequence
 *			works; the better the solution, the smaller
 *			this number must be. This number is
 *			analogous to the energy of the crystal in
 *			the crystal-growth example."
 *    -Tried to turn the energy function into a memorizing function
 *        to see if this will help avoid the plateu in small data sets.
 * 
 * 
 * 
 */

#include <cmath>
#include <iostream>
#include <fstream>
#include <iomanip>
#include <string>
#include <fstream>
#include <vector>
#include <map>
#include <sstream>
using namespace std;

//class Sequence
//{
//public:
//	int x;
//	int y;
//	int z;
//	int w;
//};

int size = 1048576;
string* words;
int numOfWords = 0;
map<string, int> energyValues;

/*===========================Begin Test Harness===========================*/
string getStringValue(int x, int y, int z, int w)
{
	std::ostringstream oss;
	oss << x << y << z << w;
	/*string s = "";
	s += itoa(x);
	s += itoa(y);
	s += itoa(z);
	s += itoa(w);*/
	return oss.str();
}


int hashString(string s)
{
	int hash = 0;
	int n = s.size() - 1;
	for (int i = 0; i <= n; i++)
	{
		//h = 31 * h + s[i]
		//hash += s[i] * pow(31, n - i);
		hash = s[i] + 31 * hash;
	}
	return hash;
}

int indexFor(int h, int length)
{
	return h & (length - 1);
}

/*
* Apply a protective hash to the original hash using the values x y z and w.
*/
int protectionHash(int originalHash, int x, int y, int z, int w)
{
	int hash = originalHash;

	hash  += ~(hash << x);//9
	hash ^= ((unsigned)hash >> y);//14
	hash += (hash << z);//4
	hash ^= ((unsigned)hash >> w);//10

	return hash;
}

int getCollisions(int x, int y, int z, int w)
{
	/*string s = getStringValue(x, y, z, w);

	map<string, int>::iterator it = energyValues.find(s);
	if (it != energyValues.end())
	{
		return energyValues[s];
	}*/

	bool* collisionTrackers = new bool[size]; //Simply track which slot in the "hash table" have an object in it. We don't care about storing the values.
	int totalCollisions = 0; //Track the total number of collisions for finding the best
	//int wordCount = 0;

	//std::ifstream infile("C:\\Users\\Samuel McAravey\\SkyDrive\\Documents\\BYUI\\Spring 2013\\CS 306 - Algorithms\\Explorations\\good\\good\\5letterwords.txt");
	string word;

	for (int i = 0; i < numOfWords; i++)
	{
		word = words[i];

		int initialHash = hashString(word); //Get the initial hash from a string to an int
		int protectedHash = protectionHash(initialHash, x, y, z, w); //Apply the extra protective hash on top of the initial hash.
		int index = indexFor(protectedHash, size); //Get the index for the hash that matches the size of the current hash table

		if (collisionTrackers[index] == true) //If there is a collision
			totalCollisions++;
		else
			collisionTrackers[index] = true;
	}

	//energyValues[s] = totalCollisions;

	return totalCollisions;
}

int getCollisions(int* hashValues)
{
	return getCollisions(
		hashValues[0], 
		hashValues[1], 
		hashValues[2], 
		hashValues[3]);
}
/*===========================End Test Harness===========================*/

int* hashValues;
int currentEnergy;
double temperature;
int upperLimit = 30;


int* pickAlteration()
{
	int* alterations = new int[4];
	alterations[0] = hashValues[0];
	alterations[1] = hashValues[1];
	alterations[2] = hashValues[2];
	alterations[3] = hashValues[3];

	int changeIndex = rand() % 4;
	int add = rand() % 2;
	int amount = (rand() % 2) + 1;

	if ((add == 1 && alterations[changeIndex] < upperLimit) || alterations[changeIndex] <= 0)
		alterations[changeIndex] += amount;
	else
		alterations[changeIndex] -= amount;

	return alterations;
}

void printResults()
{
	cout << "X: " << hashValues[0]
		 << "  Y: " << hashValues[1]
		 << "  Z: " << hashValues[2]
		 << "  W: " << hashValues[3]
		 << "  With an energy of: " << currentEnergy << endl;
}

void alterList(int* alteration, int newEnergy)
{
	currentEnergy = newEnergy;
	hashValues = alteration;
	printResults();
}

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

	for (int i = 0; i <= runLimit; i++)
	{
		int* alteration = pickAlteration();
		int newEnergy = getCollisions(alteration);
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

void simulateAnnealing()//int* initialSequence)
{
	//int* hashValues = initialSequence;
	int energy = getCollisions(hashValues);
	temperature = energy / 4; //4 = the number of items in the list of hashValues
	int runLimit = 400;
	int alterationLimit = 40;

	for (int i = 0; i < 100; i++)
	{
		int successfulAlterations = anneal(runLimit, alterationLimit);
		if (successfulAlterations == 0)
			break;

		temperature *= 0.9;
	}
	
	cout << "======================================" << endl
		 << "     And the winning results are" << endl
	     << "======================================" << endl;
}


//try 4 numbers between 50-99
int main(int argc, char* argv[])
{
	vector<string> wordList;
	std::ifstream infile("C:\\Users\\Samuel McAravey\\SkyDrive\\Documents\\BYUI\\Spring 2013\\CS 306 - Algorithms\\Explorations\\good\\good\\dictionary.txt");
	string word;
	while (infile >> word)
	{
		wordList.push_back(word);
	}

	words = &wordList[0];
	numOfWords = wordList.size();

	hashValues = new int[4];
	hashValues[0] = 30;
	hashValues[1] = 30;
	hashValues[2] = 30;
	hashValues[3] = 30;

	simulateAnnealing();//initialSequence);
	//int collisions = getCollisions(9, 14, 4, 10);
	//cout << "Collisions: " << collisions << endl; 
}