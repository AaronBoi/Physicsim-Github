#include <raylib.h>
#include <imgui.h>
#include <rlImGui.h>
#include <rlgl.h>
//#include "external/imgui.h"
//#include "external/rlImGui.h"
//#include "external/rlgl.h"
#include <rlgl.h>
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
const int width = 128; //num of cells
const int height = 64; //num of cells
const int cellSize = 1 * screenWidth/width;

//float c = gridsize / dt;
float c = 1;

float tau = 0.6f;

int dims = 9;
constexpr float inletVelocity = 0.1f;
constexpr float rho0 = 2.7f;

constexpr float velocityDrawScale = 10.0f;

float n_arr[width][height][9];	//2D array in which cells are densities of the 9 vectors to neighbor cells.
float n_temp[width][height][9];


float w_arr[9] = {4.0/9.0f, 1.0/9.0f, 1.0/36.0f, 1.0/9.0f, 1.0/36.0f, 1.0/9.0f, 1.0/36.0f, 1.0/9.0f, 1.0/36.0f};	//Boltzmann Distribution weights for the neighbor vectors.
float e_arr[9][2] = {{0, 0}, {1, 0}, {1, 1}, {0, 1}, {-1, 1}, {-1, 0}, {-1, -1}, {0, -1}, {1, -1}};


float rho_arr[width][height];
float u_arr[width][height][2];

bool wall_arr[width][height];

int timings[10];


//For collision on GPU
uint32_t compute_shader_id;
uint32_t position_buffer;
uint32_t velocity_buffer;


//general variables for GUI interaction
bool running = true;
bool reset = true;
bool periodic_border = true;
bool walls_with_mouse = false;
bool walls_top_bottom = false; 
float cylinder_radius = 4;
bool gravity = false;
bool collision_GPU = true;

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
                wall_arr[x][y] = true;
            }
            
        }
    }
    
}


void init()
{
    memset(n_arr, 0, sizeof(n_arr));
    memset(wall_arr, 0, sizeof(wall_arr));
    spawnCylinder(); 


    unsigned seed = chrono::system_clock::now().time_since_epoch().count();
    default_random_engine generator(seed);
    normal_distribution<float> distribution(0.0, 1.0);

    const float ux = 0.15f;
    const float uy = 0.0f;

    

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {

            if (walls_top_bottom && (y == 0 || y == height - 1))
            {
                wall_arr[x][y] = true;
            }
            if (x == 5 && y == 5)
            {
                //wall_arr[x][y] = true;
            }

            if (wall_arr[x][y])
                continue;

            for (int i = 0; i < dims; i++){
                //n_arr[x][y][i] = w_arr[i] * (1.0 + 0.01 * distribution(generator));
                

                const float eDotU = e_arr[i][0] * ux * (1 + 0.05 * distribution(generator)) + e_arr[i][1] * uy;
                const float u2 = ux * ux + uy * uy;
                n_arr[x][y][i] = rho0 * w_arr[i] * (1.0f + 3.0f * eDotU + 4.5f * eDotU * eDotU - 1.5f * u2);
            }
            rho_arr[x][y] = 1;

        }
    }  
    reset = false;
}




