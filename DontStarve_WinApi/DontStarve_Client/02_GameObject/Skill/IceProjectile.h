#pragma once
#include "Projectile.h"
#include <unordered_map>

class Player;

class IceProjectile : public Projectile
{
public:
    IceProjectile();
    virtual ~IceProjectile() override;

    virtual void Init() override;
    virtual void Fire(float x, float y, float dirX, float dirY, int damage, float speed, float maxDistance) override;
    virtual void OnHit(GameObject* target) override;

private:
    struct SlowTintState {
        bool m_isSlowTintApplied;
        Gdiplus::Color m_originalTintColor;
        float remainingTime;
    };

    static std::unordered_map<Player*, SlowTintState> s_slowTintStates;
    static void ApplySlowTint(Player* player, float duration);
    static bool UpdateSlowTintCoroutine(Player* player, float deltaTime);
};
