#ifndef BOSS_H
#define BOSS_H

#include "../src/Audio.h"
#include "../src/DynamicModel.h"
#include "../src/Quads2D.h"

class Player; //If we use include, we get a circular dependency

class Boss
{
protected:
    DynamicModel&           m_dynamicModel;

    JPH::Mat44              m_modelMatrixHealthBar;

    Quads2D                 m_healthBar;
    Texture                 m_crosshairTexture;
    static constexpr uint   m_crosshairTextureSlot = 0;

    Quads2D                 m_healthBarBorder;
    Texture                 m_crosshairBorderTexture;
    static constexpr uint   m_crosshairBorderTextureSlot = 1;

    static int              m_numBosses;
    float                   m_health;
    const float             m_maxHealth;

public:
    explicit Boss(DynamicModel& model, float health);

    float GetHealth() const { return m_health; }

    //Did the player hit the boss?
    bool CheckForHit(Player& player, float deltaTime);

    void DrawHealthBar(Shader& shader, Renderer& renderer, const JPH::Mat44 &projMatrix, const std::string &textureUniformName);
};

#endif //BOSS_H
