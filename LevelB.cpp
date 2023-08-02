#include "LevelB.h"
#include "Utility.h"
#define LOG(argument) std::cout << argument << '\n'


#define LEVEL_WIDTH 14
#define LEVEL_HEIGHT 8

unsigned int LEVELB_DATA[] =
{
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2,
    3, 0, 0, 0, 0, 0, 0, 0, 0, 1, 2, 2, 2, 2,
    3, 1, 1, 1, 1, 1, 1, 0, 1, 2, 2, 2, 2, 2,
    3, 2, 2, 2, 2, 2, 2, 0, 2, 2, 2, 2, 2, 2
};

LevelB::~LevelB()
{
    delete [] m_state.enemies;
    delete    m_state.player;
    delete    m_state.map;
    Mix_FreeChunk(m_state.jump_sfx);
    Mix_FreeMusic(m_state.bgm);
}

void LevelB::initialise()
{
    m_state.next_scene_id = -1;
    
    GLuint map_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final/SDLProject/assets/tileset3.png");
    m_state.map = new Map(LEVEL_WIDTH, LEVEL_HEIGHT, LEVELB_DATA, map_texture_id, 1.0f, 4, 1);

  
    // Code from main.cpp's initialise()
    /**
     George's Stuff
     */
    // Existing
    m_state.player = new Entity();
    m_state.player->set_entity_type(PLAYER);
    m_state.player->set_position(glm::vec3(3.5f, 5.0f, 0.0f));
    m_state.player->set_movement(glm::vec3(0.0f));
    m_state.player->m_speed = 1.0f;
    m_state.player->set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
    m_state.player->m_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final/SDLProject/assets/player.png");
    
    // Walking
    m_state.player->m_walking[m_state.player->LEFT]  = new int[4] { 5, 6, 7, 8     };
    m_state.player->m_walking[m_state.player->RIGHT] = new int[4] { 9, 10, 11,12   };
    m_state.player->m_walking[m_state.player->UP]    = new int[4] { 13, 14, 15, 16 };
    m_state.player->m_walking[m_state.player->DOWN]  = new int[4] { 1, 2, 3,  4    };

    m_state.player->m_animation_indices = m_state.player->m_walking[m_state.player->RIGHT];  // start George looking left
    m_state.player->m_animation_frames = 4;
    m_state.player->m_animation_index  = 0;
    m_state.player->m_animation_time   = 0.0f;
    m_state.player->m_animation_cols   = 4;
    m_state.player->m_animation_rows   = 4;
    m_state.player->set_height(0.8f);
    m_state.player->set_width(0.8f);
    
    // Jumping
    m_state.player->m_jumping_power = 5.0f;
    
    /**
     Enemies' stuff */
    GLuint enemy_texture_id = Utility::load_texture("/Users/chelsea/Desktop/Final/SDLProject/assets/ghost.png");
    
    m_state.enemies = new Entity[ENEMY_COUNT];
    
    for (int i = 0; i < ENEMY_COUNT; ++i) {
        m_state.enemies[i].set_entity_type(ENEMY);
        
        if (i == 0) {
            m_state.enemies[i].set_ai_type(GUARD);
            m_state.enemies[i].set_ai_state(IDLE);
            m_state.enemies[i].m_texture_id = enemy_texture_id;
            m_state.enemies[i].set_position(glm::vec3(5.0f, -5.0f, 0.0f));
            m_state.enemies[i].set_movement(glm::vec3(0.0f));
            m_state.enemies[i].set_speed(1.0f);
            m_state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
        }
        else if (i == 1){
            m_state.enemies[i].set_ai_type(JUMPER);
            m_state.enemies[i].set_ai_state(IDLE);
            m_state.enemies[i].set_jumping_power(3.0f);
            m_state.enemies[i].m_texture_id = enemy_texture_id;
            m_state.enemies[i].set_position(glm::vec3(10.0f, -3.0f, 0.0f));
            m_state.enemies[i].set_movement(glm::vec3(0.0f));
            m_state.enemies[i].set_speed(1.0f);
            m_state.enemies[i].set_acceleration(glm::vec3(0.0f, -9.81f, 0.0f));
        }
    }

    
    /**
     BGM and SFX
     */
    Mix_OpenAudio(44100, MIX_DEFAULT_FORMAT, 2, 4096);
    
    m_state.bgm = Mix_LoadMUS("/Users/chelsea/Desktop/Final/SDLProject/assets/bgm(games).mp3");
    Mix_PlayMusic(m_state.bgm, -1);
    Mix_VolumeMusic(MIX_MAX_VOLUME / 2.0f);
    LOG("IM HERRRREEE");
    
    m_state.jump_sfx = Mix_LoadWAV("/Users/chelsea/Desktop/Final/SDLProject/assets/jump.wav");
}

void LevelB::update(float delta_time)
{
    for (int i = 0; i < ENEMY_COUNT; ++i){
        if (m_state.enemies[i].get_ai_type() == JUMPER &&  m_state.enemies[i].m_collided_bottom == true) { m_state.enemies[i].ai_jumper(m_state.player);
        }
        m_state.player->update(delta_time, m_state.player, &(m_state.enemies[i]), ENEMY_COUNT, m_state.map);
        m_state.enemies[i].update(delta_time, m_state.player, m_state.player, 1, m_state.map);
       
    }
   
}

void LevelB::render(ShaderProgram *program)
{
    m_state.map->render(program);
    m_state.player->render(program);
    for (int i = 0; i < ENEMY_COUNT; ++i) m_state.enemies[i].render(program);
}
