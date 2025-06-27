#include "Boss.h"
#include "../src/Player.h"

int Boss::m_numBosses = 0;

Boss::Boss(DynamicModel &model, float health) :
    m_dynamicModel(model),
    m_modelMatrixHealthBar(JPH::Mat44::sIdentity()),
    m_crosshairTexture("../resources/images/red.jpeg", Texture::TextureType::diffuse, false, false),
    m_maxHealth(health),
    m_crosshairBorderTexture("../resources/images/white.jpeg", Texture::TextureType::diffuse, false, false)
{
    m_numBosses++;
    m_health = health;

    float yPosHealthBar = 10.f + ((static_cast<float>(m_numBosses) - 1.f) * -2.f);
    m_healthBar.Init(-12.5, yPosHealthBar, -15, 25, 1);
    m_healthBarBorder.Init(-12.625, yPosHealthBar - 0.125f, -15, 25.25, 1.25);
}

bool Boss::CheckForHit(Player &player, float deltaTime)
{
    if (m_health < 0.005)
    {
        m_health = -1;
        return false;
    }

    bool hit = m_dynamicModel.CastRayAgainstAllMeshes(player.GetPosition(), player.GetAimVector());

    if (hit)
    {
        m_health -= player.GetDamageAmountForCurrrentGun() * deltaTime;
        float healthPortion = m_health / m_maxHealth;

        if (healthPortion < 0.005) [[unlikely]]
            m_modelMatrixHealthBar = JPH::Mat44::sScale({0, 1, 1});
        else
            m_modelMatrixHealthBar = JPH::Mat44::sScale({healthPortion, 1, 1});
    }

    return hit;
}

void Boss::DrawHealthBar(Shader& shader, Renderer& renderer, const JPH::Mat44 &projMatrix, const std::string &textureUniformName)
{
    m_crosshairBorderTexture.Bind(m_crosshairBorderTextureSlot);
    shader.SetUniform(textureUniformName, m_crosshairBorderTextureSlot);
    m_healthBarBorder.Draw(shader, renderer, projMatrix);

    m_crosshairTexture.Bind(m_crosshairTextureSlot);
    shader.SetUniform(textureUniformName, m_crosshairTextureSlot);
    m_healthBar.Draw(shader, renderer, projMatrix, m_modelMatrixHealthBar);
}
