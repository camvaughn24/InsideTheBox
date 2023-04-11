// CS370 Final Project
// Fall 2022

#define STB_IMAGE_IMPLEMENTATION
#include "../common/stb_image.h"	// Sean Barrett's image loader - http://nothings.org/
#include <stdio.h>
#include <vector>
#include "../common/vgl.h"
#include "../common/objloader.h"
#include "../common/utils.h"
#include "../common/vmath.h"
#include "lighting.h"
#include "house.h"
#define DEG2RAD (M_PI/180.0)


using namespace vmath;
using namespace std;

// Vertex array and buffer names
enum VAO_IDs {BackBarnDoor, Barn, BarnDoor, BarnRoof, Bed, Bowl, Can, Chair, Couch, Cone, Cube, Cylinder, Garage, Fan, Fridge,
        FridgeDoor, Glass, Helicopter, HelicopterWindow, Propellors, PictureFrame, Silo, Sphere,
        Stairs, Table, TV, TVStand, Window, BigWindow, WindMill, WindMillBase, Mirror, Frame, LightSaber, VanGogh, Grass, Road, CarpetObj,
    HelicopterPad, GarageDoor, TVScreen, NumVAOs};//frame, mirror
enum ObjBuffer_IDs {PosBuffer, NormBuffer, TexBuffer, NumObjBuffers};
enum Color_Buffer_IDs {Blue, Black, White, NumColorBuffers};
enum LightBuffer_IDs {LightBuffer, NumLightBuffers};
enum LightNames {SunLight, FanLight,  };
enum MaterialBuffer_IDs {MaterialBuffer, NumMaterialBuffers};
enum MaterialNames {Silver, RedPlastic, BlackPlastic, BlackTransparent, Transparent, PurpleTransparent};
enum Textures {Apple, BlackWood, BlueCarpet, DarkBrownWood, GarageDoorTex, GrassTex, GreyWood, HelicopterPadTex, LightBrownWood,
        LightSaberArt, RedWood, RedWool, RoadTex, SiloTex, Soda, Stone, TreeTex, TVRoku, VanGoghArt, WhiteWood,
        MirrorTex, NumTextures};

// Vertex array and buffer objects
GLuint VAOs[NumVAOs];
GLuint ObjBuffers[NumVAOs][NumObjBuffers];
GLuint ColorBuffers[NumColorBuffers];
GLuint LightBuffers[NumLightBuffers];
GLuint MaterialBuffers[NumMaterialBuffers];
GLuint TextureIDs[NumTextures];

// Number of vertices in each object
GLint numVertices[NumVAOs];

// Number of component coordinates
GLint posCoords = 4;
GLint normCoords = 3;
GLint texCoords = 2;
GLint colCoords = 4;

// Model files
vector<const char *> objFiles = {"../models/backbarndoor.obj", "../models/barn.obj", "../models/barndoor.obj", "../models/barnroof.obj",
                                 "../models/bed.obj", "../models/bowl.obj", "../models/can.obj",  "../models/chair.obj",
                                 "../models/couch.obj", "../models/cone.obj", "../models/unitcube.obj",
                                 "../models/cylinder.obj","../models/opengarage.obj",
                                 "../models/fan.obj", "../models/fridge.obj", "../models/fridgedoor.obj", "../models/glass.obj",
                                 "../models/helicopter.obj", "../models/frontwindow.obj", "../models/propellors.obj",
                                 "../models/pictureframe.obj", "../models/silo.obj",
                                 "../models/sphere.obj", "../models/stairs.obj", "../models/table.obj",
                                 "../models/tv.obj", "../models/tvstand.obj", "../models/window.obj", "../models/bigwindow.obj",
                                 "../models/windmill.obj", "../models/windmillbase.obj", "../models/plane.obj"}; // mirror.obj

// Texture files
vector<const char *> texFiles = {"../textures/apple.png", "../textures/blackwood.png","../textures/bluecarpet.png","../textures/darkwood.png",
                                 "../textures/garagedoor.png", "../textures/grass.png", "../textures/greywood.png",
                                 "../textures/helicopterpad.png", "../textures/lightbrownwood.png", "../textures/lightsabers.png",
                                "../textures/redwood.png", "../textures/redwool.png",  "../textures/road.png", "../textures/silotex.png",
                                 "../textures/cokezero.png","../textures/stone.png",
                                 "../textures/tree.png", "../textures/roku.png",  "../textures/art.png",
                                 "../textures/whitewood.png"

        };

// Camera
vec3 eye_ortho = {0.0f, 5.0, 10.0f};
vec3 center_ortho = {0.0f, 0.0f, 0.0f};
vec3 up = {0.0f, 1.0f, 0.0f};
vec3 eye = {0.0f, 3.0f, 10.0f};
vec3 center = {0.0f, 3.0f, 0.0f};
vec3 mirror_eye = {2.5f, 2.0f, -1.9f};
vec3 mirror_center = {2.0f, 2.0f, 2.0f};

GLfloat azimuth = -90.0f;
GLfloat daz = 5.0f;
GLfloat elevation = 90.0f;
GLfloat del = 2.0f;
GLfloat radius = 2.0f;
GLfloat dr = 0.1f;
GLfloat min_radius = 2.0f;
GLdouble elTime = 0.0;
GLfloat step = 0.15;


