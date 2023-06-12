#include "local.hpp"

#define HITBOX 0.02

using namespace std;

// Global Variables
int NUMBER_OF_ANTS;
int FOOD_DWELL_TIME;
int ANT_SMELL_DISTANCE;
int STRONG_PHEROMONE_THRESHOLD;
int WEAK_PHEROMONE_THRESHOLD;
double MAX_PHEROMONE_AMOUNT;
int ANT_APPETITE;
int RUN_TIME;
int PIECES_OF_FOOD;
pthread_mutex_t ants_list_mutex;

vector<FOOD> foodPieces;
vector<ANT> ants;

void *antLifeCycle(void *data)
{
    srand(pthread_self());

    int speed = 9; // for production change to: (rand() % 10) + 1;
    cout << "speed: " << speed << endl;

    double x = randomDouble(-2, 2); // rand from -2 -> 2
    double y = randomDouble(-1, 1); // rand from -1 -> 1
    //
    double direction = ((rand() % 8) * 45) * M_PI / 180; // Angle of direction in rad
    int index = 0;
    ANT ant;
    ant.x = x;
    ant.y = y;
    ant.direction = direction;
    ant.pheromone = 0;

    // to get the index for the current ant
    pthread_mutex_lock(&ants_list_mutex);
    ants.push_back(ant);
    index = ants.size() - 1;
    pthread_mutex_unlock(&ants_list_mutex);

    while (1)
    {
        // cout << "x " << x << " y " << y << " direction " << direction / M_PI * 180 << endl;

        usleep(100000 / speed); // for production change to 1000000 / speed

        if (hitWall(x + 0.01 * cos(direction), y + 0.01 * sin(direction)))
        {
            int d = ((rand() % 2) * 2 - 1);     // get random value between -1 for CW and 1 CCW
            direction += (45 * M_PI / 180) * d; // turn 45 degrees
            if (direction > (2 * M_PI))
                direction -= (2 * M_PI);
            else if (direction < 0)
                direction += (2 * M_PI);
            while (hitWall(x + 0.01 * cos(direction), y + 0.01 * sin(direction)))
            {
                direction += (45 * M_PI / 180) * d; // turn 45 degrees
                usleep(100000 / speed);             // for production change to 1000000 / speed
            }
        }

        int closestFood = -1;
        bool isOnFood = false;
        // TODO: why 5 ?
        double closestFoodDistance = 5;

        // Iterate over food pieces
        for (unsigned i = 0; i < foodPieces.size(); i++)
        {
            double distance = sqrt(pow(x - foodPieces[i].x, 2) + pow(y - foodPieces[i].y, 2));

            if (distance <= HITBOX)
            {
                isOnFood = true;
                closestFood = i;
            }
            // Check if food is within smell distance
            else if (distance <= ANT_SMELL_DISTANCE && distance < closestFoodDistance)
            {
                closestFoodDistance = distance;
                closestFood = i;
            }
        }

        // Check if on food
        if (isOnFood)
        {
            // send strong phermone
            // min is used so we don't have 0 for distance
            double distance = min(sqrt(pow(x - foodPieces[closestFood].x, 2) + pow(y - foodPieces[closestFood].y, 2)), HITBOX);
            ants[index].pheromone = 1 / HITBOX + 1 / distance;

            while (foodPieces.size() && sqrt(pow(x - foodPieces[closestFood].x, 2) + pow(y - foodPieces[closestFood].y, 2)) <= HITBOX) // Hitbox size can be changed
            {
                // TODO: change mutex logic ?
                pthread_mutex_lock(&foodPieces[closestFood].portions_mutex);
                foodPieces[closestFood].numOfPortions--;
                if (foodPieces[closestFood].numOfPortions == 0)
                {
                    // TODO: what happens when another ant is waiting for the erased piece mutex?
                    // TODO: pthread_mutex_unlock(&foodPieces[closestFood].portions_mutex); ???
                    // TODO: stop sending phermones
                    foodPieces.erase(foodPieces.begin() + closestFood);
                    closestFood = -1;
                    // break;
                }
                else
                {
                    pthread_mutex_unlock(&foodPieces[closestFood].portions_mutex);
                }
            }
        }
        // GO TO FOOD
        else if (closestFood != -1)
        {

            direction = atan((y - foodPieces[closestFood].y) / (x - foodPieces[closestFood].x));
            ants[index].direction = direction;
            // SEND PHERMONE
            ants[index].foodX = foodPieces[closestFood].x;
            ants[index].foodY = foodPieces[closestFood].y;
            // send strong phermone
            double distance = min(sqrt(pow(x - foodPieces[closestFood].x, 2) + pow(y - foodPieces[closestFood].y, 2)), HITBOX);
            ants[index].pheromone = 1 / HITBOX + 1 / distance;
        }
        else
        {
            // Check if affected by pheromone, change direction
            int antWithStrongestPheromone;
            double strongestPheromone = 0;
            // TODO: why 5?
            double distanceToAnt = 5;
            for (unsigned i = 0; i < ants.size(); i++)
            {
                double distance = sqrt(pow(x - ants[i].x, 2) + pow(y - ants[i].y, 2));

                double recieved_pheromone = ants[i].pheromone / distance;

                if (recieved_pheromone < WEAK_PHEROMONE_THRESHOLD)
                {
                    continue;
                }

                else if (recieved_pheromone > strongestPheromone)
                {
                    strongestPheromone = recieved_pheromone;
                    distanceToAnt = distance;
                    antWithStrongestPheromone = i;
                }
            }

            if (strongestPheromone >= STRONG_PHEROMONE_THRESHOLD)
            {
                // GO TO FOOD
                direction = atan((y - ants[antWithStrongestPheromone].foodY) / (x - ants[antWithStrongestPheromone].foodX));
                ants[index].direction = direction;
                // TODO: to change
                // ants[index].pheromone = 0.2 * (MAX_PHEROMONE_AMOUNT < (1 / distanceToAnt) ? MAX_PHEROMONE_AMOUNT : 1 / distanceToAnt);
                ants[index].pheromone = 1 / distanceToAnt;
                ants[index].foodX = ants[antWithStrongestPheromone].foodX;
                ants[index].foodY = ants[antWithStrongestPheromone].foodY;
            }
            else
            {
                // GO TO 5 degrees closer to food
                double angleToFood = atan((y - ants[antWithStrongestPheromone].foodY) / (x - ants[antWithStrongestPheromone].foodX));
                if (angleToFood - direction < -M_PI || ((angleToFood - direction < M_PI) && (angleToFood - direction > 0)))
                    direction += 5 * M_PI / 180;
                else
                    direction -= 5 * M_PI / 180;

                ants[index].direction = direction;
                ants[index].pheromone = 0; // Don't send pheromone
            }
        }

        y += 0.01 * sin(direction);
        x += 0.01 * cos(direction);
        ants[index].x = x;
        ants[index].y = y;
    }
}

// Food creator thread creates food every interval of time
void *foodCreator(void *data)
{
    srand(pthread_self());
    while (1)
    {
        int portions = ceil(100.0 / ANT_APPETITE);

        for (int i = 0; i < PIECES_OF_FOOD; i++)
        {
            FOOD food;
            food.x = randomDouble(-2, 2); // rand from -4 -> 4
            food.y = randomDouble(-1, 1); // rand from -2 -> 2
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
    printf("MAX_PHEROMONE_AMOUNT: %f\n", MAX_PHEROMONE_AMOUNT);
    printf("ANT_APPETITE: %d\n", ANT_APPETITE);
    printf("RUN_TIME: %d\n", RUN_TIME);
    printf("PIECES_OF_FOOD: %d\n", PIECES_OF_FOOD);

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
        else if (variableName == "MAX_PHEROMONE_AMOUNT")
        {
            MAX_PHEROMONE_AMOUNT = min(stoi(value), 40);
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
