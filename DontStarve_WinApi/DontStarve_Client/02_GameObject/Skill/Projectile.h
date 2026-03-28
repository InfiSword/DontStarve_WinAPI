#pragma once
#include "../Entity/Entity.h"

class Projectile : public Entity
{
public:
    Projectile(GameObjectID id = GOID_NONE, bool isActive = false);
    virtual ~Projectile() override;

    virtual void Init() override;
    virtual void Update(float deltaTime) override;

    virtual void Fire(float x, float y, float dirX, float dirY, int damage, float speed, float maxDistance);

protected:
    float m_speed;
    float m_maxDistance;
    float m_traveledDistance;
    int m_damage;

	float m_fireDirX;
    float m_fireDirY;

    virtual void OnHit(GameObject* target);
};