// animations
    // booleans
    GLboolean fanspin = false;
    GLboolean open_windows = false;
    GLboolean fridge_open = false;
    GLboolean open_doors = false;
    GLboolean tv_on = false;
    GLboolean barn_transparent = false;
    GLboolean helicopter_fly = false;
    GLboolean free_movement = false;

    // variables
    vec3 fridgedoor_pos = {-2.0f, .5f, 9.95f};
    ;
    GLfloat fridge_max = fridgedoor_pos[2];
    GLfloat fridge_min = fridgedoor_pos[2] - 1.20f;
    GLfloat doors_step = .01f;

    vec3 frontrightdoor_pos{1.65f, 1.25f, 12.5f}; vec3 backrightdoor_pos{1.65f, 1.25f, 12.375f};
    vec3 frontleftdoor_pos{-.25f, 1.25f, 12.5f}; vec3 backleftdoor_pos{-.25f, 1.25f, 12.375f};
    GLfloat barn_min = frontrightdoor_pos[0];
    GLfloat barn_max = frontrightdoor_pos[0] + 2.0f;

    GLfloat windows_posy = 2.3f;
    GLfloat window_min = 2.3f;
    GLfloat window_max = 3.5f;

    vec3 big_windowpos = {0.0f, 1.3f, 1.5f};
    vec3 small_windowpos = {0.75f, 8.0f, 12.5f};

    vec3 helicopter_pos{-17.0f, 2.0f, 8.0f};
    GLfloat fly_value = 3.0f;
    GLfloat fly_min = 2.0f;
    GLfloat fly_max = 10.0f;
    GLfloat helicopter_dir = 1.0f;

    GLfloat propellor_ang = 0.0f;

    vec3 windmill_pos = {-10.0f, 0.0f, -12.0f};

    GLfloat fan_ang = 0.0f;
    GLfloat fan_pos = 0.0f;



// View modes
#define ORTHOGRAPHIC 0
#define PERSPECTIVE_FIRST 1
#define PERSPECTIVE_SECOND 2
int proj = PERSPECTIVE_FIRST;

// Shader variables
// Default (color) shader program references
GLuint default_program;
GLuint default_vPos;
GLuint default_vCol;
GLuint default_proj_mat_loc;
GLuint default_cam_mat_loc;
GLuint default_model_mat_loc;
const char *default_vertex_shader = "../default.vert";
const char *default_frag_shader = "../default.frag";

// Lighting shader program reference
GLuint lighting_program;
GLuint lighting_vPos;
GLuint lighting_vNorm;
GLuint lighting_camera_mat_loc;
GLuint lighting_model_mat_loc;
GLuint lighting_proj_mat_loc;
GLuint lighting_norm_mat_loc;
GLuint lighting_lights_block_idx;
GLuint lighting_materials_block_idx;
GLuint lighting_material_loc;
GLuint lighting_num_lights_loc;
GLuint lighting_light_on_loc;
GLuint lighting_eye_loc;
const char *lighting_vertex_shader = "../lighting.vert";
const char *lighting_frag_shader = "../lighting.frag";

// Texture shader program reference
GLuint texture_program;
GLuint texture_vPos;
GLuint texture_vTex;
GLuint texture_proj_mat_loc;
GLuint texture_camera_mat_loc;
GLuint texture_model_mat_loc;
const char *texture_vertex_shader = "../texture.vert";
const char *texture_frag_shader = "../texture.frag";

// Debug mirror shader
GLuint debug_mirror_program;
const char *debug_mirror_vertex_shader = "../debugMirror.vert";
const char *debug_mirror_frag_shader = "../debugMirror.frag";

// Mirror flag
GLboolean mirror = false;

// Global state
mat4 proj_matrix;
mat4 camera_matrix;
mat4 normal_matrix;
mat4 model_matrix;

vector<LightProperties> Lights;
vector<MaterialProperties> Materials;
GLuint numLights = 0;
GLint lightOn[8] = {0, 0, 0, 0, 0, 0, 0, 0};

// Global screen dimensions
GLint ww,hh;

void display();
void render_scene();

void build_geometry();
void build_solid_color_buffer(GLuint num_vertices, vec4 color, GLuint buffer);
void build_materials( );
void build_lights( );
void build_textures();
void build_mirror( );
void create_mirror();
void load_object(GLuint obj);
void draw_color_obj(GLuint obj, GLuint color);
void draw_mat_object(GLuint obj, GLuint material);
void draw_tex_object(GLuint obj, GLuint texture);
void framebuffer_size_callback(GLFWwindow *window, int width, int height);
void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods);
void mouse_callback(GLFWwindow *window, int button, int action, int mods);
void renderQuad(GLuint shader, GLuint tex);

// added functions
void draw_art();
void build_lightsaberart();
void build_vangoghart();
void build_grass();
void build_road();
void build_carpet();
void build_helicopter_pad();
void keep_cam_in_room(vec3 camera_eye, int projection);
GLboolean change_floors(vec3 camera_eye, int proj);
void build_garage_door();
void build_tv_screen();
void draw_windows();




