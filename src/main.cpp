#include "local.hpp"
#include "opgl.hpp"
#define HITBOX 0.08
#define STEP_SIZE 0.003
#define REC_INIT_SIZE 0.05

using namespace std;

// Global Variables
int NUMBER_OF_ANTS;
int SPEED_LIMIT;
int NUM_OF_DIRECTIONS;
int CHANGE_DIRECTION_ANGLE;
int SMALL_ANGLE;
int FOOD_DWELL_TIME;
double ANT_SMELL_DISTANCE;
int STRONG_PHEROMONE_THRESHOLD;
int WEAK_PHEROMONE_THRESHOLD;
int ANT_APPETITE;
int PIECES_OF_FOOD;

bool notTerminated = true;
vector<FOOD> foodPieces;
vector<ANT> ants;
pthread_mutex_t ants_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t food_list_mutex = PTHREAD_MUTEX_INITIALIZER;

using namespace std::chrono;

void *antLifeCycle(void *data)
{
    srand(pthread_self());
    auto epoch = high_resolution_clock::from_time_t(0);

    int speed = (rand() % SPEED_LIMIT) + 1;
    ANT ant;
    ant.x = randomDouble(-X_BORDER, X_BORDER);
    ant.y = randomDouble(-Y_BORDER, Y_BORDER);
    ant.direction = ((rand() % NUM_OF_DIRECTIONS) * (360.0 / NUM_OF_DIRECTIONS)) * M_PI / 180; // Angle of direction in rad
    removePheromone(ant);

    unsigned index = 0;
    pthread_mutex_lock(&ants_list_mutex);
    ants.push_back(ant);
    index = ants.size() - 1;
    pthread_mutex_unlock(&ants_list_mutex);

    while (notTerminated)
    {
        usleep(16666 / (1 + 0.1 * speed)); // for production change to 1000000 / speed // TODO-Shahd: Remove hardcoded numbers

        int closestFood = -1;
        bool isOnFood = false;

        getClosestFood(ant, &isOnFood, &closestFood);

        if (isOnFood)
        {
            sendStrongPheromone(ant, closestFood);
            eatFood(ant, &closestFood);
        }
        else if (closestFood != -1)
        {

            ant.direction = findAngle(ant.x, ant.y, foodPieces[closestFood].x, foodPieces[closestFood].y);
            ant.foodX = foodPieces[closestFood].x;
            ant.foodY = foodPieces[closestFood].y;
            sendStrongPheromone(ant, closestFood);
        }
        else // Check if affected by pheromone, change direction
        {
            int antWithStrongestPheromone = -1;
            double strongestPheromoneEffect = 0;
            double distanceToAnt = sqrt(pow(2 * X_BORDER, 2) + pow(2 * Y_BORDER, 2)) + 1;

            getStrongestAntEffect(&index, ant, &strongestPheromoneEffect, &distanceToAnt, &antWithStrongestPheromone);

            if (strongestPheromoneEffect >= STRONG_PHEROMONE_THRESHOLD)
            {
                ant.direction = findAngle(ant.x, ant.y, ants[antWithStrongestPheromone].foodX, ants[antWithStrongestPheromone].foodY);
                ant.pheromone = 1 / (100 * max(distanceToAnt, HITBOX));
                ant.foodX = ants[antWithStrongestPheromone].foodX;
                ant.foodY = ants[antWithStrongestPheromone].foodY;
            }
            else if (antWithStrongestPheromone != -1)
            {
                auto now = high_resolution_clock::now();
                auto mseconds = duration_cast<milliseconds>(now - epoch).count();
                if (mseconds >= 1000)
                {
                    rotateSmallDegrees(ant, &antWithStrongestPheromone);
                    epoch = now;
                }

                removePheromone(ant);
            }
            else
            {
                removePheromone(ant);
            }
        }

        int d = ((rand() % 2) * 2 - 1); // get random value between -1 for CW and 1 CCW
        while (hitWall(ant.x + STEP_SIZE * cos(ant.direction), ant.y + STEP_SIZE * sin(ant.direction)))
        {
            ant.direction += (CHANGE_DIRECTION_ANGLE * M_PI / 180) * d; // turn 45 degrees TODO
            if (ant.direction > (2 * M_PI))
                ant.direction -= (2 * M_PI);
            else if (ant.direction < 0)
                ant.direction += (2 * M_PI);
            usleep(16666 / (1 + 0.1 * speed)); // for production change to 1000000 / speed
        }

        if (ant.direction >= (2 * M_PI))
            ant.direction -= (2 * M_PI);
        else if (ant.direction < 0)
            ant.direction += (2 * M_PI);

        ant.x += STEP_SIZE * cos(ant.direction);
        ant.y += STEP_SIZE * sin(ant.direction);

        ants[index].x = ant.x;
        ants[index].y = ant.y;
        ants[index].direction = ant.direction;
        ants[index].pheromone = ant.pheromone;
        ants[index].foodX = ant.foodX;
        ants[index].foodY = ant.foodY;
    }
    return NULL;
}

