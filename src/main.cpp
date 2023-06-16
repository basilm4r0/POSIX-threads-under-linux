#include "local.hpp"
#include "opgl.hpp"
#define HITBOX 0.08
#define STEP_SIZE 0.003
#define REC_INIT_SIZE 0.05

using namespace std;
using namespace std::chrono;

/* Define variables extracted from conf.txt */
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
vector<FOOD> foodPieces; /* Contains all food pieces that appear */
vector<ANT> ants;        /* Define all the created ants */

/* Define mutexes used in lists */
pthread_mutex_t ants_list_mutex = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t food_list_mutex = PTHREAD_MUTEX_INITIALIZER;

/* This function contains all the functionalities of an ant */
void *antLifeCycle(void *data)
{
    srand(pthread_self()); /* Define a random seed related to the thread using the thread ID */

    auto epoch = high_resolution_clock::from_time_t(0); /* start counting to use time for small degrees rotation */

    int speed = (rand() % SPEED_LIMIT) + 1; /* Generate random speed */

    ANT ant;
    /* Generate random coordinations */
    ant.x = randomDouble(-X_BORDER, X_BORDER);
    ant.y = randomDouble(-Y_BORDER, Y_BORDER);
    /* Generate a random direction */
    ant.direction = ((rand() % NUM_OF_DIRECTIONS) * (360.0 / NUM_OF_DIRECTIONS)) * M_PI / 180; // Angle of direction in rad
    /*Initialize pheromone */
    removePheromone(ant);

    /* Add the ant to the list and get the index */
    unsigned index = 0;
    pthread_mutex_lock(&ants_list_mutex);
    ants.push_back(ant);
    index = ants.size() - 1;
    pthread_mutex_unlock(&ants_list_mutex);

    while (notTerminated) /* Runtime didn't elapse yet */
    {
        usleep(16666 / (0.9 + 0.1 * speed)); /* Sleep between movements */

        /* Get the closest food within the smell distance */
        int closestFood = -1;
        bool isOnFood = false;
        getClosestFood(ant, &isOnFood, &closestFood);

        if (isOnFood) /* if an ant is on a food piece */
        {
            sendStrongPheromone(ant, closestFood); /* Send strong pheromone to attract other ants */
            eatFood(ant, &closestFood);            /* Eat the food piece */
        }
        else if (closestFood != -1) /* not on a food piece, but within a smell distance */
        {

            ant.direction = findAngle(ant.x, ant.y, foodPieces[closestFood].x, foodPieces[closestFood].y); /* Find angle between ant and food */
            ant.foodX = foodPieces[closestFood].x;
            ant.foodY = foodPieces[closestFood].y;
            sendStrongPheromone(ant, closestFood); /* Send strong pheromone to atrract other ants */
        }
        else /* possible to be affected by other ants */
        {
            int antWithStrongestPheromone = -1;
            double strongestPheromoneEffect = 0;
            double distanceToAnt = sqrt(pow(2 * X_BORDER, 2) + pow(2 * Y_BORDER, 2)) + 1; /* Initilize with max distance possible */

            /* Get the ant affected with the most */
            getStrongestAntEffect(&index, ant, &strongestPheromoneEffect, &distanceToAnt, &antWithStrongestPheromone);

            if (strongestPheromoneEffect >= STRONG_PHEROMONE_THRESHOLD) /* if ant is affected by a strong pheromone */
            {
                ant.direction = findAngle(ant.x, ant.y, ants[antWithStrongestPheromone].foodX, ants[antWithStrongestPheromone].foodY); /* Find angle between ant and food */
                sendWeakPheromone(ant, antWithStrongestPheromone, distanceToAnt); /* Send weak pheromone */
            }
            else if (antWithStrongestPheromone != -1) /* if ant is affected by a weak pheromone */
            {
                auto now = high_resolution_clock::now();
                auto mseconds = duration_cast<milliseconds>(now - epoch).count();
                if (mseconds >= 1000) /* to rotate only every second */
                {
                    rotateSmallDegrees(ant, &antWithStrongestPheromone);
                    epoch = now;
                }

                removePheromone(ant); /* Remove pherpmone effect */
            }
            else
            {
                removePheromone(ant); /* Remove pheromone effect */
            }
        }

        int d = ((rand() % 2) * 2 - 1); /* get random value between -1 for CW and 1 CCW */

        while (hitWall(ant.x + STEP_SIZE * cos(ant.direction), ant.y + STEP_SIZE * sin(ant.direction))) /* Change direction until a valid change */
        {
            ant.direction += (CHANGE_DIRECTION_ANGLE * M_PI / 180) * d; 
            if (ant.direction > (2 * M_PI))
                ant.direction -= (2 * M_PI);
            else if (ant.direction < 0)
                ant.direction += (2 * M_PI);
            usleep(16666 / (1 + 0.1 * speed)); // for production change to 1000000 / speed
        }

        /* Keep direction angle in range between 0 and 2*PI */
        if (ant.direction >= (2 * M_PI))
            ant.direction -= (2 * M_PI);
        else if (ant.direction < 0)
            ant.direction += (2 * M_PI);

        /* Move the ant one step in the correct direction */
        ant.x += STEP_SIZE * cos(ant.direction);
        ant.y += STEP_SIZE * sin(ant.direction);

        /* Update ant values in the list */
        ants[index].x = ant.x;
        ants[index].y = ant.y;
        ants[index].direction = ant.direction;
        ants[index].pheromone = ant.pheromone;
        ants[index].foodX = ant.foodX;
        ants[index].foodY = ant.foodY;
    }
    return NULL;
}