int main(int argc, char**argv)
{
	// Create OpenGL window
	GLFWwindow* window = CreateWindow("Think Inside The Box");
    if (!window) {
        fprintf(stderr, "ERROR: could not open window with GLFW3\n");
        glfwTerminate();
        return 1;
    } else {
        printf("OpenGL window successfully created\n");
    }

    // Store initial window size
    glfwGetFramebufferSize(window, &ww, &hh);

    // Register callbacks
    glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
    glfwSetKeyCallback(window,key_callback);
    glfwSetMouseButtonCallback(window, mouse_callback);

    // Get initial time
    elTime = glfwGetTime();

    // Load shaders and associate variables
    ShaderInfo default_shaders[] = { {GL_VERTEX_SHADER, default_vertex_shader},{GL_FRAGMENT_SHADER, default_frag_shader},{GL_NONE, NULL} };
    default_program = LoadShaders(default_shaders);
    default_vPos = glGetAttribLocation(default_program, "vPosition");
    default_vCol = glGetAttribLocation(default_program, "vColor");
    default_proj_mat_loc = glGetUniformLocation(default_program, "proj_matrix");
    default_cam_mat_loc = glGetUniformLocation(default_program, "camera_matrix");
    default_model_mat_loc = glGetUniformLocation(default_program, "model_matrix");

    // Load shaders
    // Load light shader
    ShaderInfo lighting_shaders[] = { {GL_VERTEX_SHADER, lighting_vertex_shader},{GL_FRAGMENT_SHADER, lighting_frag_shader},{GL_NONE, NULL} };
    lighting_program = LoadShaders(lighting_shaders);
    lighting_vPos = glGetAttribLocation(lighting_program, "vPosition");
    lighting_vNorm = glGetAttribLocation(lighting_program, "vNormal");
    lighting_proj_mat_loc = glGetUniformLocation(lighting_program, "proj_matrix");
    lighting_camera_mat_loc = glGetUniformLocation(lighting_program, "camera_matrix");
    lighting_norm_mat_loc = glGetUniformLocation(lighting_program, "normal_matrix");
    lighting_model_mat_loc = glGetUniformLocation(lighting_program, "model_matrix");
    lighting_lights_block_idx = glGetUniformBlockIndex(lighting_program, "LightBuffer");
    lighting_materials_block_idx = glGetUniformBlockIndex(lighting_program, "MaterialBuffer");
    lighting_material_loc = glGetUniformLocation(lighting_program, "Material");
    lighting_num_lights_loc = glGetUniformLocation(lighting_program, "NumLights");
    lighting_light_on_loc = glGetUniformLocation(lighting_program, "LightOn");
    lighting_eye_loc = glGetUniformLocation(lighting_program, "EyePosition");

    // Load texture shaders
    ShaderInfo texture_shaders[] = { {GL_VERTEX_SHADER, texture_vertex_shader},{GL_FRAGMENT_SHADER, texture_frag_shader},{GL_NONE, NULL} };
    texture_program = LoadShaders(texture_shaders);
    texture_vPos = glGetAttribLocation(texture_program, "vPosition");
    texture_vTex = glGetAttribLocation(texture_program, "vTexCoord");
    texture_proj_mat_loc = glGetUniformLocation(texture_program, "proj_matrix");
    texture_camera_mat_loc = glGetUniformLocation(texture_program, "camera_matrix");
    texture_model_mat_loc = glGetUniformLocation(texture_program, "model_matrix");

    // Create geometry buffers
    build_geometry();
    // Create material buffers
    build_materials();
    // Create light buffers
    build_lights();
    // Create textures
    build_textures();
    // create mirror texture
    build_mirror();


    // Enable depth test
    glEnable(GL_CULL_FACE);
    glEnable(GL_DEPTH_TEST);

    // Set background color
    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);

    // enable alpha blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    // Set Initial camera position
    GLfloat x, y, z;

    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye_ortho = vec3(x, y, z); //eye= vec3(x,y,z);



    // Start loop
    while ( !glfwWindowShouldClose( window ) ) {
        create_mirror();
        // Uncomment instead of display() to view mirror map for debugging
       //renderQuad(debug_mirror_program, MirrorTex);
    	// Draw graphics
       display();
        // Update other events like input handling
        glfwPollEvents();
        GLdouble curTime = glfwGetTime();
        GLdouble dT = curTime - elTime;

        // rotation animations
        // fan
        if (fanspin){

        fan_ang += dT * (30.0/60.0) * 360.0;}

        // helicopter
        if (helicopter_fly){
            propellor_ang += dT * (180.0/60.0) * 360.0;

            helicopter_pos[1] += helicopter_dir*fly_value*dT;

            if ((helicopter_fly) && (helicopter_pos[1] > fly_max)) {
                helicopter_pos[1] = fly_max - 0.05f;
                helicopter_dir *= -1;
            } else if ((helicopter_fly) && (helicopter_pos[1] < fly_min)) {
                helicopter_pos[1] = fly_min + 0.05f;
                helicopter_dir *= -1;
            }


        }

        // if helicopter is stopped while in mid air
        if (!helicopter_fly && (helicopter_pos[1] > 2.0f)){

            helicopter_pos[1] -= (fly_value*5.0f)*dT;
        }


        // translation animations
        // fridge
        if (fridge_open)
        {
            if (fridgedoor_pos[2] > fridge_min){
                fridgedoor_pos[2] -= doors_step;


            }
        }
        else{
            if (fridgedoor_pos[2] < fridge_max){
                fridgedoor_pos[2] += doors_step;


            }
        }

        // if doors are open
        if (open_doors){
            // front door
            if (frontrightdoor_pos[0] < barn_max){
                frontrightdoor_pos[0] += doors_step;
                backrightdoor_pos[0] += doors_step;
                frontleftdoor_pos[0] -= doors_step;
                backleftdoor_pos[0] -= doors_step;
            }
        }
        else{

            // front door
            if (frontrightdoor_pos[0] > barn_min){
                frontrightdoor_pos[0] -= doors_step;
                backrightdoor_pos[0] -= doors_step;
                frontleftdoor_pos[0] += doors_step;
                backleftdoor_pos[0] += doors_step;

            }
        }


        if (open_windows){
            if (windows_posy <= window_max){
                windows_posy += .005f;
            }
        }

        else{
            if (windows_posy >= window_min){
                windows_posy -= .005f;
            }

        }

        elTime = curTime;
        // Swap buffer onto screen
        glfwSwapBuffers( window );
    }

    // Close window
    glfwTerminate();
    return 0;

}

