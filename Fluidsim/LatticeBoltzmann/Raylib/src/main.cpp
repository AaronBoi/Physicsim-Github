#include <raylib.h>
#include <imgui.h>
#include <rlImGui.h>
#include <rlgl.h>
#include <cmath>
#include <iostream>
#include <vector>
#include <stdio.h> 
#include <random>
#include <chrono>
#include <raymath.h>
#include <string.h>

using namespace std;
using namespace std::chrono;


void CustomTraceLog(int msgType, const char *text, va_list args)
{
    char timeStr[64] = { 0 };
    time_t now = time(NULL);
    struct tm *tm_info = localtime(&now);

    strftime(timeStr, sizeof(timeStr), "%Y-%m-%d %H:%M:%S", tm_info);
    printf("[%s] ", timeStr);

    switch (msgType)
    {
        case LOG_INFO:
            if (string(text).rfind("TEXT", 0) == 0)
                return;
            
             
            printf("[INFO] : "); 
            break;
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
const int width = 250; //num of cells
const int height = 125; //num of cells
const float cellSize = 1.0 * float(screenWidth/width);

//float c = gridsize / dt;
float c = 1;

float tau = 0.6f; //Standard 0.6

int dims = 9;
constexpr float inletVelocity = 0.05f;
constexpr float rho0 = 2.7f;

constexpr float velocityDrawScale = 10.0f;

float n_arr[width][height][9];	//2D array in which cells are densities of the 9 vectors to neighbor cells.
float n_temp[width][height][9];


float w_arr[9] = {4.0/9.0f, 1.0/9.0f, 1.0/36.0f, 1.0/9.0f, 1.0/36.0f, 1.0/9.0f, 1.0/36.0f, 1.0/9.0f, 1.0/36.0f};	//Boltzmann Distribution weights for the neighbor vectors.
float e_arr[9][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};


float rho_arr[width][height];
float u_arr[width][height][2];

int wall_arr[width][height];

int timings[10];

float test = 0;

Vector2 obj_pos;
float obj_radius;


//For collision on GPU
uint32_t compute_shader_id;
uint32_t position_buffer;
uint32_t velocity_buffer;


//general variables for GUI interaction
bool running = true;
bool reset = true;
int periodic_border = false;
bool walls_with_mouse = false;
bool walls_top_bottom = true; 
float cylinder_radius = height / 4;
bool gravity = false;
bool collision_GPU = false;

void spawnCylinder()
{
    int middle_x = width / 4;
    int middle_y = height / 2;
    float radius = cylinder_radius;

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            float midpoint_sq = pow(x - middle_x + 0.5, 2) + pow(y - middle_y + 0.5, 2);
            if (midpoint_sq <= pow(radius, 2))
            {
                wall_arr[x][y] = 1;
            }
            
        }
    }
    
}

void init()
{
    memset(n_arr, 1.0, sizeof(n_arr));
    memset(wall_arr, 0.0, sizeof(wall_arr));
    memset(rho_arr, 1.0, sizeof(rho_arr));
    memset(u_arr, 0.0, sizeof(u_arr));
    spawnCylinder(); 

    obj_pos = {5.0, height / 5};
    obj_radius = 1;

    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    //unsigned seed = 10;
    default_random_engine generator(seed);
    normal_distribution<float> distribution(0.0, 1.0);

    const float ux = inletVelocity;
    const float uy = 0.0f;

    

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {

            if (walls_top_bottom && (y == 0 || y == height - 1))
            {
                wall_arr[x][y] = 1;
            }
            if (x == 5 && y == 5)
            {
                //wall_arr[x][y] = true;
            }

            if (wall_arr[x][y])
                continue;

            for (int i = 0; i < dims; i++){
                //n_arr[x][y][i] = w_arr[i] * (1.0 + 0.01 * distribution(generator));
                

                const float eDotU = e_arr[i][0] * ux * (1.0 + 0.02 * distribution(generator)) + e_arr[i][0] * uy;
                const float u2 = ux * ux + uy * uy;
                n_arr[x][y][i] = rho0 * w_arr[i] * (1.0f + 3.0f * eDotU + 4.5f * eDotU * eDotU - 1.5f * u2);
            }

        }
    }  
    reset = false;
}