/* This function create a user-defined pieces of food every user defined interval */
void *foodCreator(void *data)
{
    srand(pthread_self()); /* Define a random seed related to the thread using the thread ID */
    while (notTerminated) /* Runtime didn't elapse yet */
    {
        int portions = ceil(100.0 / ANT_APPETITE); /* Get the number of portions from the user defined percentage */

        for (int i = 0; i < PIECES_OF_FOOD; i++)
        {
            FOOD food;
            /* Generate random coordinations */
            food.x = randomDouble(-X_BORDER, X_BORDER);
            food.y = randomDouble(-Y_BORDER, Y_BORDER);
            food.numOfPortions = portions;
            food.portions_mutex = PTHREAD_MUTEX_INITIALIZER;
            /* Add food piece to the list */
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

    /* Create ants threads */
    for (int i = 0; i < NUMBER_OF_ANTS; i++)
    {
        pthread_create(&antsThread[i], NULL, antLifeCycle, (void *)&i);
    }
    /* Create food thread */
    pthread_create(&foodThread, NULL, foodCreator, 0);

    /* Create opengl thread */
    pthread_create(&openGlThread, NULL, opengl, 0);

    sleep(RUN_TIME * 60);

    notTerminated = false;

    for (int i = 0; i < NUMBER_OF_ANTS; i++)
        pthread_join(antsThread[i], NULL);

    pthread_join(openGlThread, NULL);

    return 0;
}

/* This function returns the closest piee of food within the smell distance */
void getClosestFood(ANT ant, bool *isOnFood, int *closestFood)
{
    double closestFoodDistance = sqrt(pow(2 * X_BORDER, 2) + pow(2 * Y_BORDER, 2)) + 1;

    for (unsigned i = 0; i < foodPieces.size(); i++)
    {
        if (foodPieces[i].numOfPortions == 0) /* If food piece was eaten */
            continue;
        
        double distance = sqrt(pow(ant.x - foodPieces[i].x, 2) + pow(ant.y - foodPieces[i].y, 2)); /* Calculate distnace between the ant and the food */

        if (distance <= HITBOX) /* Ant is on food */
        {
            *isOnFood = true;
            *closestFood = i;
        }
        else if (distance <= ANT_SMELL_DISTANCE && distance < closestFoodDistance) /* Ant is within the smell distance and closest so far */
        {
            closestFoodDistance = distance;
            *closestFood = i;
        }
    }
}

/* This function makes the ant eat from the piece of food */
void eatFood(ANT &ant, int *closestFood)
{
    while (1)
    {
        pthread_mutex_lock(&foodPieces[*closestFood].portions_mutex); /* Lock food portions mutex */
        if (foodPieces[*closestFood].numOfPortions > 0) /* There are still available portions to eat */
        {
            foodPieces[*closestFood].numOfPortions--; /* Eat one portion */
            pthread_mutex_unlock(&foodPieces[*closestFood].portions_mutex); /* Unlock mutex */
            sleep(1); /* Eat one portion every second */
        }
        else
        {
            pthread_mutex_unlock(&foodPieces[*closestFood].portions_mutex);
            for (int k = 0; k < NUMBER_OF_ANTS; k++) /* Remove pheromone effects of all other ants attracted to this food */
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

/* This function returns the ant that has the strongest phoromone effect */
void getStrongestAntEffect(unsigned *index, ANT &ant, double *strongestPheromone, double *distanceToAnt, int *antWithStrongestPheromone)
{
    removePheromone(ant); /* remove previous pheromone effect */
    for (unsigned i = 0; i < ants.size(); i++)
    {
        if (i == *index) /* can't be affected by itself */
            continue;

        double distance = max(sqrt(pow(ant.x - ants[i].x, 2) + pow(ant.y - ants[i].y, 2)), HITBOX); /* Calculate the distance between the ants */

        double recievedPheromone = ants[i].pheromone / distance; /* Calculate the received pheromone considering the distance */

        if (recievedPheromone < WEAK_PHEROMONE_THRESHOLD) /* No pheromones affect it */
        {
            continue;
        }

        else if (recievedPheromone > *strongestPheromone) /* Affected by the strongest pheromone so far */
        {
            *strongestPheromone = recievedPheromone;
            *distanceToAnt = distance;
            *antWithStrongestPheromone = i;
        }
    }
}

/* This function rotates by a user defined degress when affected by a weak pheromone */
void rotateSmallDegrees(ANT &ant, int *antWithStrongestPheromone)
{
    double angleToFood = findAngle(ant.x, ant.y, ants[*antWithStrongestPheromone].foodX, ants[*antWithStrongestPheromone].foodY); /* Calculate the angle between the ant and the food */
    if (abs(angleToFood - ant.direction) < SMALL_ANGLE * M_PI / 180) /* If angle is less than needed to be directed to the food */
    {
        return;
    }

    if (angleToFood - ant.direction < -M_PI || ((angleToFood - ant.direction < M_PI) && (angleToFood - ant.direction > 0))) /* Decide rotation direction */
    {
        ant.direction += SMALL_ANGLE * M_PI / 180;
    }
    else
        ant.direction -= SMALL_ANGLE * M_PI / 180;
}

/* This function updates the pheromone value for an ant to send a strong pheromone */
void sendStrongPheromone(ANT &ant, int closestFood)
{
    double distance = max(sqrt(pow(ant.x - foodPieces[closestFood].x, 2) + pow(ant.y - foodPieces[closestFood].y, 2)), HITBOX);
    ant.pheromone = 0.1 * (1 / HITBOX + 1 / distance);
}

/* This function updates the pheromone value for an ant to send a weak pheromone */
void sendWeakPheromone(ANT &ant, int antWithStrongestPheromone, double distanceToAnt)
{
    ant.pheromone = 1 / (100 * max(distanceToAnt, HITBOX));
    ant.foodX = ants[antWithStrongestPheromone].foodX;
    ant.foodY = ants[antWithStrongestPheromone].foodY;
}

/* Check if the ant hits the boundaries of the screen */
bool hitWall(double x, double y)
{
    if (x <= -X_BORDER || x >= X_BORDER || y <= -Y_BORDER || y >= Y_BORDER)
    {
        return true;
    }
    return false;
}

/* This function returns the angle between two points based on their x, y coordinates */
double findAngle(double x1, double y1, double x2, double y2)
{
    double angle = atan2(y2 - y1, x2 - x1);
    if (angle < 0)
        angle += 2 * M_PI;
    return angle;
}

/* Remove pheromone effect for an ant */
void removePheromone(ANT &ant)
{
    ant.pheromone = 0;
    ant.foodX = -X_BORDER - 1;
    ant.foodY = -Y_BORDER - 1;
}

/* This functions reads the user defined variables from conf.txt */
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

/* Return a random double value */
double randomDouble()
{
    return (double)(rand()) / (double)(RAND_MAX);
}

/* Return a random integer value between a and b */
int randomInt(int a, int b)
{
    if (a > b)
        return randomInt(b, a);
    if (a == b)
        return a;
    return a + (rand() % (b - a));
}

/* Return a random double value between a and b */
double randomDouble(int a, int b)
{
    if (a > b)
        return randomDouble(b, a);
    if (a == b)
        return a;

    return (double)randomInt(a, b) + randomDouble();
}

/* GLUT display function */
void display()
{
    modifyMatrix();

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);

    glColor3f(0.0f, 0.0f, 0.0f);

    glColor3f(0.0f, 0.78f, 0.0f);
    drawRectangle(0, 0, 10, 2, true);

    /* Draw ants on screen */
    for (int i = 0; i < NUMBER_OF_ANTS; i++)
    {
        drawAnt(ants[i].x, ants[i].y, (ants[i].direction * 180.0 / M_PI) + 270);
    }
    glColor3f(1.0f, 1.0f, 1.0f);

    /* Draw food pieces on screen */
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

    /* Display timeout screen when runtime is over */
    if (!notTerminated)
    {
        drawRectangle(0, 0, 2, 1, true);
        renderText("TIMEOUT", 0, 0, 60);
    }

    glutSwapBuffers();
}

/* This function is executed by the opengl thread */
void *opengl(void *arg)
{
    int argc = 0;
    char *argv[1] = {nullptr};
    glutInit(&argc, argv);
    glutInitDisplayMode(GLUT_RGB | GLUT_DOUBLE);
    glutInitWindowSize(800, 600);
    glutCreateWindow("Ants Simulator");

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