void display( )
{
    // Declare projection and camera matrices
    proj_matrix = mat4().identity();
    camera_matrix = mat4().identity();

	// Clear window and depth buffer
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Compute anisotropic scaling
    GLfloat xratio = 1.0f;
    GLfloat yratio = 1.0f;
    // If taller than wide adjust y
    if (ww <= hh)
    {
        yratio = (GLfloat)hh / (GLfloat)ww;
    }
        // If wider than tall adjust x
    else if (hh <= ww)
    {
        xratio = (GLfloat)ww / (GLfloat)hh;
    }


    // Position camera for orthographic projection
    if (proj == ORTHOGRAPHIC)
    {
        // TODO: Set orthographic (birds-eye) view (spherical coords)
        proj_matrix = ortho(-20.0f*xratio, 20.0f*xratio, -20.0f*yratio, 20.0f * yratio, -40.0f, 40.0f);

        camera_matrix = lookat(eye_ortho, center_ortho, up);



    }
        // Position camera for perspective projection
    else if (proj == PERSPECTIVE_FIRST)
    {

        // TODO: Set dynamic perspective (first-person) view
        proj_matrix = frustum(-0.1f*xratio, 0.1f*xratio, -0.1f*yratio, 0.1f*yratio, 0.1f, 30.0f);

        // Set camera matrix
        camera_matrix = lookat(eye, center, up);

    }

    else if (proj == PERSPECTIVE_SECOND)
    {

        // TODO: Set dynamic perspective (first-person) view
        proj_matrix = frustum(-0.1f*xratio, 0.1f*xratio, -0.1f*yratio, 0.1f*yratio, 0.1f, 30.0f);

        // Set camera matrix
        camera_matrix = lookat(eye, center, up);

    }


    // Render objects
	render_scene();

	// Flush pipeline
	glFlush();
}