void applyOpenBoundary()
{
    if (periodic_border == 1) return;

    // left/right edges: copy from one cell inward
    for (int y = 0; y < height; y++) {
        for (int i = 0; i < dims; i++) {
            n_arr[0][y][i]         = n_arr[1][y][i];
            n_arr[width-1][y][i]   = n_arr[width-2][y][i];
        }
    }
    // top/bottom edges
    for (int x = 0; x < width; x++) {
        for (int i = 0; i < dims; i++) {
            n_arr[x][0][i]          = n_arr[x][1][i];
            n_arr[x][height-1][i]   = n_arr[x][height-2][i];
        }
    }
}

void ZouHe_Velocity_Boundary(float u_x = inletVelocity, float u_y = 0.0f)
{
    if (periodic_border) return;

    //Constant velocity at the left and right grid boundaries.
    float n_left[9];
    float n_right[9];
    for (int y = 0; y < height; y++) {

        if (y == 0 || y == height - 1)
        {
            //continue;
        }
        

        memcpy(n_left, n_arr[0][y], sizeof(n_left));
        memcpy(n_right, n_arr[width - 1][y], sizeof(n_right));

        //left side
        float rho = 1.0 / (1.0 - u_x) * (2 * (n_left[4] + n_left[5] + n_left[6]) + n_left[0] + n_left[3] + n_left[7]);
    
        n_arr[0][y][1] = n_left[5] + 2.0/3.0 * rho * u_x;
        n_arr[0][y][2] = 1.0/6.0 * rho * u_x + 0.5 * rho * u_y + n_left[6] + 0.5 * (n_left[7] - n_left[3]);
        n_arr[0][y][8] = 1.0/6.0 * rho * u_x - 0.5 * rho * u_y + n_left[4] - 0.5 * (n_left[7] - n_left[3]);

        /*
        if (y == 0) { //corner left bottom
            
            n_arr[0][y][1] = n_arr[0][y][5];
            n_arr[0][y][3] = n_arr[0][y][7];
            n_arr[0][y][2] = n_arr[0][y][6];
            n_arr[0][y][4] = 0.5 * (rho - n_left[0] - n_left[1] - n_left[2] - n_left[3] - n_left[5] - n_left[6] - n_left[7]);
            n_arr[0][y][8] = n_arr[0][y][4];
        }

        if (y == height - 1) { //corner left top
            n_arr[0][y][1] = n_arr[0][y][5];
            n_arr[0][y][8] = n_arr[0][y][4];
            n_arr[0][y][7] = n_arr[0][y][3];
            n_arr[0][y][2] = 0.5 * (rho - n_left[0] - n_left[1] - n_left[3] - n_left[4] - n_left[5] - n_left[7] - n_left[8]);
            n_arr[0][y][6] = n_arr[0][y][2];
        }
        */
        //right side

        rho = 1.0 / (1.0 + u_x) * (2 * (n_right[1] + n_right[2] + n_right[8]) + n_right[0] + n_right[3] + n_right[7]);
        n_arr[width - 1][y][5] = n_right[1] - 2.0/3.0 * rho * u_x;
        n_arr[width - 1][y][4] = -1.0/6.0 * rho * u_x + 0.5 * rho * u_y + n_right[8] + 0.5 * (n_right[7] - n_right[3]);
        n_arr[width - 1][y][6] = -1.0/6.0 * rho * u_x - 0.5 * rho * u_y + n_right[2] - 0.5 * (n_right[7] - n_right[3]);

        
        /*
        if (y == 0) {   //corner right bottom
            
            n_arr[0][y][5] = n_arr[0][y][1];
            n_arr[0][y][3] = n_arr[0][y][7];
            n_arr[0][y][4] = n_arr[0][y][8];
            n_arr[0][y][2] = 0.5 * (rho - n_left[0] - n_left[1] - n_left[3] - n_left[4] - n_left[5] - n_left[7] - n_left[8]);
            n_arr[0][y][6] = n_arr[0][y][2];
        }

        if (y == height - 1) { //corner right top
            n_arr[0][y][5] = n_arr[0][y][1];
            n_arr[0][y][6] = n_arr[0][y][2];
            n_arr[0][y][7] = n_arr[0][y][3];
            n_arr[0][y][4] = 0.5 * (rho - n_left[0] - n_left[1] - n_left[2] - n_left[3] - n_left[5] - n_left[6] - n_left[7]);
            n_arr[0][y][8] = n_arr[0][y][4];
        }
        */

    }
}

