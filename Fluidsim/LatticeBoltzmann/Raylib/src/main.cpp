#include <raylib.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <stdio.h> 
#include <random>
#include <chrono>
#include <raymath.h>

using namespace std;
using namespace std::chrono;


void CustomTraceLog(int msgType, const char *text, va_list args)
{
    return;
    char timeStr[64] = { 0 };
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] ", timeStr);

    switch (msgType)
    {
        case LOG_INFO: printf("[INFO] : "); break;
        case LOG_ERROR: printf("[ERROR]: "); break;
        case LOG_WARNING: printf("[WARN] : "); break;
        case LOG_DEBUG: printf("[DEBUG]: "); break;
        default: break;
    }

    vprintf(text, args);
    printf("\n");
}

float dot_product(float arr1[], float arr2[], int dim)
{
	float sum = 0;
	for (int i = 0; i < dim; i++) {
		sum += arr1[i] * arr2[i];
	}
	return sum;
}

float magnitude(float x, float y) {
    return sqrt(x*x+y*y);
}


constexpr int screenWidth = 1200;
constexpr int screenHeight = 900;

const float dt = 1.0 / 60;
const int width = 150; //num of cells
const int heigth = 100; //num of cells
const int cellSize = 1 * screenWidth/width;

//float c = gridsize / dt;
float c = 1;

float tau = 0.6f;

int dims = 9;
constexpr float inletVelocity = 0.1f;
constexpr float rho0 = 1.0f;

constexpr float velocityDrawScale = 10.0f;

float n_arr[width][heigth][9];	//2D array in which cells are densities of the 9 vectors to neighbor cells.
float n_temp[width][heigth][9];


float w_arr[9] = {4.0/9.0f, 1.0/9.0f, 1.0/36.0f, 1.0/9.0f, 1.0/36.0f, 1.0/9.0f, 1.0/36.0f, 1.0/9.0f, 1.0/36.0f};	//Boltzmann Distribution weights for the neighbor vectors.

float e_arr[9][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};


float rho_arr[width][heigth];
float u_arr[width][heigth][2];

bool wall_arr[width][heigth];

int timings[10];

void spawnCylinder()
{
    int middle_x = width / 4;
    int middle_y = heigth / 2;
    int radius = 16;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < heigth; y++) {
            float midpoint_sq = pow(x - middle_x + 0.5, 2) + pow(y - middle_y + 0.5, 2);
            if (midpoint_sq <= pow(radius, 2))
            {
                wall_arr[x][y] = true;
            }
            
        }
    }
    
}


void init()
{
    spawnCylinder(); 
    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);
    normal_distribution<float> distribution(0.0, 1.0);

    const float ux = 0.15f;
    const float uy = 0.0f;
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < heigth; y++) {
            if (wall_arr[x][y])
                continue;
            for (int i = 0; i < dims; i++){
                //n_arr[x][y][i] = w_arr[i] * (1.0 + 0.01 * distribution(generator));
                

                const float eDotU = e_arr[i][0] * ux * (1 + 0.05 * distribution(generator)) + e_arr[i][1] * uy;
                const float u2 = ux * ux + uy * uy;
                n_arr[x][y][i] = rho0 * w_arr[i] * (1.0f + 3.0f * eDotU + 4.5f * eDotU * eDotU - 1.5f * u2);
            }
            rho_arr[x][y] = 1;

            //wall_arr[x][y] = false;

            if (y == 0 || y == heigth - 1)
            {
                //wall_arr[x][y] = true;
            }
            if (x == 5 && y == 5)
            {
                //wall_arr[x][y] = true;
            }
        }
    }   
    
}

void applyBoundaryConditions()
{
    for (int y = 0; y < heigth; y++) {
        if (!wall_arr[0][y]) {
            const float ux = inletVelocity;
            const float uy = 0.0f;
            const float rho = rho_arr[1][y] > 0.0f ? rho_arr[1][y] : rho0;
            for (int i = 0; i < dims; i++) {
                const float eDotU = e_arr[i][0] * ux + e_arr[i][1] * uy;
                const float u2 = ux * ux + uy * uy;
                n_arr[0][y][i] = rho * w_arr[i] * (1.0f + 3.0f * eDotU + 4.5f * eDotU * eDotU - 1.5f * u2);
            }
        }

        
        if (!wall_arr[width - 1][y]) {
            const int prev = width - 2;
            const float rho = rho0;
            const float ux = u_arr[prev][y][0];
            const float uy = u_arr[prev][y][1];
            for (int i = 0; i < dims; i++) {
                const float eDotU = e_arr[i][0] * ux + e_arr[i][1] * uy;
                const float u2 = ux * ux + uy * uy;
                n_arr[width - 1][y][i] = rho * w_arr[i] * (1.0f + 3.0f * eDotU + 4.5f * eDotU * eDotU - 1.5f * u2);
            }
        }
        
    }
}


