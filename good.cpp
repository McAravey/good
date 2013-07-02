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
#include <ctime>
using namespace std;

#define windows

/*******************************************************************************
* Global Constants
*******************************************************************************/
int const SIZE = 32768;//1048576;
string const DICTIONARY = "bigdictionary.txt";

/*******************************************************************************
* State Structure
* Represents a state of numbers in our search.
*******************************************************************************/
struct State
{
   int x;
   int y;
   int z;
   int w;

   /****************************************************************************
   * toString
   * Provides easy way to display the output of the string.
   ****************************************************************************/
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

/*******************************************************************************
* Hash Class
* Represents an instance of hash of a dictionary.
*******************************************************************************/
class Hash
{
public:
   /****************************************************************************
   * Constructor
   * Input is the filename of a dictionary to use as input.
   ****************************************************************************/
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

   /****************************************************************************
   * Constructor
   * Input is the size of random integers to be placed in the hash.
   ****************************************************************************/
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

   /****************************************************************************
   * Destructor
   ****************************************************************************/
   ~Hash()
   {
      //delete hashTable;
   }

   /****************************************************************************
   * getCollisions
   * Returns the number of collisons for a given state and boolean hash table.
   ****************************************************************************/
   int getCollisions(State& state, bool* hashTable)
   {
      memset(hashTable,0,sizeof(bool)*SIZE);

      //Track the total number of collisions for finding the best
      int totalCollisions = 0;
      int initialHash = 0;

      //Treat the vector as an array for fast indexing.
      //Discovered this performance hit using Visual Studio
      //   performance analysis tools.
      int* p = &wordHashes[0];


      for(int i = 0; i < numOfWords; ++i)
      {
         initialHash = *p++; //Get the next number to hash.
         //Apply the extra protective hash on top of the initial hash.
         unsigned protectedHash = protectionHash(initialHash, state);
         //Get the index for the hash that matches the size of the current hash table
         int index = indexFor(protectedHash, SIZE);
         if (hashTable[index] == true)
            //If there is a collision
            totalCollisions++;
         else
            hashTable[index] = true;
      }
      return totalCollisions;
   }

   /****************************************************************************
   * getStdDeviation
   * Given a state and a integer hash map, it returns its standar deviation.
   ****************************************************************************/
   double getStdDeviation(State& state, int* collisionMap)
   {
      memset(collisionMap,0,sizeof(int)*SIZE);

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
         int index = indexFor(protectedHash, SIZE); //Get the index for the hash that matches the size of the current hash table
         if (collisionMap[index] > 1) //If there is a collision
         {
            totalCollisions++;
            collisionMap[index]++;
         }
         else
         {
            collisionMap[index]++;
         }
      }

      //Calculate the standard deviation.
      double sum = 0;
      for (int i = 0; i < SIZE; i++)
      {
         sum += collisionMap[i];
      }
      double mean = sum / SIZE;
      double sq_diff_sum = 0;
      for (int i = 0; i < SIZE; i++)
      {
         double diff = collisionMap[i] - mean;
         sq_diff_sum += diff * diff;
      }
      double variance = sq_diff_sum / SIZE;
      return sqrt(variance);
   }

   /****************************************************************************
   * getSize
   * Accessor function to return the number of words in the hash.
   ****************************************************************************/
   int getSize()
   {
      return numOfWords;
   }

private:
   vector<int> wordHashes;  //Holds the hashes of the words.
   int numOfWords;          //Holds the total number of words.
   //bool* hashTable;

   /****************************************************************************
   * hashString
   * Takes the given string and hashes it into an integer.
   ****************************************************************************/
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

   /****************************************************************************
   * Gets the index into the hash table. Note: This function
   * has been inlined for some crazy performance improvements.
   ****************************************************************************/
   inline int indexFor(unsigned h, int length)
   {
      return h & (length - 1);
   }