void ZouHe_Pressure_Boundary(float rho = rho0, float u_y = 0.0)
{
    if (periodic_border) return;

    //Constant velocity at the left and right grid boundaries.
    float n_left[9];
    float n_right[9];
    for (int y = 0; y < height; y++) {

        if (y == 0 || y == height - 1)
        {
            //continue;
        }
        

        memcpy(n_left, n_arr[0][y], sizeof(n_left));
        memcpy(n_right, n_arr[width - 1][y], sizeof(n_right));

        //left side
        float u_x = 1.0 - 1.0 / rho * (2 * (n_left[4] + n_left[5] + n_left[6]) + n_left[0] + n_left[3] + n_left[7]);
        n_arr[0][y][1] = n_left[5] + 2.0/3.0 * rho * u_x;
        n_arr[0][y][2] = 1.0/6.0 * rho * u_x + 0.5 * rho * u_y + n_left[6] + 0.5 * (n_left[7] - n_left[3]);
        n_arr[0][y][8] = 1.0/6.0 * rho * u_x - 0.5 * rho * u_y + n_left[4] - 0.5 * (n_left[7] - n_left[3]);

        //right side

        u_x = -1.0 + 1.0 / rho * (2 * (n_right[1] + n_right[2] + n_right[8]) + n_right[0] + n_right[3] + n_right[7]);
        n_arr[width - 1][y][5] = n_right[1] - 2.0/3.0 * rho * u_x;
        n_arr[width - 1][y][4] = -1.0/6.0 * rho * u_x + 0.5 * rho * u_y + n_right[8] + 0.5 * (n_right[7] - n_right[3]);
        n_arr[width - 1][y][6] = -1.0/6.0 * rho * u_x - 0.5 * rho * u_y + n_right[2] - 0.5 * (n_right[7] - n_right[3]);

    }
}

float abs_u_arr[width][height];
float max_u = 0;
Color pixels[width*height];

Texture2D CalculatePixels() {
    auto start = high_resolution_clock::now();
    
    for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
            abs_u_arr[x][y] = magnitude(u_arr[x][y][0], u_arr[x][y][1]);
            if (wall_arr[x][y] == 0 && abs_u_arr[x][y] > max_u)
            {
                max_u = abs_u_arr[x][y];
            }
        }
    }

    for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
            pixels[x + width * y] = ColorFromHSV(abs_u_arr[x][y]/max_u*360.0f, 1, 1);
            //pixels[x + width * y] = ColorFromHSV(abs_u_arr[x][y] * 360 *20, 1, 1);
        }
    }
    Image screenImage = { .data = pixels, .width = width, .height = height, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture2D texture = LoadTextureFromImage(screenImage);

    auto stop = high_resolution_clock::now();
    timings[4] = duration_cast<chrono::microseconds>(stop - start).count();

    return texture;
}

