#include "local.hpp"
#include "opgl.hpp"
#define HITBOX 0.05
#define STEP_SIZE 0.01

using namespace std;

// Global Variables
int NUMBER_OF_ANTS;
int FOOD_DWELL_TIME;
double ANT_SMELL_DISTANCE;
int STRONG_PHEROMONE_THRESHOLD;
int WEAK_PHEROMONE_THRESHOLD;
double MAX_PHEROMONE_AMOUNT;
int ANT_APPETITE;
int RUN_TIME;
int PIECES_OF_FOOD;
pthread_mutex_t ants_list_mutex;
pthread_mutex_t food_list_mutex;

vector<FOOD> foodPieces;
vector<ANT> ants;

void *antLifeCycle(void *data)
{
    srand(pthread_self());

    int speed = 9; // for production change to: (rand() % 10) + 1;
    // int speed = (rand() % 10) + 1;
    int index = 0;
    ANT ant;
    ant.x = randomDouble(-2, 2);                      // rand from -2 -> 2
    ant.y = randomDouble(-1, 1);                      // rand from -1 -> 1
    ant.direction = ((rand() % 8) * 45) * M_PI / 180; // Angle of direction in rad

    ant.pheromone = 0;

    // to get the index for the current ant
    pthread_mutex_lock(&ants_list_mutex);
    ants.push_back(ant);
    index = ants.size() - 1;
    pthread_mutex_unlock(&ants_list_mutex);
    
    while (1)
    {
        cout << "x " << ant.x << " y " << ant.y << " direction " << ant.direction / M_PI * 180 << " pheromone " << ant.pheromone << endl;

        usleep(100000 / speed); // for production change to 1000000 / speed

        int closestFood = -1;
        bool isOnFood = false;
        // TODO: why 5 ?
        double closestFoodDistance = 5;

        // pthread_mutex_lock(&food_list_mutex);
        // Iterate over food pieces
        for (unsigned i = 0; i < foodPieces.size(); i++)
        {
            double distance = sqrt(pow(ant.x - foodPieces[i].x, 2) + pow(ant.y - foodPieces[i].y, 2));

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
            double distance = max(sqrt(pow(ant.x - foodPieces[closestFood].x, 2) + pow(ant.y - foodPieces[closestFood].y, 2)), HITBOX);
            ant.pheromone = 1 / HITBOX + 1 / distance;

            while (foodPieces.size() && sqrt(pow(ant.x - foodPieces[closestFood].x, 2) + pow(ant.y - foodPieces[closestFood].y, 2)) <= HITBOX) // Hitbox size can be changed
            {
                // TODO: change mutex logic ?
                pthread_mutex_lock(&foodPieces[closestFood].portions_mutex);
                if (foodPieces[closestFood].numOfPortions == 0) // TODO
                {
                    break;
                }
                foodPieces[closestFood].numOfPortions--;
                if (foodPieces[closestFood].numOfPortions == 0)
                {

                    // TODO: what happens when another ant is waiting for the erased piece mutex?
                    // TODO: pthread_mutex_unlock(&foodPieces[closestFood].portions_mutex); ???
                    // TODO: stop sending phermones
                    for (int k = 0; k < NUMBER_OF_ANTS - 1; k++) // TODO
                    {
                        pthread_mutex_unlock(&foodPieces[closestFood].portions_mutex);
                    }
                    pthread_mutex_lock(&food_list_mutex); //TODO where to put? caused other ants to stop on delay when the mutex was for all the code
                    foodPieces.erase(foodPieces.begin() + closestFood);
                    pthread_mutex_unlock(&food_list_mutex);
                    closestFood = -1;
                    // break;
                }
                else
                {
                    pthread_mutex_unlock(&foodPieces[closestFood].portions_mutex);
                    sleep(1);
                }
            }
        }
        // GO TO FOOD
        else if (closestFood != -1)
        {

            ant.direction = findAngle(ant.x, ant.y, foodPieces[closestFood].x, foodPieces[closestFood].y);
            // SEND PHERMONE
            ant.foodX = foodPieces[closestFood].x;
            ant.foodY = foodPieces[closestFood].y;
            // send strong phermone
            double distance = max(sqrt(pow(ant.x - foodPieces[closestFood].x, 2) + pow(ant.y - foodPieces[closestFood].y, 2)), HITBOX);
            ant.pheromone = 1 / HITBOX + 1 / distance;
        }
        else
        {
            // Check if affected by pheromone, change direction
            int antWithStrongestPheromone = -1;
            double strongestPheromone = 0;
            // TODO: why 5?
            double distanceToAnt = 5;
            // pthread_mutex_lock(&ants_list_mutex);
            for (unsigned i = 0; i < ants.size(); i++)
            {
                if (i == index)
                    continue;
                double distance = sqrt(pow(ant.x - ants[i].x, 2) + pow(ant.y - ants[i].y, 2));

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
                ant.direction = findAngle(ant.x, ant.y, ants[antWithStrongestPheromone].foodY, ants[antWithStrongestPheromone].foodX);
                // TODO: to change
                // ant.pheromone = 0.2 * (MAX_PHEROMONE_AMOUNT < (1 / distanceToAnt) ? MAX_PHEROMONE_AMOUNT : 1 / distanceToAnt);
                ant.pheromone = 1 / max(distanceToAnt, HITBOX);
                ant.foodX = ants[antWithStrongestPheromone].foodX;
                ant.foodY = ants[antWithStrongestPheromone].foodY;
            }
            else if (antWithStrongestPheromone != -1)
            {
                // GO TO 5 degrees closer to food
                double angleToFood = findAngle(ant.x, ant.y, ants[antWithStrongestPheromone].foodY, ants[antWithStrongestPheromone].foodX);
                if (angleToFood - ant.direction < -M_PI || ((angleToFood - ant.direction < M_PI) && (angleToFood - ant.direction > 0)))
                    ant.direction += 5 * M_PI / 180;
                else
                    ant.direction -= 5 * M_PI / 180;

                ant.pheromone = 0; // Don't send pheromone
            }
            else
            {
                ant.pheromone = 0; // Don't send pheromone
            }
            // pthread_mutex_unlock(&ants_list_mutex);
        }
        // pthread_mutex_unlock(&food_list_mutex);

        if (hitWall(ant.x + STEP_SIZE * cos(ant.direction), ant.y + STEP_SIZE * sin(ant.direction)))
        {
            int d = ((rand() % 2) * 2 - 1);         // get random value between -1 for CW and 1 CCW
            ant.direction += (45 * M_PI / 180) * d; // turn 45 degrees
            if (ant.direction > (2 * M_PI))
                ant.direction -= (2 * M_PI);
            else if (ant.direction < 0)
                ant.direction += (2 * M_PI);
            while (hitWall(ant.x + STEP_SIZE * cos(ant.direction), ant.y + STEP_SIZE * sin(ant.direction)))
            {
                ant.direction += (45 * M_PI / 180) * d; // turn 45 degrees
                usleep(100000 / speed);                 // for production change to 1000000 / speed
                cout << "WEEEEEE" << endl;
            }
        }

        if (ant.direction >= (2 * M_PI))
            ant.direction -= (2 * M_PI);
        else if (ant.direction < 0)
            ant.direction += (2 * M_PI);

        ant.x += STEP_SIZE * cos(ant.direction);
        ant.y += STEP_SIZE * sin(ant.direction);
        // pthread_mutex_lock(&ants_list_mutex);
        ants[index].x = ant.x;
        ants[index].y = ant.y;
        ants[index].direction = ant.direction;
        ants[index].pheromone = ant.pheromone;
        ants[index].foodX = ant.foodX;
        ants[index].foodY = ant.foodY;
        // pthread_mutex_unlock(&ants_list_mutex);
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
            pthread_mutex_lock(&food_list_mutex);
            foodPieces.push_back(food);
            pthread_mutex_unlock(&food_list_mutex);
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
    printf("ANT_SMELL_DISTANCE: %f\n", ANT_SMELL_DISTANCE);
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

    int opglId = pthread_create(&foodThread, NULL, opengl, 0);

    for (int i = 0; i < NUMBER_OF_ANTS; i++)
        pthread_join(antsThread[i], NULL);

    pthread_join(foodThread, NULL);

    // for(int i=0; i<foodPieces.size(); i++){
    //	 cout<< foodPieces[i].x << "\t" << foodPieces[i].y << "\t" << foodPieces[i].numOfPortions<< endl;
    // }

    // pthread_mutex_lock(&foodPieces[1].portions_mutex);
    // foodPieces[1].numOfPortions--;
    // pthread_mutex_unlock(&foodPieces[1].portions_mutex);

    return 0;
}

// TODO: make area user definable
bool hitWall(double x, double y)
{
    if (x <= -2 || x >= 2 || y <= -1 || y >= 1)
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
            NUMBER_OF_ANTS = stoi(value);
        }
        else if (variableName == "FOOD_DWELL_TIME")
        {
            FOOD_DWELL_TIME = stoi(value);
        }
        else if (variableName == "ANT_SMELL_DISTANCE")
        {
            ANT_SMELL_DISTANCE = stof(value); // TODO revisit
        }
        else if (variableName == "STRONG_PHEROMONE_THRESHOLD")
        {
            STRONG_PHEROMONE_THRESHOLD = stoi(value);
        }
        else if (variableName == "WEAK_PHEROMONE_THRESHOLD")
        {
            WEAK_PHEROMONE_THRESHOLD = stoi(value);
        }
        else if (variableName == "MAX_PHEROMONE_AMOUNT")
        {
            MAX_PHEROMONE_AMOUNT = stof(value); // TODO revisit
        }
        else if (variableName == "ANT_APPETITE")
        {
            ANT_APPETITE = stoi(value);
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

    glColor3f(1.0f, 0.0f, 0.0f);
    drawRectangle(0, 0, 10, 10, true);
    glColor3f(0.0f, 1.0f, 0.0f);
    drawRectangle(0, 0, 10, 2, true);

    renderText(TIME, 0, 1.12, 24);

    // Render the text at position (0, 0)
    // renderText("Hello, World!", 0, 0, 24);

    // drawCircle(0.05, 0.5, 0.5);
    // drawAnt();
    for (int i = 0; i < NUMBER_OF_ANTS; i++)
    {
        //drawCircle(0.01, ants[i].x, ants[i].y);
        // cout<<ants[i].direction<<"\t"<<(ants[i].direction *180.0 / M_PI) + 90<<endl;
        drawAnt(ants[i].x, ants[i].y, (ants[i].direction * 180.0 / M_PI) + 270);
    }
    glColor3f(1.0f, 1.0f, 1.0f);
    for (int i = 0; i < foodPieces.size(); i++)
    {
        drawRectangle(foodPieces[i].x, foodPieces[i].y, 0.03, 0.03, false);
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
    glutCreateWindow("FreeType Text Rendering");

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