void computeMacroskopic()
{
    auto start = high_resolution_clock::now();

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < heigth; y++) {
            
            rho_arr[x][y] = 0;
            u_arr[x][y][0] = 0;
            u_arr[x][y][1] = 0;

            if (wall_arr[x][y]) {
                rho_arr[x][y] = 1.0f;
                continue;
            }

            for (int i = 0; i < dims; i++) {    
                rho_arr[x][y] += n_arr[x][y][i];
                u_arr[x][y][0] += e_arr[i][0] * c * n_arr[x][y][i];
                u_arr[x][y][1] += e_arr[i][1] * c * n_arr[x][y][i];
            }

            if (rho_arr[x][y] < 0)
            if (rho_arr[x][y] <= 0)
                cout << "uh oh, rho = 0" << endl;
            
            u_arr[x][y][0] /= rho_arr[x][y];
            u_arr[x][y][1] /= rho_arr[x][y];
        }
    }

    auto stop = high_resolution_clock::now();
    timings[1] = duration_cast<chrono::microseconds>(stop - start).count();
}

void collision()
{   
    auto start = high_resolution_clock::now();
    applyBoundaryConditions();
    computeMacroskopic();
    
    //float n_new[width][heigth][9];
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < heigth; y++) {
            if (wall_arr[x][y]) //Bounceback from Wall, switch the 8 vectors
            {
                float temp = n_arr[x][y][1];
                n_arr[x][y][1] = n_arr[x][y][5];
                n_arr[x][y][5] = temp; 

                temp = n_arr[x][y][2];
                n_arr[x][y][2] = n_arr[x][y][6];
                n_arr[x][y][6] = temp; 

                temp = n_arr[x][y][3];
                n_arr[x][y][3] = n_arr[x][y][7];
                n_arr[x][y][7] = temp; 

                temp = n_arr[x][y][4];
                n_arr[x][y][4] = n_arr[x][y][8];
                n_arr[x][y][8] = temp; 
                continue;
            }
    
            //Collision between Distribution
            float rho = rho_arr[x][y];
            float u[] = {u_arr[x][y][0], u_arr[x][y][1]};
                
            for (int i = 0; i < dims; i++) {
                
                float eDotU = e_arr[i][0] * u[0] + e_arr[i][1] * u[1];
                float temp = 1.0f + 3.0f * eDotU + 4.5f * eDotU * eDotU - 1.5f * (u[0] * u[0] + u[1] * u[1]);
                
                float n_eq = rho * w_arr[i] * temp;
                n_arr[x][y][i] = n_arr[x][y][i] - 1 / tau * (n_arr[x][y][i] - n_eq);
            }
        }
    }

    auto stop = high_resolution_clock::now();
    timings[0] = duration_cast<chrono::microseconds>(stop - start).count();
}

void streaming()
{
    auto start = high_resolution_clock::now();

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < heigth; y++) {
            int x_n = x - 1;
            int x_p = x + 1;
            int y_n = y - 1;
            int y_p = y + 1;
            
            if (x_n < 0)
            {
                x_n = width - 1;
            }
            else if(x_p > width - 1)
            {
                x_p = 0;
            }
            if (y_n < 0)
            {
                y_n = heigth - 1;
            }
            else if(y_p > heigth - 1)
            {
                y_p = 0;
            }

            n_temp[x][y][0] = n_arr[x][y][0];
            n_temp[x_p][y][1] = n_arr[x][y][1];
            n_temp[x_p][y_p][2] = n_arr[x][y][2];
            n_temp[x][y_p][3] = n_arr[x][y][3];
            n_temp[x_n][y_p][4] = n_arr[x][y][4];
            n_temp[x_n][y][5] = n_arr[x][y][5];
            n_temp[x_n][y_n][6] = n_arr[x][y][6];
            n_temp[x][y_n][7] = n_arr[x][y][7];
            n_temp[x_p][y_n][8] = n_arr[x][y][8];

        }
    }

    memcpy(n_arr, n_temp, sizeof(n_arr));
    
    auto stop = high_resolution_clock::now();
    timings[3] = duration_cast<chrono::microseconds>(stop - start).count();
}

float abs_u_arr[width][heigth];
float max_u = 0;
Color pixels[width*heigth];

Texture2D CalculatePixels() {
    auto start = high_resolution_clock::now();
    
    for (int x = 0; x < width; x++) {
		for (int y = 0; y < heigth; y++) {
            abs_u_arr[x][y] = magnitude(u_arr[x][y][0], u_arr[x][y][1]);
            if (abs_u_arr[x][y] > max_u)
            {
                max_u = abs_u_arr[x][y];
            }
        }
    }

    for (int x = 0; x < width; x++) {
		for (int y = 0; y < heigth; y++) {
            pixels[x + width * y] = ColorFromHSV(abs_u_arr[x][y]/max_u*360.0f, 1, 1);
        }
    }
    Image screenImage = { .data = pixels, .width = width, .height = heigth, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture2D texture = LoadTextureFromImage(screenImage);

    auto stop = high_resolution_clock::now();
    timings[4] = duration_cast<chrono::microseconds>(stop - start).count();

    return texture;
}

void DrawDensityAsColor()
{
    float max = 0;
    for (int x = 0; x < width; x++) {
		for (int y = 0; y < heigth; y++) {
            //cout << rho_arr[x][y];
            if (rho_arr[x][y] > max)
            {
                //cout << max << endl;
                max = rho_arr[x][y];
            }
        }
    }
    if (max == 0)
        return;
    
    for (int x = 0; x < width; x++) {
		for (int y = 0; y < heigth; y++) {
            Color color = ColorFromHSV(rho_arr[x][y]/max*360, 1, 1);
            DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, color);
        }
    }
    return;
}