void computeMacroskopic()
{
    auto start = high_resolution_clock::now();

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
            
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
    computeMacroskopic();
    
    //float n_new[width][heigth][9];
    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {
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
        for (int y = 0; y < height; y++) {
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
                y_n = height - 1;
            }
            else if(y_p > height - 1)
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

void DrawDensityAsColor()
{
    float max = 0;
    for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
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
		for (int y = 0; y < height; y++) {
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
    float abs_u_arr[width][height];
    float max = 0;
    for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
            abs_u_arr[x][y] = sqrt(pow(u_arr[x][y][0], 2) + pow(u_arr[x][y][1], 2));
            if (abs_u_arr[x][y] > max)
                max = abs_u_arr[x][y];
        }
    }
    if (max == 0)
        return;

    for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
            float rotation = atan2(u_arr[x][y][1], u_arr[x][y][0]);
            //float rotation = 0;
            DrawArrow((x + 0.5) * cellSize, (y + 0.5) * cellSize, abs_u_arr[x][y] * cellSize/max * velocityDrawScale, rotation);
            //DrawCircle((x+0.5) * cellSize, (y+0.5)* cellSize, cellSize/10, BLACK);
            //DrawLine((x+0.5) * cellSize, (y+0.5)* cellSize, x * cellSize, (y+0.5) *cellSize, BLACK);

        }
    }
    return;
}

void DrawWall()
{
    for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
            if (wall_arr[x][y] == true)
                DrawRectangle(x * cellSize, y * cellSize, cellSize, cellSize, BLACK);
        }
    }
}

void MakeImGui()
{
    rlImGuiBegin();	            
            
    //IMGUI

    ImGui::Begin("Hello, world!");                          // Create a window called "Hello, world!" and append into it.

    ImGui::Text("This is some useful text.");               // Display some text (you can use a format strings too)

    ImGui::SliderFloat("Cylinder radius", &cylinder_radius, 1.0f, 32.0f);            // Edit 1 float using a slider from 0.0f to 1.0f


    if (ImGui::Button("Start"))                           
        running = true;
    if (ImGui::Button("Stop"))                            
        running = false;
    if (ImGui::Button("Reset"))                           
        reset = true;
    if (ImGui::Button("Walls with mouseclick"))   
    {  
        walls_with_mouse = !walls_with_mouse;
    }
    ImGui::SameLine();
    ImGui::Text("%d", walls_with_mouse);

    if (ImGui::Button("Periodic border"))   
    {  
        periodic_border = !periodic_border;
    }
    ImGui::SameLine();
    ImGui::Text("%d", periodic_border);

    if (ImGui::Button("Top and bottom walls on init"))   
    {  
        walls_top_bottom = !walls_top_bottom;
    }
    ImGui::SameLine();
    ImGui::Text("%d", walls_top_bottom);

    ImGui::End();

    rlImGuiEnd();
}