void applyBoundaryConditions()
{
    for (int y = 0; y < height; y++) {
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

    memset(u_arr, 0, sizeof(u_arr));
    memset(rho_arr, 0, sizeof(rho_arr));

    for (int x = 0; x < width; x++) {
        for (int y = 0; y < height; y++) {

            if (wall_arr[x][y]) {
                //rho_arr[x][y] = 1.0f;
                continue;
            }

            for (int i = 0; i < dims; i++) {    
                rho_arr[x][y] += n_arr[x][y][i];
                u_arr[x][y][0] += e_arr[i][0] * n_arr[x][y][i];
                u_arr[x][y][1] += e_arr[i][1] * n_arr[x][y][i];
            }
            
            
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
    //applyBoundaryConditions();
    computeMacroskopic();
    

    //float n_new[width][height][9];
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
            //u[1] += 0.00001;

            for (int i = 0; i < dims; i++) {
                
                //compute equilibrium distribution
                float eDotU = e_arr[i][0] * u[0] + e_arr[i][1] * u[1];
                float temp = 1.0f + 3.0f * eDotU + 4.5f * eDotU * eDotU - 1.5f * (u[0] * u[0] + u[1] * u[1]);
                float n_eq = rho * w_arr[i] * temp;

                //relax distribution toward equilibrium using:

                //Bhatnagar-Gross-Krook model
                n_arr[x][y][i] = n_arr[x][y][i] - 1 / tau * (n_arr[x][y][i] - n_eq);

                //

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

            if (periodic_border)
            {
                int x_n = x - 1;
                int x_p = x + 1;
                int y_n = y - 1;
                int y_p = y + 1;

                if (x_n < 0)
                    x_n = width - 1;

                else if(x_p > width - 1)
                    x_p = 0;

                if (y_n < 0)
                    y_n = height - 1;

                else if(y_p > height - 1)
                    y_p = 0;

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

            else
            {
                
                for (int i = 0; i < dims; i++)
                {
                    int nextX = x + e_arr[i][0];
                    int nextY = y + e_arr[i][1];

                    if (nextX < 0 || nextX >= width || nextY < 0 || nextY >= height) {
                        //nextX = x;
                        //nextY = y;
                        continue;
                    }
                    n_temp[nextX][nextY][i] = n_arr[x][y][i];
                    
                }
                

                /*
                if (x_n < 0)
                    x_n = x;

                else if(x_p > width - 1)
                    x_p = x;

                if (y_n < 0)
                    y_n = y;

                else if(y_p > height - 1)
                    y_p = y;
                */
            }
            
        }
    }

    memcpy(n_arr, n_temp, sizeof(n_arr));
    
    auto stop = high_resolution_clock::now();
    timings[3] = duration_cast<chrono::microseconds>(stop - start).count();
}

float abs_u_arr[width][height];
float max_u = 0;
Color pixels[width*height];

Texture2D CalculatePixels() {
    auto start = high_resolution_clock::now();
    
    for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
            abs_u_arr[x][y] = magnitude(u_arr[x][y][0], u_arr[x][y][1]);
            if (abs_u_arr[x][y] > max_u)
            {
                max_u = abs_u_arr[x][y];
            }
        }
    }

    for (int x = 0; x < width; x++) {
		for (int y = 0; y < height; y++) {
            pixels[x + width * y] = ColorFromHSV(abs_u_arr[x][y]/max_u*360.0f, 1, 1);
        }
    }
    Image screenImage = { .data = pixels, .width = width, .height = height, .mipmaps = 1, .format = PIXELFORMAT_UNCOMPRESSED_R8G8B8A8};
    Texture2D texture = LoadTextureFromImage(screenImage);

    auto stop = high_resolution_clock::now();
    timings[4] = duration_cast<chrono::microseconds>(stop - start).count();

    return texture;
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
            float rotation = -atan2(u_arr[x][y][1], u_arr[x][y][0]);
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
    
    int x = rlGetVersion();
    int numParticles = 10;
    Vector4 *positions = new Vector4[numParticles];
    
    for (int i = 0; i < numParticles; i++) {
        // We only use the XYZ components of position and velocity.
        // Use the remainder for extra effects if needed, or create more buffers.
        positions[i] = (Vector4){ 
            1.0,
            2.0,
            3.0,
            4.0,
        }; 
    }

    InitWindow(screenWidth, screenHeight, "LBM");
    SetTargetFPS(15);
    SetTraceLogCallback(CustomTraceLog);
    
    rlImGuiSetup(true);
    init();
    // Compute shader for updating particles.
    //char *shaderCode = LoadFileText("D:/Programming/Physicsim Github/Fluidsim/LatticeBoltzmann/Raylib/Shaders/particle_compute.glsl");
    //SaveFileText("test.txt", "gibberisch \0");

    
    char *shaderCode = LoadFileText("Shaders/particle_compute.glsl");
    int shaderData = rlCompileShader(shaderCode, RL_COMPUTE_SHADER);
    int computeShader = rlLoadComputeShaderProgram(shaderData);

    //int computeShader = rlLoadShader(shaderCode, RL_COMPUTE_SHADER);
    //int computeShader = rlLoadComputeShaderProgram(shaderData);
    

    UnloadFileText(shaderCode);

    cout << shaderCode << endl;
    
    //int numParticles = 1024*1;
    //float *positions = RL_MALLOC(sizeof(float)*numParticles);
    
    int ssbo0 = rlLoadShaderBuffer(numParticles*sizeof(Vector4), positions, RL_DYNAMIC_COPY);

    //int ssbo0 = rlLoadShaderBuffer(sizeof(n_arr), n_arr, RL_DYNAMIC_COPY);
    //int ssbo1 = rlLoadShaderBuffer(sizeof(wall_arr), wall_arr, RL_DYNAMIC_COPY);
    //int ssbo2 = rlLoadShaderBuffer(sizeof(rho_arr), rho_arr, RL_DYNAMIC_COPY);
    //int ssbo3 = rlLoadShaderBuffer(sizeof(u_arr), u_arr, RL_DYNAMIC_COPY);

    //int ssbo2 = rlLoadShaderBuffer(sizeof(test), test, RL_DYNAMIC_COPY);
    //int ssbo4 = rlLoadShaderBuffer(sizeof(rho_arr), rho_arr, RL_DYNAMIC_COPY);


    int i = 0;
    while (!WindowShouldClose())
    {        
        if (reset) init();
        //rotation += PI /180;

        rho_arr[0][0] = 1;
        for (int x = 0; x < 1; x++) {
            if (!running)
                break;


            if (collision_GPU)
            {
                //computeMacroskopic();
            
                //rlUpdateShaderBuffer(ssbo2, &rho_arr, sizeof(rho_arr), 0);
                rlEnableShader(computeShader);

                    //rlSetUniform(0, &width, SHADER_UNIFORM_INT, 1);
                    //rlSetUniform(1, &height, SHADER_UNIFORM_INT, 1);

                    rlBindShaderBuffer(ssbo0, 0);
                    //rlBindShaderBuffer(ssbo1, 1);
                    //rlBindShaderBuffer(ssbo2, 2);
                    //rlBindShaderBuffer(ssbo3, 3);
                    
                    //rlComputeShaderDispatch(ceil(width / 16.0), ceil(height / 16), 1);
                    rlComputeShaderDispatch(numParticles, 1, 1);
                    rlReadShaderBuffer(ssbo0, positions, numParticles, 0);
                    
                rlDisableShader();
               
            }
            else
            {
                //collision();
            }
            //streaming();
            cout << positions[0].x << endl;
            //cout << ssbo2 << endl;
            
            
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





