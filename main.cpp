#define GL_SILENCE_DEPRECATION
#define GL_GLEXT_PROTOTYPES 1
#define FIXED_TIMESTEP 0.0166666f
#define LEVEL1_WIDTH 14
#define LEVEL1_HEIGHT 8
#define LEVEL1_LEFT_EDGE 5.0f
#define LOG(argument) std::cout << argument << '\n'


#ifdef _WINDOWS
#include <GL/glew.h>
#endif

#include <list>
#include <SDL_mixer.h>
#include <SDL.h>
#include <SDL_opengl.h>
#include "glm/mat4x4.hpp"
#include "glm/gtc/matrix_transform.hpp"
#include "ShaderProgram.h"
#include "cmath"
#include <ctime>
#include <vector>
#include "Entity.h"
#include "Map.h"
#include "Utility.h"
#include "Scene.h"
#include "MainMenu.hpp"
#include "LevelA.h"
#include "LevelB.h"
#include "LevelC.hpp"
#include "Effects.h"

// ––––– CONSTANTS ––––– //
const int WINDOW_WIDTH  = 640,
          WINDOW_HEIGHT = 480,
          FONTBANK_SIZE = 16;

const float BG_RED     = 0.1922f,
            BG_BLUE    = 0.549f,
            BG_GREEN   = 0.9059f,
            BG_OPACITY = 1.0f;

const int VIEWPORT_X = 0,
          VIEWPORT_Y = 0,
          VIEWPORT_WIDTH  = WINDOW_WIDTH,
          VIEWPORT_HEIGHT = WINDOW_HEIGHT;

const char V_SHADER_PATH[] = "shaders/vertex_textured.glsl",
           F_SHADER_PATH[] = "shaders/fragment_textured.glsl",
           FONT_FILEPATH[] = "/Users/chelsea/Desktop/Final/SDLProject/assets/font1.png";

const float MILLISECONDS_IN_SECOND = 1000.0;


// ––––– GLOBAL VARIABLES ––––– //
int g_frame_counter;
int g_death_count = 0;

Scene  *g_current_scene;

Level0 *g_level0;
LevelA *g_levelA;
LevelB *g_levelB;
LevelC *g_levelC;

Effects *g_effects;
Scene   *g_levels[4];

SDL_Window* g_display_window;
bool g_game_is_running = true,
       is_game_running = true,
         final_lvl_completed = false;

ShaderProgram g_program;
glm::mat4 g_view_matrix, g_projection_matrix;



float g_previous_ticks = 0.0f;
float g_accumulator = 0.0f;

bool g_is_colliding_bottom = false;

// ––––– GENERAL FUNCTIONS ––––– //
void switch_to_scene(Scene *scene)
{
    g_current_scene = scene;
    g_current_scene->initialise(); // DON'T FORGET THIS STEP!
}


