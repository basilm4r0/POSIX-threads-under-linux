#include "local.hpp"

using namespace std;

//Global Variables
int NUMBER_OF_ANTS;
int FOOD_DWELL_TIME;
int ANT_SMELL_DISTANCE;
int STRONG_PHEROMONE_THRESHOLD;
int WEAK_PHEROMONE_THRESHOLD;
int PHEROMONE_THRESHOLD;
int ANT_APPETITE;
int RUN_TIME;

int main(int argc, char *argv[]) {
	read_constants("./conf.txt");
	printf("NUMBER_OF_ANTS: %d\n", NUMBER_OF_ANTS);
	printf("FOOD_DWELL_TIME: %d\n", FOOD_DWELL_TIME);
	printf("ANT_SMELL_DISTANCE: %d\n", ANT_SMELL_DISTANCE);
	printf("STRONG_PHEROMONE_THRESHOLD: %d\n", STRONG_PHEROMONE_THRESHOLD);
	printf("WEAK_PHEROMONE_THRESHOLD: %d\n", WEAK_PHEROMONE_THRESHOLD);
	printf("PHEROMONE_THRESHOLD: %d\n", PHEROMONE_THRESHOLD);
	printf("ANT_APPETITE: %d\n", ANT_APPETITE);
	printf("RUN_TIME: %d\n", RUN_TIME);
	return 0;
}

void read_constants(string filename)
{
    ifstream inputVariableFile(filename);
    if (!inputVariableFile.good())
    {
        perror("Error opening config file");
        exit(2);
    }

    string line;
    while (getline(inputVariableFile, line)) /* Read values and assign to suitable variable */
    {
        stringstream sline(line);
        string variableName;
        getline(sline, variableName, ' ');
        string value;
        getline(sline, value, ' ');

        /* remove negative value */
        if (value[0] == '-')
        {
            value = value.substr(1, value.size());
        }
        /* skip if not all digits */
        bool skip = false;
        for (unsigned i = 0; i < value.size(); i++)
        {
            if (!isdigit(value[i]))
            {
                skip = true;
                break;
            }
        }
        if (skip)
        {
            continue;
        }
        if (variableName == "NUMBER_OF_ANTS")
        {
			NUMBER_OF_ANTS = min(stoi(value), 40);
        }
        else if (variableName == "FOOD_DWELL_TIME ")
        {
            FOOD_DWELL_TIME = min(stoi(value), 40);
        }
        else if (variableName == "ANT_SMELL_DISTANCE")
        {
            ANT_SMELL_DISTANCE = min(stoi(value), 40);
        }
        else if (variableName == "STRONG_PHEROMONE_THRESHOLD")
        {
            STRONG_PHEROMONE_THRESHOLD = min(stoi(value), 40);
        }
        else if (variableName == "WEAK_PHEROMONE_THRESHOLD")
        {
	        WEAK_PHEROMONE_THRESHOLD = min(stoi(value), 40);
        }
        else if (variableName == "PHEROMONE_THRESHOLD")
        {
            PHEROMONE_THRESHOLD = min(stoi(value), 40);
        }
        else if (variableName == "ANT_APPETITE")
        {
            ANT_APPETITE = min(stoi(value), 40);
        }
        else if (variableName == "RUN_TIME")
        {
	        RUN_TIME = min(stoi(value), 40);
        }
    }
    inputVariableFile.close();
}