// Food creator thread creates food every interval of time
void *foodCreator(void *data)
{
    srand(pthread_self());
    while (notTerminated)
    {
        int portions = ceil(100.0 / ANT_APPETITE);

        for (int i = 0; i < PIECES_OF_FOOD; i++)
        {
            FOOD food;
            food.x = randomDouble(-X_BORDER, X_BORDER);
            food.y = randomDouble(-Y_BORDER, Y_BORDER);
            food.numOfPortions = portions;
            food.portions_mutex = PTHREAD_MUTEX_INITIALIZER;
            pthread_mutex_lock(&food_list_mutex);
            foodPieces.push_back(food);
            pthread_mutex_unlock(&food_list_mutex);
            cout << "Spawned food at x " << food.x << ", y " << food.y << endl;
        }

        sleep(FOOD_DWELL_TIME);
    }
    return NULL;
}

int main(int argc, char *argv[])
{
    read_constants("./conf.txt");
    printf("NUMBER_OF_ANTS: %d\n", NUMBER_OF_ANTS);
    printf("SPEED_LIMIT: %d\n", SPEED_LIMIT);
    printf("CHANGE_DIRECTION_ANGLE: %d\n", CHANGE_DIRECTION_ANGLE);
    printf("SMALL_ANGLE: %d\n", SMALL_ANGLE);
    printf("NUM_OF_DIRECTIONS: %d\n", NUM_OF_DIRECTIONS);
    printf("FOOD_DWELL_TIME: %d\n", FOOD_DWELL_TIME);
    printf("ANT_SMELL_DISTANCE: %f\n", ANT_SMELL_DISTANCE);
    printf("STRONG_PHEROMONE_THRESHOLD: %d\n", STRONG_PHEROMONE_THRESHOLD);
    printf("WEAK_PHEROMONE_THRESHOLD: %d\n", WEAK_PHEROMONE_THRESHOLD);
    printf("ANT_APPETITE: %d\n", ANT_APPETITE);
    printf("RUN_TIME: %d\n", RUN_TIME);
    printf("PIECES_OF_FOOD: %d\n", PIECES_OF_FOOD);

    pthread_t antsThread[NUMBER_OF_ANTS];
    pthread_t foodThread;
    pthread_t openGlThread;

    for (int i = 0; i < NUMBER_OF_ANTS; i++)
    {
        pthread_create(&antsThread[i], NULL, antLifeCycle, (void *)&i);
    }
    pthread_create(&foodThread, NULL, foodCreator, 0);

    pthread_create(&openGlThread, NULL, opengl, 0);

    sleep(RUN_TIME * 60);

    notTerminated = false;

    for (int i = 0; i < NUMBER_OF_ANTS; i++)
        pthread_join(antsThread[i], NULL);

    pthread_join(openGlThread, NULL);

    return 0;
}