void render_scene( ) {
    // Declare transformation matrices
    model_matrix = mat4().identity();
    mat4 scale_matrix = mat4().identity();
    mat4 rot_matrix = mat4().identity();
    mat4 trans_matrix = mat4().identity();

    // draw barn

    scale_matrix = scale(0.9f, 1.0f, 1.0f);
    trans_matrix = translate(BARN_X, BARN_Y, BARN_Z);
    model_matrix = trans_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    if (barn_transparent){
    glDepthMask(GL_FALSE);
    draw_mat_object(Barn, Transparent);
    glDepthMask(GL_TRUE);}

    else if (!barn_transparent){
    glDisable(GL_CULL_FACE);
    draw_tex_object(Barn, RedWood);
    glEnable(GL_CULL_FACE);}

    // draw barn roof
    scale_matrix = scale(1.6f, 2.0f, 1.5f);
    rot_matrix = rotate(-90.0f, 0.0f, 1.0f, 0.0f);
    trans_matrix = translate(0.0f, 0.3f, 5.0f);
    model_matrix = trans_matrix * scale_matrix * rot_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    if (!barn_transparent)
    draw_tex_object(BarnRoof, GreyWood);
    else if (barn_transparent){
    glDepthMask(GL_FALSE);
    draw_mat_object(BarnRoof, Transparent);
    glDepthMask(GL_TRUE);}

    // FIRST FLOOR
    // draw cross windows
    draw_windows();

    // draw stairs
    rot_matrix = rotate(90.0f, 0.0f, 1.0f, 0.0f);
    scale_matrix = scale(5.0f, 4.0f, 4.0f);
    trans_matrix = translate(-3.0f, 0.0f, 4.5f);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_tex_object(Stairs,LightBrownWood);

    // draw left barn door
    scale_matrix = scale(0.5f, 0.45f, 0.85f);
    rot_matrix = rotate(90.0f, 0.0f, 1.0f, 0.0f);
    trans_matrix = translate(frontleftdoor_pos);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_tex_object(BarnDoor, WhiteWood);

    // draw left back barn door
    trans_matrix = translate(backleftdoor_pos);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_tex_object(BackBarnDoor, BlackWood);

    // draw right barn door
    scale_matrix = scale(0.5f, 0.45f, 0.85f);
    rot_matrix = rotate(90.0f, 0.0f, 1.0f, 0.0f);
    trans_matrix = translate(frontrightdoor_pos);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_tex_object(BarnDoor, WhiteWood);

    // draw right back barn door
    trans_matrix = translate(backrightdoor_pos);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_tex_object(BackBarnDoor, BlackWood);

    // draw back barn door
    scale_matrix = scale(1.0f, 2.0f, 0.25f);
    trans_matrix = translate(0.0f, 1.25f, -2.5f);
    model_matrix = trans_matrix * scale_matrix;


    if (open_doors){
        rot_matrix = rotate(-90.0f, 0.0f, 1.0f, 0.0f);
        trans_matrix = translate(0.5f, 1.25f, -2.0f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
    }
    normal_matrix = model_matrix.inverse().transpose();
    draw_tex_object(Cube, LightBrownWood);

    // draw fridge
    scale_matrix = scale(.25f, .5f, .3f);
    trans_matrix = translate(FRIDGE_X, FRIDGE_Y, FRIDGE_Z);
    model_matrix = trans_matrix *  scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    glDisable(GL_CULL_FACE);
    draw_mat_object(Fridge, Silver);
    glEnable(GL_CULL_FACE);


    // draw fridge door
    scale_matrix = scale(.65f, .5f, .1f);
    rot_matrix = rotate(90.0f, 0.0f, 1.0f, 0.0f);
    trans_matrix = translate(fridgedoor_pos);
    model_matrix = trans_matrix *  rot_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_mat_object(FridgeDoor, Silver);


    // draw table
    scale_matrix = scale(0.4f, 0.4f, 0.4f);
    trans_matrix = translate(TABLE_X, TABLE_Y, TABLE_Z);

    model_matrix = trans_matrix * scale_matrix;

    draw_tex_object(Table, LightBrownWood);

    // draw couch
    scale_matrix = scale(0.5f, 0.4f, 0.5f);
    trans_matrix = translate(COUCH_X, COUCH_Y, COUCH_Z);
    model_matrix = trans_matrix * scale_matrix;

    draw_tex_object(Couch, RedWool);


    // draw fan
    scale_matrix = scale(0.35f, 0.35f, 0.35f);
    trans_matrix = translate(0.0f, 4.0f, 4.0f);
    rot_matrix = rotate(fan_pos, 0.0f, 1.0f, 0.0f);

    if (fanspin){
        rot_matrix=rotate(fan_ang, 0.0f, 1.0f, 0.0f);
    }
    model_matrix = trans_matrix * rot_matrix * scale_matrix;

    normal_matrix = model_matrix.inverse().transpose();
    draw_tex_object(Fan, LightBrownWood);


    // draw bowl
    scale_matrix = scale(0.4f, 0.4f, 0.4f);
    trans_matrix = translate(BOWL_X, BOWL_Y, BOWL_Z);
    model_matrix = trans_matrix * scale_matrix;
    glDisable(GL_CULL_FACE);
    draw_tex_object(Bowl, BlackWood);
    glEnable(GL_CULL_FACE);

    // draw fruit
    trans_matrix = translate(1.15f, 2.0f, 2.5f);
    scale_matrix = scale(0.125f, 0.125f, 0.125f);
    model_matrix = trans_matrix * scale_matrix;

    draw_tex_object(Sphere, Apple);

    model_matrix *= translate(1.5f, 0.0f, 0.0f);

    draw_tex_object(Sphere, Apple);

    model_matrix *= translate(-.75f, 0.0f, -1.0f);

    draw_tex_object(Sphere, Apple);

    // draw glass
 //   rot_matrix = rotate(180.0f, 1.0f, 0.0f, 0.0f);
    scale_matrix = scale(0.3f, 0.2f, .25f);
    trans_matrix = translate(CUP_X, CUP_Y, CUP_Z);
    model_matrix = trans_matrix * scale_matrix; //rot_matrix * scale_matrix;

    draw_mat_object(Glass, PurpleTransparent);


    // draw soda
    scale_matrix = scale(0.275f, 0.2f, .25f);
    model_matrix = trans_matrix * scale_matrix;
   draw_mat_object(Cylinder,BlackPlastic);

    // draw soda can
    trans_matrix = translate(1.9f, 2.0f, 1.5f);

    model_matrix = trans_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    glDisable(GL_CULL_FACE);
    draw_tex_object(Can, Soda);
    glEnable(GL_CULL_FACE);

    // draw back chair

    scale_matrix = scale(0.4f, 0.4f, 0.4f);
    trans_matrix = translate(CHAIR2_X, CHAIR2_Y, CHAIR2_Z);

    model_matrix = trans_matrix * scale_matrix;

    draw_tex_object(Chair, DarkBrownWood);


    // draw front chair
    scale_matrix = scale(0.4f, 0.4f, 0.4f);
    rot_matrix = rotate(-180.0f, 0.0f, 1.0f, 0.0f);
    trans_matrix = translate(CHAIR1_X, CHAIR1_Y, CHAIR1_Z);

    model_matrix = trans_matrix * rot_matrix * scale_matrix;

    draw_tex_object(Chair, DarkBrownWood);

    // draw frames

    // left
    scale_matrix = scale(0.5f, 1.1f, 0.85f);
    trans_matrix = translate(STARRYNIGHTFRAME_X, STARRYNIGHTFRAME_Y, STARRYNIGHTFRAME_Z);

    model_matrix = trans_matrix * scale_matrix;

    draw_tex_object(PictureFrame, BlackWood);

    // right
    trans_matrix = translate(LIGHTSABERFRAME_X, LIGHTSABERFRAME_Y, LIGHTSABERFRAME_Z);

    model_matrix = trans_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();

    draw_tex_object(PictureFrame, BlackWood);


// SECOND FLOOR

    if (!open_windows){
    // draw small window
    scale_matrix = scale(0.5f, 0.45f, 0.85f);
    trans_matrix = translate(small_windowpos);
    rot_matrix = rotate(90.0f, 0.0f, 1.0f, 0.0f);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_tex_object(BarnDoor, WhiteWood);

    // draw big window
    scale_matrix = scale(0.75f, 0.85f, 0.5f);
    trans_matrix = translate(big_windowpos);
    model_matrix = trans_matrix *scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    glDisable(GL_CULL_FACE);
    draw_tex_object(BigWindow, WhiteWood);
    glEnable(GL_CULL_FACE);}

    if (open_windows){
        // small window
        scale_matrix = scale(0.5f, 0.45f, 0.85f);
        trans_matrix = translate(small_windowpos[0], small_windowpos[1], small_windowpos[2] + 2.0f);
        model_matrix = trans_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        draw_tex_object(BarnDoor, WhiteWood);

        // big window
        scale_matrix = scale(0.75f, 0.85f, 0.5f);
        trans_matrix = translate(2.0f, 1.3f, -4.2f);
        rot_matrix = rotate(90.0f, 0.0f, 1.0f, 0.0f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        normal_matrix = model_matrix.inverse().transpose();
        glDisable(GL_CULL_FACE);
        draw_tex_object(BigWindow, WhiteWood);
        glEnable(GL_CULL_FACE);
    }


    // draw second floor
    scale_matrix = scale(9.0f, 0.5f, 12.5f);
    trans_matrix = translate(0.0f, 5.0f, 6.0f);
    model_matrix = trans_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    draw_tex_object(Cube, DarkBrownWood);


    // draw bed
    scale_matrix = scale(0.4f, 0.4f, 0.35f);
    rot_matrix = rotate(90.0f, 0.0f, 1.0f, 0.0f);
    trans_matrix = translate(BED_X, BED_Y, BED_Z);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    normal_matrix = model_matrix.inverse().transpose();
    glDisable(GL_CULL_FACE);
    draw_tex_object(Bed, BlackWood);
    glEnable(GL_CULL_FACE);

    // draw mattress
    scale_matrix = scale(2.75f, 0.20f,1.65f);
    trans_matrix = translate(MATTRESS_X, MATTRESS_Y, MATTRESS_Z);

    model_matrix = trans_matrix * scale_matrix;
    draw_color_obj(Cube, Blue);

    // draw tv stand
    scale_matrix = scale(0.5f, 0.2f, 0.3f);
    trans_matrix = translate(TV_X, TV_Y - 0.5f, TV_Z);
    model_matrix = trans_matrix * scale_matrix;
    draw_tex_object(TVStand, GreyWood);

    // draw tv
    scale_matrix = scale(0.35f, 0.3f, 0.1f);
    rot_matrix = rotate(90.0f,0.0f,1.0f,0.0f);
    trans_matrix = translate(TV_X, TV_Y, TV_Z);
    model_matrix = trans_matrix * rot_matrix * scale_matrix;
    draw_mat_object(TV, BlackPlastic);

    // draw tree
    scale_matrix = scale(0.75f, 1.0f, 0.75f);
    trans_matrix = translate(TREE_X, TREE_Y, TREE_Z);

    model_matrix = trans_matrix * scale_matrix;

    draw_tex_object(Cone, TreeTex);

    // draw tree stand
    scale_matrix = scale(0.2f, 0.5f, 0.2f);
    trans_matrix = translate(TREESTAND_X, TREESTAND_Y, TREESTAND_Z);

    model_matrix = trans_matrix * scale_matrix;

    draw_mat_object(Cylinder, BlackPlastic);


    // draw mirror
    if (!mirror) {
        // Render mirror in scene
        // TODO: Set mirror translation
        trans_matrix = translate(mirror_eye);
        rot_matrix = rotate(90.0f, vec3(1.0f, 0.0f, 0.0f));
        scale_matrix = scale(1.5f, 0.75f, 2.0f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix ;//rot_matrix * scale_matrix;
        // TODO: Draw mirror
       draw_tex_object(Mirror, MirrorTex);
    }




    // outside

        // draw textures
        draw_art();

        // draw silo
        scale_matrix = scale(1.0f, 1.25f, 1.0f);
        trans_matrix = translate(SILO_X, SILO_Y, SILO_Z);
        model_matrix = trans_matrix * scale_matrix;
        draw_tex_object(Silo, SiloTex);

        // draw windmill base
        scale_matrix = scale(1.0f, 1.5f, 1.0f);
        rot_matrix = rotate(90.0f, 0.0f, 1.0f, 0.0f);
        trans_matrix = translate(windmill_pos);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        draw_mat_object(WindMillBase, BlackPlastic);


        // draw windmill propellors
        scale_matrix = scale(2.0f, 2.0f, 2.0f);
        trans_matrix = translate(windmill_pos[0], windmill_pos[1] + 2.0f, windmill_pos[2] + 3.0f);
        model_matrix = trans_matrix * scale_matrix;
        draw_mat_object(WindMill, BlackPlastic);

        // draw garage
        scale_matrix = scale(1.0f, 1.0f, 1.0f);
        rot_matrix = rotate(-90.0f, 0.0f, 1.0f, 0.0f);
        trans_matrix = translate(GARAGE_X, GARAGE_Y, GARAGE_Z);
        model_matrix = trans_matrix * scale_matrix * rot_matrix;

        draw_tex_object(Garage, Stone);

        // draw helicopter
        rot_matrix = rotate(-45.0f, 0.0f, 1.0f, 0.0f);
        scale_matrix = scale(1.0f, 1.0f, 1.0f);
        trans_matrix = translate(helicopter_pos);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;

        draw_mat_object(Helicopter, RedPlastic);

        // draw heli blades
        rot_matrix = rotate(propellor_ang, 0.0f, 1.0f, 0.0f);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        draw_mat_object(Propellors, BlackPlastic);

        // draw window
        rot_matrix = rotate(-45.0f, 0.0f, 1.0f, 0.0f);
        trans_matrix = translate(helicopter_pos);
        model_matrix = trans_matrix * rot_matrix * scale_matrix;
        draw_mat_object(HelicopterWindow, BlackTransparent);



}

void build_geometry( )
{
    // Generate vertex arrays and buffers
    glGenVertexArrays(NumVAOs, VAOs);

    // Load models
    load_object(BackBarnDoor);
    load_object(Barn);
    load_object(BarnDoor);
    load_object(BarnRoof);
    load_object(Bed);
    load_object(Bowl);
    load_object(Can);
    load_object(Chair);
    load_object(Couch);
    load_object(Cone);
    load_object(Cube);
    load_object(Cylinder);
    load_object(Fan);
    load_object(Fridge);
    load_object(FridgeDoor);
    load_object(Garage);
    load_object(Glass);
    load_object(Helicopter);
    load_object(HelicopterWindow);
    load_object(Propellors);
    load_object(PictureFrame);
    load_object(Silo);
    load_object(Sphere);
    load_object(Stairs);
    load_object(Table);
    load_object(TV);
    load_object(TVStand);
    load_object(Window);
    load_object(BigWindow);
    load_object(WindMill);
    load_object(WindMillBase);
    load_object(Mirror);


    build_lightsaberart();
    build_vangoghart();
    build_grass();
    build_road();
    build_carpet();
    build_helicopter_pad();
    build_garage_door();
    build_tv_screen();


    // Generate color buffers
    glGenBuffers(NumColorBuffers, ColorBuffers);

    // Build color buffers
    build_solid_color_buffer(numVertices[Cube], vec4(0.0f, 0.0f, 1.0f, 1.0f), Blue);
    build_solid_color_buffer(numVertices[Cube], vec4(0.0f, 0.0f, 0.0f, 1.0f), Black);
    build_solid_color_buffer(numVertices[Cube], vec4(1.0f, 1.0f, 1.0f, 1.0f), White);

}

void build_materials( ) {

   // create silver
    MaterialProperties silver = {

            vec4(0.23125f, 0.23125f, 0.23125f, 1.0f ), //ambient
            vec4(0.2775f, 0.2775f, 0.2775f, 1.0f ), //diffuse
            vec4(1.0, 1.0, 1.0, 1.0f), //specular
            89.6f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad

    };

    // Create red plastic material
    MaterialProperties redPlastic = {
            vec4(0.3f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.6f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(0.8f, 0.6f, 0.6f, 1.0f), //specular
            50.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    // Create black plastic material
    MaterialProperties blackPlastic = {
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //ambient
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //diffuse
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    // Create peach plastic material
    MaterialProperties blackTransparent = {
            vec4(0.0f, 0.0f, 0.0f, 0.25f), //ambient
            vec4(0.0f, 0.0f, 0.0f, 0.25f), //diffuse
            vec4(0.0f, 0.0f, 0.0f, 0.25f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties transparent = {
            vec4(0.25f, 0.25f, 0.25f, 0.25f), //ambient
            vec4(0.25f, 0.25f, 0.25f, 0.25f), //diffuse
            vec4(0.25f, 0.25f, 0.25f, 0.25f), //specular
            32.0f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };

    MaterialProperties purpletransparent = {
            vec4(0.4f, 0.0f, 0.8f, 1.0f), //ambient
            vec4(0.2f, 0.0f, 0.5f, 1.0f), //diffuse
            vec4(0.333333f, 0.333333f, 0.521569f, 1.0f), //specular
            9.846f, //shininess
            {0.0f, 0.0f, 0.0f}  //pad
    };
    // Add materials to Materials vector
    Materials.push_back(silver);
    Materials.push_back(redPlastic);
    Materials.push_back(blackPlastic);
    Materials.push_back(blackTransparent);
    Materials.push_back(transparent);
    Materials.push_back(purpletransparent);


    glGenBuffers(NumMaterialBuffers, MaterialBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, MaterialBuffers[MaterialBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Materials.size()*sizeof(MaterialProperties), Materials.data(), GL_STATIC_DRAW);
}

void build_lights( ) {

    LightProperties fanLight = {
            POINT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //ambient
            vec4(.25f, .25f, .25f, .25f), //diffuse
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //specular
            vec4(0.0f, 4.0f, 5.0f, 1.0f),  //position
            vec4(0.0f, -1.0f, 0.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    // point light
   LightProperties sunLight = {
            POINT, //type
            {0.0f, 0.0f, 0.0f}, //pad
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //ambient
            vec4(1.0f, 1.0f, 1.0f, 1.0f), //diffuse
            vec4(0.0f, 0.0f, 0.0f, 1.0f), //specular
            vec4(0.0f, 20.0f, 0.0f, 1.0f),  //position
            vec4(0.0f, 0.0f, 0.0f, 0.0f), //direction
            0.0f,   //cutoff
            0.0f,  //exponent
            {0.0f, 0.0f}  //pad2
    };

    // Add lights to Lights vector
    Lights.push_back(fanLight);
    Lights.push_back(sunLight);

    // Set numLights
    numLights = Lights.size();

    // Turn sunlight on
    lightOn[1] = 1;

    // Create uniform buffer for lights
    glGenBuffers(NumLightBuffers, LightBuffers);
    glBindBuffer(GL_UNIFORM_BUFFER, LightBuffers[LightBuffer]);
    glBufferData(GL_UNIFORM_BUFFER, Lights.size()*sizeof(LightProperties), Lights.data(), GL_STATIC_DRAW);
}

void key_callback(GLFWwindow *window, int key, int scancode, int action, int mods) {
    // ESC to quit
    if (key == GLFW_KEY_ESCAPE) {
        glfwSetWindowShouldClose(window, true);
    }

    // Toggle projection mode
    if ((key == GLFW_KEY_O) && (action == GLFW_PRESS))
    {
        azimuth = 0.0f;
        elevation = 90.0f;
        proj = ORTHOGRAPHIC;


    }

    else if ((key == GLFW_KEY_P) && (action == GLFW_PRESS))
    {
        azimuth = -90.0f;
        elevation = 90.0f;

        proj = PERSPECTIVE_FIRST;
    }

    // perspective camera controls
    if ((proj == PERSPECTIVE_FIRST) || (proj == PERSPECTIVE_SECOND)){
            GLboolean changed = false;

            // move forward
            if (key == GLFW_KEY_W) {
                vec3 direction = center - eye;
                eye = eye + (direction * step);
                if (!free_movement){
                changed = change_floors(eye, proj);
                if (changed == true){
                    if (proj == PERSPECTIVE_FIRST){

                        proj = PERSPECTIVE_SECOND;
                    }
                    else{
                        proj = PERSPECTIVE_FIRST;

                    }
                    azimuth = 90.0f;
                }

               keep_cam_in_room(eye, proj);}




            }


            // move backward

            if (key == GLFW_KEY_S) {
                vec3 direction = center - eye;
                eye = eye - (direction * step);
                if (!free_movement){
                changed = change_floors(eye, proj);
                if (changed == true){
                    if (proj == PERSPECTIVE_FIRST){
                        proj = PERSPECTIVE_SECOND;
                    }
                    else{
                        proj = PERSPECTIVE_FIRST;
                    }
                }

                keep_cam_in_room(eye, proj);}

            }






    }


    // Adjust azimuth
    if (key == GLFW_KEY_A) {
        azimuth -= daz;
        if (azimuth > 360.0) {
            azimuth -= 360.0;
        }

    } else if (key == GLFW_KEY_D) {
        azimuth += daz;
        if (azimuth < 0.0)
        {
            azimuth += 360.0;
        }

    }

    // Adjust elevation angle
    if (key == GLFW_KEY_Z)
    {
        elevation += del;
        if (elevation > 179.0)
        {
            elevation = 179.0;
        }

    }
    else if (key == GLFW_KEY_X)
    {
        elevation -= del;
        if (elevation < 1.0)
        {
            elevation = 1.0;
        }


    }


    center = vec3(eye[0] + float(cos(azimuth*DEG2RAD)), eye[1] + float(cos(elevation*DEG2RAD)), eye[2] + float(sin(azimuth*DEG2RAD)));


    // ANIMATIONS
        // fan spin animation
        if ((key == GLFW_KEY_F) && (action == GLFW_PRESS))
        {
            fan_pos = fan_ang;
            if (fanspin){

            fanspin = false;}
            else{
            fanspin = true;}
        }

        // turn fan light on/off
    if ((key == GLFW_KEY_L) && (action == GLFW_PRESS)) {

        if (lightOn[0] == 1){
            lightOn[0] = 0;}
        else{
            lightOn[0] = 1;
        }

    }


        // open fridge animation
        if ((key == GLFW_KEY_M) && (action == GLFW_PRESS))
        {
            if (fridge_open){
                fridge_open = false;}
            else{
                fridge_open = true;}
        }

        // open doors animation
        if ((key == GLFW_KEY_C) && (action == GLFW_PRESS))
        {
            if (open_doors){
                open_doors = false;}
            else{
                open_doors = true;}
        }

        // open windows animation
        if ((key == GLFW_KEY_UP) && (action == GLFW_PRESS))
        {
            if (!open_windows){
                open_windows = true;
            }

        }

        // close windows animation
        if ((key == GLFW_KEY_DOWN) && (action == GLFW_PRESS)){
            if (open_windows){
                open_windows = false;
            }
        }


        // helicopter on/off
        if ((key == GLFW_KEY_H) && (action == GLFW_PRESS))
        {
            if (helicopter_fly){
                helicopter_fly = false;

            }
            else{
                helicopter_fly = true;
            }
        }


        // tv on/off
        if ((key == GLFW_KEY_T) && (action == GLFW_PRESS))
        {
            if (tv_on){
                tv_on = false;
            }
            else{
                tv_on = true;

            }
        }


        // barn transparency
        if ((key == GLFW_KEY_B) && (action == GLFW_PRESS))
        {
            if (barn_transparent){
                barn_transparent = false;}
            else{
                barn_transparent = true;}
        }

        // unrestricted movement

        if ((key == GLFW_KEY_0) && (action == GLFW_PRESS))
        {
            if (free_movement){
                free_movement = false;}
            else{
                free_movement = true;}
        }





    if (proj == ORTHOGRAPHIC){
    // Compute updated camera position
    GLfloat x, y, z;
    x = (GLfloat)(radius*sin(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    y = (GLfloat)(radius*cos(elevation*DEG2RAD));
    z = (GLfloat)(radius*cos(azimuth*DEG2RAD)*sin(elevation*DEG2RAD));
    eye_ortho = vec3(x, y,  z); }






}

void mouse_callback(GLFWwindow *window, int button, int action, int mods){

}
#include "utilfuncs.cpp"


/* USER GUIDE
    O to toggle orthographic cam
    P to toggle perspective cam
    F to spin fan
    UP/DOWN to open/close windows
    T to turn TV on/off
    M to open/close fridge
    L to turn fan light on/off
    C to open/close doors
    H to fly helicopter
    B to make barn transparent
    0 to move throughout scene unrestricted
*/