void draw_text(ShaderProgram *program, GLuint font_texture_id, std::string text, float screen_size, float spacing, glm::vec3 position)
{
    // Scale the size of the fontbank in the UV-plane
    // We will use this for spacing and positioning
    float width = 1.0f / FONTBANK_SIZE;
    float height = 1.0f / FONTBANK_SIZE;

    // Instead of having a single pair of arrays, we'll have a series of pairs—one for each character
    // Don't forget to include <vector>!
    std::vector<float> vertices;
    std::vector<float> texture_coordinates;

    // For every character...
    for (int i = 0; i < text.size(); i++) {
        // 1. Get their index in the spritesheet, as well as their offset (i.e. their position
        //    relative to the whole sentence)
        int spritesheet_index = (int) text[i];  // ascii value of character
        float offset = (screen_size + spacing) * i;
        
        // 2. Using the spritesheet index, we can calculate our U- and V-coordinates
        float u_coordinate = (float) (spritesheet_index % FONTBANK_SIZE) / FONTBANK_SIZE;
        float v_coordinate = (float) (spritesheet_index / FONTBANK_SIZE) / FONTBANK_SIZE;

        // 3. Inset the current pair in both vectors
        vertices.insert(vertices.end(), {
            offset + (-0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (0.5f * screen_size), -0.5f * screen_size,
            offset + (0.5f * screen_size), 0.5f * screen_size,
            offset + (-0.5f * screen_size), -0.5f * screen_size,
        });

        texture_coordinates.insert(texture_coordinates.end(), {
            u_coordinate, v_coordinate,
            u_coordinate, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate + width, v_coordinate + height,
            u_coordinate + width, v_coordinate,
            u_coordinate, v_coordinate + height,
        });
    }

    // 4. And render all of them using the pairs
    glm::mat4 model_matrix = glm::mat4(1.0f);
    model_matrix = glm::translate(model_matrix, position);
    
    program->SetModelMatrix(model_matrix);
    glUseProgram(program->programID);
    
    glVertexAttribPointer(program->positionAttribute, 2, GL_FLOAT, false, 0, vertices.data());
    glEnableVertexAttribArray(program->positionAttribute);
    glVertexAttribPointer(program->texCoordAttribute, 2, GL_FLOAT, false, 0, texture_coordinates.data());
    glEnableVertexAttribArray(program->texCoordAttribute);
    
    glBindTexture(GL_TEXTURE_2D, font_texture_id);
    glDrawArrays(GL_TRIANGLES, 0, (int) (text.size() * 6));
    
    glDisableVertexAttribArray(program->positionAttribute);
    glDisableVertexAttribArray(program->texCoordAttribute);
}

void initialise()
{
    SDL_Init(SDL_INIT_VIDEO | SDL_INIT_AUDIO);
    g_display_window = SDL_CreateWindow("Hello, Special Effects!",
                                      SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                      WINDOW_WIDTH, WINDOW_HEIGHT,
                                      SDL_WINDOW_OPENGL);
    
    SDL_GLContext context = SDL_GL_CreateContext(g_display_window);
    SDL_GL_MakeCurrent(g_display_window, context);
    
#ifdef _WINDOWS
    glewInit();
#endif
    
    glViewport(VIEWPORT_X, VIEWPORT_Y, VIEWPORT_WIDTH, VIEWPORT_HEIGHT);
    
    g_program.Load(V_SHADER_PATH, F_SHADER_PATH);
    
    g_view_matrix = glm::mat4(1.0f);
    g_projection_matrix = glm::ortho(-5.0f, 5.0f, -3.75f, 3.75f, -1.0f, 1.0f);
    
    g_program.SetProjectionMatrix(g_projection_matrix);
    g_program.SetViewMatrix(g_view_matrix);
    
    glUseProgram(g_program.programID);
    
    glClearColor(BG_RED, BG_BLUE, BG_GREEN, BG_OPACITY);
    
    // enable blending
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    g_level0  = new Level0();
    g_levelA  = new LevelA();
    g_levelB  = new LevelB();
    g_levelC  = new LevelC();
    
    g_levels[0] = g_level0;
    g_levels[1] = g_levelA;
    g_levels[2] = g_levelB;
    g_levels[3] = g_levelC;
    
   
    // Start at level 0
    switch_to_scene(g_levels[0]);
    g_effects = new Effects(g_projection_matrix, g_view_matrix);
    g_effects->start(SHRINK, 2.0f);
    
    g_frame_counter = 0;
}

void process_input()
{
    // VERY IMPORTANT: If nothing is pressed, we don't want to go anywhere
    g_current_scene->m_state.player->set_movement(glm::vec3(0.0f));
    
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        switch (event.type) {
            // End game
            case SDL_QUIT:
            case SDL_WINDOWEVENT_CLOSE:
                g_game_is_running = false;
                break;
                
            case SDL_KEYDOWN:
                switch (event.key.keysym.sym) {
                    case SDLK_q:
                        // Quit the game with a keystroke
                        g_game_is_running = false;
                        break;
                        
                    case SDLK_SPACE:
                        // Jump
                        if (g_current_scene->m_state.player->m_collided_bottom)
                        {
                            g_current_scene->m_state.player->m_is_jumping = true;
                            Mix_PlayChannel(-1, g_current_scene->m_state.jump_sfx, 0);
                        }
                        break;
                    case SDLK_RETURN:
                        if (g_current_scene == g_levels[0]) switch_to_scene(g_levels[1]);
                        break;

                    default:
                        break;
                }
                
            default:
                break;
        }
    }
    
    const Uint8 *key_state = SDL_GetKeyboardState(NULL);

    if (key_state[SDL_SCANCODE_LEFT])
    {
        g_current_scene->m_state.player->m_movement.x = -1.0f;
        g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->LEFT];
    }
    else if (key_state[SDL_SCANCODE_RIGHT])
    {
        g_current_scene->m_state.player->m_movement.x = 1.0f;
        g_current_scene->m_state.player->m_animation_indices = g_current_scene->m_state.player->m_walking[g_current_scene->m_state.player->RIGHT];
    }
    
    if (glm::length(g_current_scene->m_state.player->m_movement) > 1.0f)
    {
        g_current_scene->m_state.player->m_movement = glm::normalize(g_current_scene->m_state.player->m_movement);
    }
}


