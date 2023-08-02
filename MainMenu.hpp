#include "Scene.h"

class Level0 : public Scene {
public:
    int ENEMY_COUNT = 1;
    
    ~Level0();
    
    void initialise() override;
    void update(float delta_time) override;
    void render(ShaderProgram *program) override;
};