   /****************************************************************************
   * Apply a protective hash to the original hash using the values x y z and w.
   * Note: This function has been inlined for some crazy performance improvements.
   ****************************************************************************/
   inline unsigned protectionHash(unsigned hash, State& state)
   {
      hash += ~(hash << state.x); //9
      hash ^= (hash >> state.y);  //14
      hash += (hash << state.z);  //4
      hash ^= (hash >> state.w);  //10
      return hash;
   }
};

/*******************************************************************************
* SimulateAnnealing Class
* Represents an object which allows the process of simulated annealing to
* take place.
*******************************************************************************/
class SimulateAnnealing
{
public:
   /****************************************************************************
   * Constructor
   * Sets everyting up with an upper bound and provides a hash table as well.
   * pAlgorithm determines the algorithm to use for calcuating energy.
   *    1 - Collisions
   *    2 - Standard Deviation
   ****************************************************************************/
   SimulateAnnealing(Hash& hashValue, int limit = 30,
                     int pAlgorithm = 1) : currentHash(hashValue)
   {
      upperLimit = limit;
      hashTable = new bool[SIZE];
      mAlgorithm = pAlgorithm;
      collisionMap = new int[SIZE];
   }
   /****************************************************************************
   * Destructor
   ****************************************************************************/
   ~SimulateAnnealing()
   {
      delete hashTable;
   }