void update()
{
    g_frame_counter++;
    
    float ticks = (float)SDL_GetTicks() / MILLISECONDS_IN_SECOND;
    float delta_time = ticks - g_previous_ticks;
    g_previous_ticks = ticks;
    
    delta_time += g_accumulator;
    
    if (delta_time < FIXED_TIMESTEP)
    {
        g_accumulator = delta_time;
        return;
    }
    
    while (delta_time >= FIXED_TIMESTEP) {
        g_current_scene->update(FIXED_TIMESTEP);
        g_effects->update(FIXED_TIMESTEP);
        
        
        if (g_is_colliding_bottom == false && g_current_scene->m_state.player->m_collided_bottom) g_effects->start(SHAKE, 1.0f);
        
        g_is_colliding_bottom = g_current_scene->m_state.player->m_collided_bottom;
        
        delta_time -= FIXED_TIMESTEP;
    }
    
    g_accumulator = delta_time;
    
    // Prevent the camera from showing anything outside of the "edge" of the level
    g_view_matrix = glm::mat4(1.0f);
    
    if (g_current_scene->m_state.player->get_position().x > LEVEL1_LEFT_EDGE) {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-g_current_scene->m_state.player->get_position().x, 3.75, 0));
    } else {
        g_view_matrix = glm::translate(g_view_matrix, glm::vec3(-5, 3.75, 0));
    }
    
    int defeated_enemy_count = 0;
    for (int i = 0; i < g_current_scene->get_number_of_enemies(); ++i) {
        if ((g_current_scene->m_state.enemies[i].get_is_active()) == false) ++defeated_enemy_count;
    }

    // if the player has fallen off the map
    
    if (defeated_enemy_count < g_current_scene->get_number_of_enemies() && g_current_scene->m_state.player->get_position().y < -10.0f) {
        ++g_death_count;
        switch_to_scene(g_current_scene);
    }
    else if (defeated_enemy_count == g_current_scene->get_number_of_enemies()) {
        if (g_current_scene == g_levelA && g_current_scene->m_state.player->get_position().y < -10.0f) {
            switch_to_scene(g_levelB);
            defeated_enemy_count = 0;
            g_frame_counter = 0;
        }
        else if (g_current_scene == g_levelB && g_current_scene->m_state.player->get_position().y < -10.0f) {
            switch_to_scene(g_levelC);
            defeated_enemy_count = 0;
            g_frame_counter = 0;
        }
        else if (g_current_scene == g_levelC) {
            final_lvl_completed = true;
        }
    }
    
    
    g_view_matrix = glm::translate(g_view_matrix, g_effects->m_view_offset);
    
   
    
    if (!(g_current_scene->m_state.player->get_is_active())) {
        ++g_death_count;
        if (g_death_count < 3) {
            switch_to_scene(g_current_scene); // restart the level (include a choice for the user to continue)
            g_frame_counter = 0;
        }
    }
    
    
//    LOG("DEATH COUNT");
//    LOG(g_death_count);
    if (g_death_count >= 3) {
        is_game_running = false;
    }
}

void render()
{
    GLuint font_texture_id = Utility::load_texture(FONT_FILEPATH);
    g_program.SetViewMatrix(g_view_matrix);
    glClear(GL_COLOR_BUFFER_BIT);
    
//    bool appear = true;
    std::list<int> range = {0,1,2,3,4,5,6,7,8,9,10,11,12,13,14,15};
    for (int i = *(range.begin()); i < *(range.end()); ++i ) {
        if (g_current_scene == g_levels[0]) {
            draw_text(&g_program, font_texture_id, "Press Enter to Start", 0.30f, 0.0f, glm::vec3(2.25f, -3.75f, 0.0f));
            if ((g_frame_counter % 55) == i) {
                draw_text(&g_program, font_texture_id, "PLATFORM PERIL", 0.5f, 0.05f, glm::vec3(1.4f, -2.75f, 0.0f));
            }
        }
    }
    LOG(g_frame_counter);
    if (g_frame_counter < 500) {
        if (g_current_scene == g_levels[1]) {
            draw_text(&g_program, font_texture_id, "Level 1", 0.5f, 0.05f, glm::vec3(3.4f, -2.50f, 0.0f));
        }
        else if (g_current_scene == g_levels[2]) {
            draw_text(&g_program, font_texture_id, "Level 2", 0.5f, 0.05f, glm::vec3(3.4f, -2.50f, 0.0f));
        }
        else if (g_current_scene == g_levels[3]) {
            draw_text(&g_program, font_texture_id, "Final Level", 0.5f, 0.05f, glm::vec3(2.4f, -2.50f, 0.0f));
        }
        
    }
    
    if (!is_game_running) {
        draw_text(&g_program, font_texture_id, "YOU LOSE!", 0.5f, 0.10f, glm::vec3(2.5f, -3.0f, 0.0f)); // modify this so that if follows the player and doesnt just stay in the same position
    }
    else if (is_game_running && g_current_scene == g_levels[3] && final_lvl_completed) {
        draw_text(&g_program, font_texture_id, "YOU WIN!", 0.5f, 0.10f, glm::vec3(2.5f, -3.0f, 0.0f));
    }
 
    glUseProgram(g_program.programID);
    g_current_scene->render(&g_program);
    SDL_GL_SwapWindow(g_display_window);
}

void shutdown()
{
    SDL_Quit();
    
    delete g_level0;
    delete g_levelA;
    delete g_levelB;
    delete g_levelC;
    delete g_effects;
}

// ––––– DRIVER GAME LOOP ––––– //
int main(int argc, char* argv[])
{
    initialise();
    
    while (g_game_is_running)
    {
        process_input();
        update();
        
//        if (g_current_scene->m_state.next_scene_id >= 0) switch_to_scene(g_levels[g_current_scene->m_state.next_scene_id]);
        
        render();
    }
    
    shutdown();
    return 0;
}