int main() 
{   

    InitWindow(screenWidth, screenHeight, "LBM");
    SetTargetFPS(60);
    SetTraceLogCallback(CustomTraceLog);
    
    rlImGuiSetup(true);
    init();
    
    int ssbo0, ssbo1, ssbo2, ssbo3, ssbo4;
    int computeShader, computeShaderStreaming;

    if (collision_GPU)
    {
        char *shaderCode = LoadFileText("resources/collision.glsl");
        int shaderData = rlLoadShader(shaderCode, RL_COMPUTE_SHADER);
        computeShader = rlLoadShaderProgramCompute(shaderData);
        UnloadFileText(shaderCode);

        shaderCode = LoadFileText("resources/streaming.glsl");
        shaderData = rlLoadShader(shaderCode, RL_COMPUTE_SHADER);
        computeShaderStreaming = rlLoadShaderProgramCompute(shaderData);
        UnloadFileText(shaderCode);

        ssbo0 = rlLoadShaderBuffer(sizeof(n_arr), n_arr, RL_DYNAMIC_COPY);
        ssbo1 = rlLoadShaderBuffer(sizeof(wall_arr), wall_arr, RL_DYNAMIC_COPY);
        ssbo2 = rlLoadShaderBuffer(sizeof(rho_arr), rho_arr, RL_DYNAMIC_COPY);
        ssbo3 = rlLoadShaderBuffer(sizeof(u_arr), u_arr, RL_DYNAMIC_COPY);
        ssbo4 = rlLoadShaderBuffer(sizeof(n_temp), n_temp, RL_DYNAMIC_COPY);
    }
    
    int i = 0;
    while (!WindowShouldClose())
    {        
        if (reset) init();
        
        for (int x = 0; x < 10; x++) {
            if (!running) break;
            
            if (collision_GPU)
            {
               
                rlUpdateShaderBuffer(ssbo0, &n_arr, sizeof(n_arr), 0);
                rlUpdateShaderBuffer(ssbo1, &wall_arr, sizeof(wall_arr), 0);
                
                //rlUpdateShaderBuffer(ssbo2, &rho_arr, sizeof(rho_arr), 0);
                //rlUpdateShaderBuffer(ssbo3, &u_arr, sizeof(u_arr), 0);

                memcpy(n_temp, n_arr, sizeof(n_temp));

                rlEnableShader(computeShader);
            
                    rlSetUniform(0, &width, SHADER_UNIFORM_INT, 1);
                    rlSetUniform(1, &height, SHADER_UNIFORM_INT, 1);
                    rlSetUniform(2, &tau, SHADER_UNIFORM_FLOAT, 1);
                    rlSetUniform(3, &periodic_border, SHADER_UNIFORM_INT, 1);

                    rlBindShaderBuffer(ssbo0, 0);
                    rlBindShaderBuffer(ssbo1, 1);
                    rlBindShaderBuffer(ssbo2, 2);
                    rlBindShaderBuffer(ssbo3, 3);
                    rlBindShaderBuffer(ssbo4, 4);
                    
                    rlComputeShaderDispatch(ceil(width / 32.0), ceil(height / 32.0), 1);
                    //rlComputeShaderDispatch(width, height, 1);

                    rlReadShaderBuffer(ssbo0, n_arr, sizeof(n_arr), 0);
                    rlReadShaderBuffer(ssbo2, rho_arr, sizeof(rho_arr), 0);
                    rlReadShaderBuffer(ssbo3, u_arr, sizeof(u_arr), 0);
                    rlReadShaderBuffer(ssbo4, n_temp, sizeof(n_temp), 0);
                    
                    
                rlDisableShader();
                

                memset(n_temp, 0, sizeof(n_temp)); //not good if periodic boundary is false
                rlEnableShader(computeShaderStreaming);

                    rlSetUniform(0, &width, SHADER_UNIFORM_INT, 1);
                    rlSetUniform(1, &height, SHADER_UNIFORM_INT, 1);
                    rlSetUniform(3, &periodic_border, SHADER_UNIFORM_INT, 1);

                    rlUpdateShaderBuffer(ssbo0, &n_arr, sizeof(n_arr), 0);
                    rlUpdateShaderBuffer(ssbo1, &wall_arr, sizeof(wall_arr), 0);
                    rlUpdateShaderBuffer(ssbo4, &n_temp, sizeof(n_temp), 0);

                    rlBindShaderBuffer(ssbo0, 0);
                    rlBindShaderBuffer(ssbo1, 1);
                    rlBindShaderBuffer(ssbo4, 4);

                    rlComputeShaderDispatch(ceil(width / 32.0), ceil(height / 32.0), 1);

                    rlReadShaderBuffer(ssbo4, n_temp, sizeof(n_temp), 0);

                rlDisableShader();
            }
            else
            {
                collision();
                streaming();
            }
            memcpy(n_arr, n_temp, sizeof(n_arr));
            //applyOpenBoundary();        
            ZouHe_Velocity_Boundary();
            //ZouHe_Pressure_Boundary(rho0);
        }

        int obj_index_x = floor(obj_pos.x);
        int obj_index_y = floor(obj_pos.y);
        obj_pos.x += u_arr[obj_index_x][obj_index_y][0] * 5;
        obj_pos.y += u_arr[obj_index_x][obj_index_y][1] * 5;
        
        
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

            DrawCircle(obj_pos.x * cellSize, obj_pos.y * cellSize, obj_radius * cellSize, BLUE);

            DrawFPS(10, screenHeight - 30);

            MakeImGui();	

        EndDrawing();
        
        

        Vector2 mouse_pos = GetMousePosition();
        Vector2 mouse_index = {floor(mouse_pos.x / cellSize), floor(mouse_pos.y / cellSize)};
        if (mouse_index.x < width && mouse_index.y < height && walls_with_mouse)
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
    rlImGuiShutdown();	

    CloseWindow();
}