void getClosestFood(ANT ant, bool *isOnFood, int *closestFood)
{

    double closestFoodDistance = sqrt(pow(2 * X_BORDER, 2) + pow(2 * Y_BORDER, 2)) + 1;

    for (unsigned i = 0; i < foodPieces.size(); i++)
    {
        if (foodPieces[i].numOfPortions == 0)
            continue;
        double distance = sqrt(pow(ant.x - foodPieces[i].x, 2) + pow(ant.y - foodPieces[i].y, 2));

        if (distance <= HITBOX)
        {
            *isOnFood = true;
            *closestFood = i;
        }
        else if (distance <= ANT_SMELL_DISTANCE && distance < closestFoodDistance)
        {
            closestFoodDistance = distance;
            *closestFood = i;
        }
    }
}

void eatFood(ANT &ant, int *closestFood)
{
    while (sqrt(pow(ant.x - foodPieces[*closestFood].x, 2) + pow(ant.y - foodPieces[*closestFood].y, 2)) <= HITBOX)
    {
        pthread_mutex_lock(&foodPieces[*closestFood].portions_mutex);
        if (foodPieces[*closestFood].numOfPortions > 0)
        {
            foodPieces[*closestFood].numOfPortions--;
            pthread_mutex_unlock(&foodPieces[*closestFood].portions_mutex);
            sleep(1);
        }
        else
        {
            pthread_mutex_unlock(&foodPieces[*closestFood].portions_mutex);
            for (int k = 0; k < NUMBER_OF_ANTS; k++)
            {
                if (ants[k].foodX == foodPieces[*closestFood].x && ants[k].foodY == foodPieces[*closestFood].y)
                {
                    ants[k].pheromone = 0;
                }
            }
            removePheromone(ant);
            *closestFood = -1;
            break;
        }
    }
}

void getStrongestAntEffect(unsigned *index, ANT &ant, double *strongestPheromone, double *distanceToAnt, int *antWithStrongestPheromone)
{
    removePheromone(ant);
    for (unsigned i = 0; i < ants.size(); i++)
    {
        if (i == *index)
            continue;

        double distance = max(sqrt(pow(ant.x - ants[i].x, 2) + pow(ant.y - ants[i].y, 2)), HITBOX);

        double recievedPheromone = ants[i].pheromone / distance;

        if (recievedPheromone < WEAK_PHEROMONE_THRESHOLD)
        {
            continue;
        }

        else if (recievedPheromone > *strongestPheromone)
        {
            *strongestPheromone = recievedPheromone;
            *distanceToAnt = distance;
            *antWithStrongestPheromone = i;
        }
    }
}

void rotateSmallDegrees(ANT &ant, int *antWithStrongestPheromone)
{
    double angleToFood = findAngle(ant.x, ant.y, ants[*antWithStrongestPheromone].foodX, ants[*antWithStrongestPheromone].foodY);
    if (abs(angleToFood - ant.direction) < SMALL_ANGLE * M_PI / 180)
    {
        return;
    }

    if (angleToFood - ant.direction < -M_PI || ((angleToFood - ant.direction < M_PI) && (angleToFood - ant.direction > 0)))
    {
        ant.direction += SMALL_ANGLE * M_PI / 180;
    }
    else
        ant.direction -= SMALL_ANGLE * M_PI / 180;
}

void sendStrongPheromone(ANT &ant, int closestFood)
{
    double distance = max(sqrt(pow(ant.x - foodPieces[closestFood].x, 2) + pow(ant.y - foodPieces[closestFood].y, 2)), HITBOX);
    ant.pheromone = 0.1 * (1 / HITBOX + 1 / distance);
}

// TODO: make area user definable
bool hitWall(double x, double y)
{
    if (x <= -X_BORDER || x >= X_BORDER || y <= -Y_BORDER || y >= Y_BORDER)
    {
        return true;
    }
    return false;
}

