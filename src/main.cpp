#include "local.hpp"

using namespace std;

// Global Variables
int NUMBER_OF_ANTS;
int FOOD_DWELL_TIME;
int ANT_SMELL_DISTANCE;
int STRONG_PHEROMONE_THRESHOLD;
int WEAK_PHEROMONE_THRESHOLD;
int PHEROMONE_THRESHOLD;
int ANT_APPETITE;
int RUN_TIME;
int PIECES_OF_FOOD;

vector<FOOD> foodPieces;
vector<ANT> ants;

void *antLifeCycle(void *data)
{
    int speed = 9; // for production change to: (rand() % 10) + 1;
    cout << "speed: " << speed << endl;

    double x = randomDouble(-4, 4); // rand from -4 -> 4
    double y = randomDouble(-2, 2); // rand from -2 -> 2
    //
    double direction = ((rand() % 8) * 45) * M_PI / 180; // Angle of direction in rad

    ANT ant;
    ant.x = x;
    ant.y = y;
    ant.direction = direction;
    ant.phormone = 0;

    while (1)
    {
        // TODO:  y+=speed*sin(direction) ?

        //cout << "x " << x << " y " << y << " direction " << direction / M_PI * 180 << endl;

        usleep(100000 / speed); // for production change to 1000000 / speed

        if (hitWall(x + 0.01 * cos(direction), y + 0.01 * sin(direction)))
        {
            int d = ((rand() % 2) * 2 - 1);     // get random value between -1 for CW and 1 CCW
            direction += (45 * M_PI / 180) * d; // turn 45 degrees
            if (direction > (2 * M_PI))
                direction -= (2 * M_PI);
            while (hitWall(x + 0.01 * cos(direction), y + 0.01 * sin(direction)))
            {
                direction += (45 * M_PI / 180) * d; // turn 45 degrees
            }
        }

        // Check if at food
        for (unsigned i = 0; i < foodPieces.size(); i++)
        {
            while (sqrt(pow(x - foodPieces[i].x, 2) + pow(x - foodPieces[i].x, 2)) <= 0.02)
            {
                pthread_mutex_lock(&foodPieces[i].portions_mutex);
                foodPieces[i].numOfPortions--;
                if (foodPieces[i].numOfPortions == 0)
                {
                    foodPieces.erase(foodPieces.begin() + i);
                }
                else
                {
                    pthread_mutex_unlock(&foodPieces[i].portions_mutex);
                }
            }
        }
        // TODO: Cancel sending phermones

        // Check if food is near
        for (unsigned i = 0; i < foodPieces.size(); i++)
        {
            double distance = pow(x - foodPieces[i].x, 2) + pow(x - foodPieces[i].x, 2);
            if (distance <= pow(ANT_SMELL_DISTANCE, 2))
            {
                direction = atan((y - foodPieces[i].y) / (x - foodPieces[i].x));
                ant.direction = direction;
                ant.phormone = 1; // strong phermone
                ant.foodX = foodPieces[i].x;
                ant.foodY = foodPieces[i].y;
            }
        }

        // TODO: MERGE INTO ONE LOOP

        // Check if affected by phermone 1, change direction to food
        for (unsigned i = 0; i < ants.size(); i++)
        {
            if(ants[i].phormone != 1){
                continue;
            }
            double distance = pow(x - ants[i].x, 2) + pow(x - ants[i].x, 2);
            if (distance <= pow(STRONG_PHEROMONE_THRESHOLD, 2))
            {
                // GO TO FOOD
                direction = atan((y - ants[i].foodY) / (x - ants[i].foodX));
                ant.direction = direction;
                // SEND WEAK PHERMONE
                ant.phormone = 2;
                ant.foodX = foodPieces[i].x;
                ant.foodY = foodPieces[i].y;
            }
        }

        // TODO: Check if affected by phermone 2, change direct by 5 degrees toward the food
        for (unsigned i = 0; i < ants.size(); i++)
        {
            if(ants[i].phormone != 2){
                continue;
            }
            double distance = pow(x - ants[i].x, 2) + pow(x - ants[i].x, 2);
            if (distance <= pow(WEAK_PHEROMONE_THRESHOLD, 2))
            {
                // GO TO 5 degrees closer to food

                double angleToFood = atan((y - ants[i].foodY) / (x - ants[i].foodX));
				// if (angleToFood - direction < -M_PI || (angleToFood - direction < M_PI) && (angleToFood - direction > 0))
				// 	direction += 5 * M_PI / 180;
				// else if ((angleToFood - direction > -M_PI) && (angleToFood - direction < 0) || angleToFood - direction > M_PI)
				// 	direction -= 5 * M_PI / 180;

                ant.direction = direction;

                // SEND WEAK PHERMONE

                ant.phormone = 0; //TOOD: CHECK
            }
        }

        y += 0.01 * sin(direction);
        x += 0.01 * cos(direction);
        ant.x = x;
        ant.y = y;
    }
}

// Food creator thread creates food every interval of time
void *foodCreator(void *data)
{
    while (1)
    {
        int portions = ceil(100.0 / ANT_APPETITE);

        for (int i = 0; i < PIECES_OF_FOOD; i++)
        {
            FOOD food;
            food.x = randomDouble(-4, 4); // rand from -4 -> 4
            food.y = randomDouble(-2, 2); // rand from -2 -> 2
            food.numOfPortions = portions;
            food.portions_mutex = PTHREAD_MUTEX_INITIALIZER;
            foodPieces.push_back(food);
        	cout << "Spawned food at x " << food.x << ", y " << food.y << endl;
        }

        sleep(FOOD_DWELL_TIME);
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
    printf("PIECES_OF_FOOD: %d\n", PIECES_OF_FOOD);

    srand(getpid());
    int antsId[NUMBER_OF_ANTS];
    int foodId;
    pthread_t antsThread[NUMBER_OF_ANTS];
    pthread_t foodThread;

    for (int i = 0; i < NUMBER_OF_ANTS; i++)
    {
        antsId[i] = pthread_create(&antsThread[i], NULL, antLifeCycle, (void *)&i);
    }
    foodId = pthread_create(&foodThread, NULL, foodCreator, 0);

    for (int i = 0; i < NUMBER_OF_ANTS; i++)
        pthread_join(antsThread[i], NULL);

    pthread_join(foodThread, NULL);

    // for(int i=0; i<foodPieces.size(); i++){
    //     cout<< foodPieces[i].x << "\t" << foodPieces[i].y << "\t" << foodPieces[i].numOfPortions<< endl;
    // }

    // pthread_mutex_lock(&foodPieces[1].portions_mutex);
    // foodPieces[1].numOfPortions--;
    // pthread_mutex_unlock(&foodPieces[1].portions_mutex);

    return 0;
}

// TODO: make area user definable
bool hitWall(double x, double y)
{
    if (x <= -4 || x >= 4 || y <= -2 || y >= 2)
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
        else if (variableName == "FOOD_DWELL_TIME")
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
        else if (variableName == "PIECES_OF_FOOD")
        {
            PIECES_OF_FOOD = min(stoi(value), 40);
        }
    }
    inputVariableFile.close();
}
double randomDouble()
{
    return (double)(rand()) / (double)(RAND_MAX);
}

int randomInt(int a, int b)
{
    if (a > b)
        return randomInt(b, a);
    if (a == b)
        return a;
    return a + (rand() % (b - a));
}

double randomDouble(int a, int b)
{
    if (a > b)
        return randomDouble(b, a);
    if (a == b)
        return a;

    return (double)randomInt(a, b) + randomDouble();
}