void DrawArrow(float posX, float posY, float length, float rotation = 0.0)
{
    float endPosX = posX + length * cos(rotation);
    float endPosY = posY + length * sin(rotation);
    float headWidth = length / 16;
    float headLength = length / 3;
    //DrawTriangle({100, 100}, {75,150}, {125, 150}, BLACK);
    //DrawTriangle({endPosX - sin(rotation) * 10 , endPosY + cos(rotation) * 10}, {endPosX + cos(rotation)*10, endPosY + sin(rotation)*10},  {endPosX + sin(rotation) * 10, endPosY - cos(rotation) * 10}, BLACK);
    DrawTriangle({(float)(endPosX - sin(rotation) * headWidth - cos(rotation) * headLength), (float)(endPosY + cos(rotation) * headWidth - sin(rotation) * headLength)}, {(float)endPosX, (float)endPosY},  {(float)(endPosX + sin(rotation) * headWidth - cos(rotation) * headLength), (float)(endPosY - cos(rotation) * headWidth - sin(rotation) * headLength)}, BLACK);
    DrawLine(posX, posY, endPosX, endPosY, BLACK);

}

void DrawVelocityFieldVectors()
{
    float abs_u_arr[width][heigth];
    float max = 0;
    for (int x = 0; x < width; x++) {
		for (int y = 0; y < heigth; y++) {
            abs_u_arr[x][y] = sqrt(pow(u_arr[x][y][0], 2) + pow(u_arr[x][y][1], 2));
            if (abs_u_arr[x][y] > max)
                max = abs_u_arr[x][y];
        }
    }
    if (max == 0)
        return;

    for (int x = 0; x < width; x++) {
		for (int y = 0; y < heigth; y++) {
            float rotation = atan2(u_arr[x][y][1], u_arr[x][y][0]);
            //float rotation = 0;
            DrawArrow((x + 0.5) * cellSize, (y + 0.5) * cellSize, abs_u_arr[x][y] * cellSize * velocityDrawScale, rotation);
            //DrawCircle((x+0.5) * cellSize, (y+0.5)* cellSize, cellSize/10, BLACK);
            //DrawLine((x+0.5) * cellSize, (y+0.5)* cellSize, x * cellSize, (y+0.5) *cellSize, BLACK);

        }
    }
    return;
}

void DrawWall()
{
    for (int x = 0; x < width; x++) {
		for (int y = 0; y < heigth; y++) {
            if (wall_arr[x][y] == true)
                DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, BLACK);
        }
    }
}



int main() 
{
    //screen setup
    
    InitWindow(screenWidth, screenHeight, "LBM");
    //SetTargetFPS(60);
    SetTraceLogCallback(CustomTraceLog);
    
    //cout << dot_product(w_arr, w_arr, sizeof(w_arr)/sizeof(*w_arr)) << endl;
    //cout <<  << endl;
    init();
    
    //collision();
    //streaming();
    //cout << u_arr[19][9][1];

    //cout << n_arr[0][0][0] << endl;
    //update

    int i = 0;
    while (!WindowShouldClose())
    {        
        //rotation += PI /180;
        for (int x = 0; x < 1; x++) {
            streaming();
            collision();
        }
        
        Texture2D texture = CalculatePixels();


        BeginDrawing();
            ClearBackground(WHITE);
            //DrawDensityAsColor();
            
            DrawTexturePro(
                texture,
                (Rectangle){ 0, 0, (float)texture.width, (float)texture.height }, 
                (Rectangle){ 0, 0, (float)texture.width*cellSize, (float)texture.height*cellSize },
                (Vector2) { 0, 0 }, 0, WHITE);

            //DrawVelocityFieldVectors();
            DrawWall();

            DrawFPS(10, screenHeight - 30);
        EndDrawing();
            
        
        Vector2 mouse_pos = GetMousePosition();
        Vector2 mouse_index = {floor(mouse_pos.x / cellSize), floor(mouse_pos.y / cellSize)};
        if (mouse_index.x < width && mouse_index.y < heigth)
        {
            if (IsMouseButtonDown(MOUSE_BUTTON_LEFT))
            {
                wall_arr[int(mouse_index.x)][int(mouse_index.y)] = true;
            }
            if (IsMouseButtonDown(MOUSE_BUTTON_RIGHT) && wall_arr[int(mouse_index.x)][int(mouse_index.y)] == true)
            {
                wall_arr[int(mouse_index.x)][int(mouse_index.y)] = false;
            }
        }
        
        printf("\rTimings: collision(%d), makros(%d), streaming(%d), drawSpeed(%d)       ", timings[0], timings[1], timings[3], timings[4]);
        fflush(stdout);
        
        //DrawCircle(100, 100, 40, RED);
        
        i++;
    }
    CloseWindow();
}