// function to find angle between two points based on their x, y coordinates
double findAngle(double x1, double y1, double x2, double y2)
{
    double angle = atan2(y2 - y1, x2 - x1);
    if (angle < 0)
        angle += 2 * M_PI;
    return angle;
}

void removePheromone(ANT &ant)
{
    ant.pheromone = 0;
    ant.foodX = -X_BORDER - 1;
    ant.foodY = -Y_BORDER - 1;
}

// TODO remove mins
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
            if (!(isdigit(value[i]) || value[i] == '.'))
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
            NUMBER_OF_ANTS = min(stoi(value), 1500);
        }
        else if (variableName == "SPEED_LIMIT")
        {
            SPEED_LIMIT = min(stoi(value), 10000);
        }
        else if (variableName == "CHANGE_DIRECTION_ANGLE")
        {
            CHANGE_DIRECTION_ANGLE = stoi(value);
        }
        else if (variableName == "SMALL_ANGLE")
        {
            SMALL_ANGLE = stoi(value);
        }
        else if (variableName == "NUM_OF_DIRECTIONS")
        {
            NUM_OF_DIRECTIONS = stoi(value);
        }
        else if (variableName == "FOOD_DWELL_TIME")
        {
            FOOD_DWELL_TIME = stoi(value);
        }
        else if (variableName == "ANT_SMELL_DISTANCE")
        {
            ANT_SMELL_DISTANCE = stof(value);
        }
        else if (variableName == "STRONG_PHEROMONE_THRESHOLD")
        {
            STRONG_PHEROMONE_THRESHOLD = stoi(value);
        }
        else if (variableName == "WEAK_PHEROMONE_THRESHOLD")
        {
            WEAK_PHEROMONE_THRESHOLD = stoi(value);
        }
        else if (variableName == "ANT_APPETITE")
        {
            ANT_APPETITE = min(stoi(value), 100);
        }
        else if (variableName == "RUN_TIME")
        {
            RUN_TIME = stoi(value);
        }
        else if (variableName == "PIECES_OF_FOOD")
        {
            PIECES_OF_FOOD = stoi(value);
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

// GLUT display function
void display()
{
    modifyMatrix();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0f, 0.0f, 0.0f);

    glColor3f(0.0f, 0.78f, 0.0f);
    drawRectangle(0, 0, 10, 2, true);

    for (int i = 0; i < NUMBER_OF_ANTS; i++)
    {
        drawAnt(ants[i].x, ants[i].y, (ants[i].direction * 180.0 / M_PI) + 270);
    }
    glColor3f(1.0f, 1.0f, 1.0f);

    pthread_mutex_lock(&food_list_mutex);
    for (unsigned i = 0; i < foodPieces.size(); i++)
    {
        if (foodPieces[i].numOfPortions <= 0)
            continue;
        double length = REC_INIT_SIZE * foodPieces[i].numOfPortions * ANT_APPETITE / 100;
        drawRectangle(foodPieces[i].x, foodPieces[i].y, length, length, false);
    }
    pthread_mutex_unlock(&food_list_mutex);

    glColor3f(1.0f, 1.0f, 1.0f);
    drawRectangle(0, 1.25, 4, 0.5, false);
    renderText(TIME, 0, 1.15, 24);

    if (!notTerminated)
    {
        drawRectangle(0, 0, 2, 1, true);
        renderText("TIMEOUT", 0, 0, 60);
    }

    glutSwapBuffers();
}

void *opengl(void *arg)
{
    int argc = 0;
    char *argv[1] = {nullptr};
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Ants Simulation");

    initFreeType();

    glutDisplayFunc(display);
    glutReshapeFunc(reshape);
    glutTimerFunc(0, updateTimer, 0);

    start = clk::now();
    glutMainLoop();

    // Cleanup FreeType resources
    FT_Done_Face(fontFace);
    FT_Done_FreeType(ftLibrary);

    return 0;
}