   /****************************************************************************
   * Executes the simulated annealing algorithm.
   * Returns the best state.
   ****************************************************************************/
   State simulate()
   {
      if (mAlgorithm == 1)
      {
         currentEnergy = currentHash.getCollisions(currentState, hashTable);
      }
      else if (mAlgorithm == 2)
      {
         currentEnergyDev = currentHash.getStdDeviation(currentState, collisionMap);
      }
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

private:
   Hash& currentHash;
   State currentState;
   int currentEnergy;
   double currentEnergyDev;
   double temperature;
   int upperLimit;
   bool* hashTable;
   int* collisionMap;
   int mAlgorithm; //Switch to determine algorithm to use.

   /****************************************************************************
   * Pick a new state to work with.
   ****************************************************************************/
   void pickAlteration(State& alterations)
   {
      alterations.x = rand() % upperLimit;
      alterations.y = rand() % upperLimit;
      alterations.z = rand() % upperLimit;
      alterations.w = rand() % upperLimit;
   }

   /****************************************************************************
   * Print results out.
   ****************************************************************************/
   void printResults()
   {
      cout << currentState.toString()
         << " With an energy of: ";
      if (mAlgorithm == 1)
      {
    		cout << currentEnergy << endl;
      }
      else if (mAlgorithm == 2)
      {
      	cout << currentEnergyDev << endl;
      }
   }

   /****************************************************************************
   * Accept the new values as the current values.
   * Commits changes.
   ****************************************************************************/
   void alterList(State& alteredState, int newEnergy)
   {
      currentEnergy = newEnergy;
      currentState = alteredState;
   }

   /****************************************************************************
   * Accept the new values as the current values.
   * Commits changes. Uses doubles for this one.
   ****************************************************************************/
   void alterList(State& alteredState, double newEnergyDeviation)
   {
   	currentEnergyDev = newEnergyDeviation;
   	currentState = alteredState;
   }

   /****************************************************************************
   * Tell if the new energy difference is acceptable or not.
   * The Boltzman factor allows for a certain tolerance of
   * regression.
   ****************************************************************************/
   bool oracle(int energyDiff)
   {
      if (energyDiff < 0)
         return true;

      if (exp((-energyDiff)/temperature) > (rand() % 2))
         return true;

      return false;
   }

   /*******************************************************************************
   * oracle
   * Same as previous oracle (with the exception of doubles instead)
   *******************************************************************************/
   bool oracle(double energyDiff)
   {
      if (energyDiff < 0)
         return true;

      if (exp((-energyDiff)/temperature) > (rand() % 2))
         return true;

      return false;
   }

   /****************************************************************************
   * Returns the number of successful alterations made.
   ****************************************************************************/
   int anneal(int runLimit, int alterationLimit)
   {
      int alterationCount = 10;
      State alteration;
      int newEnergy = 0;
      double newEnergyDeviation = 0.0;
      for (int i = 0; i <= runLimit; i++)
      {
         pickAlteration(alteration);
         if (mAlgorithm == 1)
         {
            newEnergy = currentHash.getCollisions(alteration, hashTable);
            int energyDiff = newEnergy - currentEnergy;
            if (oracle(energyDiff))
            {
               alterList(alteration, newEnergy); //Commit changes
               alterationCount++;
            }
         }
         else if (mAlgorithm == 2)
         {
            newEnergyDeviation = currentHash.getStdDeviation(alteration, collisionMap);
            double energyDiff = newEnergyDeviation - currentEnergyDev;
            if (oracle(energyDiff))
            {
               alterList(alteration, newEnergyDeviation);
               alterationCount++;
            }
         }

         if (alterationCount >= alterationLimit)
            break;
      }
      return alterationLimit;
   }
};


Hash* commonHash; //The hash all threads will use.

/*******************************************************************************
* work
* Used with threads to handle the Simulated Annealing for each one.
*******************************************************************************/
void* work(void* data)
{
   int* algorithm = (int*) data;
   cout << algorithm;
   SimulateAnnealing sa(*commonHash, 30, *algorithm);
   sa.simulate();
   delete algorithm;
}


/*******************************************************************************
* Prompts for input from the user.
* Returns a number 1, 2, or 3 indicating what the user wishes to do.
*******************************************************************************/
int menuPrompt()
{
	srand(time(NULL));
   cout << "\nWelcome to the Goodness tester." << endl;
   cout << "Select from the choices below to proceed:" << endl;
   cout << "\t1. Use # of collisions as the energy. (faster)" << endl;
   cout << "\t2. Use standard deviation as the energy." << endl;
   cout << "\t3. Exit";

   bool validInput = false;
   int returnValue = 0;
   while (!validInput)
   {
      cout << "\nSelection: ";
      cin >> returnValue;
      if ((cin.fail()) || (returnValue > 3) || (returnValue < 1))
      {
         validInput = false;
         cin.clear();
         cin.ignore(80, '\n');
         cout << "Invalid selection. Try again.";
      }
      else
      {
         validInput = true;
      }
   }
   cin.ignore(80, '\n');
   return returnValue;
}

//try 4 numbers between 50-99}
int main(int argc, char* argv[])
{
   //The number of threads that should be spun off to do the checking.
   int threadCount = 10;

   //Creates a hash object based on the given file.
   commonHash = new Hash(DICTIONARY);

   //Create our base state for comparision. Using Java's ideal hash numbers.
   State baseState;
   baseState.x = 9;
   baseState.y = 14;
   baseState.z = 4;
   baseState.w = 10;

   //Create our hashtable for use.
   //commonHash = new Hash(32768);
   bool* hashTable = new bool[SIZE];
   int* collisionMap = new int[SIZE];

   int algorithm = menuPrompt();

   //Decide what should be done next based on the users choices.
   switch (algorithm)
   {
      case 1:
         //Perform collison energy check.
         cout << "BASE (x=9, y=14, z=4, w=10): "
              << commonHash->getCollisions(baseState, hashTable)
              << endl << endl;
         break;
      case 2:
         //Perform standard deviation energy check.
         cout << "BASE (x=9, y=14, z=4, w=10): "
              << commonHash->getStdDeviation(baseState, collisionMap)
              << endl << endl;
         break;
      case 3:
         //Exit;
         return 0;
   }

#ifndef windows
   vector<pthread_t> threads;

   for (int i = 0; i < threadCount; i++)
   {
      pthread_t thread;
      int result = pthread_create(&thread, NULL, work, (void*) algorithm);
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

#ifdef windows
   SimulateAnnealing sa(*commonHash, 30);
   State res = sa.simulate();
#endif

   delete hashTable;
   delete commonHash;
   return 0;
}
