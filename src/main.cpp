#include "local.hpp"

using namespace std;

bool hitWall(double, double);

// Global Variables
int NUMBER_OF_ANTS;
int FOOD_DWELL_TIME;
int ANT_SMELL_DISTANCE;
int STRONG_PHEROMONE_THRESHOLD;
int WEAK_PHEROMONE_THRESHOLD;
int PHEROMONE_THRESHOLD;
int ANT_APPETITE;
int RUN_TIME;

void *
antLifeCycle(void *data)
{
    int speed = (rand() % 10) + 1;

    double x = (rand() % 5) - 2; // rand from -2 -> 2
    double y = (rand() % 9) - 4; // rand from -4 -> 4
    //
    int direction = (rand() % 8) * 45;
    while (1)
    {
        // TODO:  y+=speed*sin(direction) ?

        // TODO: check x , y
        y += sin(direction);
        x += cos(direction);

        cout << "OUTTER x " << x << " y " << y << endl;

        usleep(10000 / speed);
        int d = ((rand() % 2) * 2 - 1);

        if (hitWall(x, y))
        {
            direction += 45 * d; // get random value between -1 for CW and 1 CCW
            while (hitWall(x + cos(direction), y + sin(direction)))
            {
                direction += 45 * d;
            }
            y += sin(direction);
            x += cos(direction);
            cout << "INNER x " << x << " y " << y << endl;
        }
    }
}

int main(int argc, char *argv[])
{
    read_constants("./conf.txt");
    printf("NUMBER_OF_ANTS: %d\n", NUMBER_OF_ANTS);
    printf("FOOD_DWELL_TIME: %d\n", FOOD_DWELL_TIME);
    printf("ANT_SMELL_DISTANCE: %d\n", ANT_SMELL_DISTANCE);
    printf("STRONG_PHEROMONE_THRESHOLD: %d\n", STRONG_PHEROMONE_THRESHOLD);
    printf("WEAK_PHEROMONE_THRESHOLD: %d\n", WEAK_PHEROMONE_THRESHOLD);
    printf("PHEROMONE_THRESHOLD: %d\n", PHEROMONE_THRESHOLD);
    printf("ANT_APPETITE: %d\n", ANT_APPETITE);
    printf("RUN_TIME: %d\n", RUN_TIME);

    srand(getpid());
    int antsId[NUMBER_OF_ANTS];
    pthread_t antsThread[NUMBER_OF_ANTS];

    for (int i = 0; i < NUMBER_OF_ANTS; i++)
    {
        antsId[i] = pthread_create(&antsThread[i], NULL, antLifeCycle, (void *)&i);
    }
    for (int i = 0; i < NUMBER_OF_ANTS; i++)
        pthread_join(antsThread[i], NULL);

    return 0;
}

bool hitWall(double x, double y)
{
    if (x <= -2 || x >= 2 || y <= -4 || y >= 4)
    {
        return true;
    }
    return false;
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
