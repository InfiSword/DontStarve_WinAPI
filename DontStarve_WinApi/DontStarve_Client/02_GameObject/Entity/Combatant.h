#pragma once
#include "Entity.h"

class BoxCollider;

// 전투 가능한 엔티티의 기반 클래스 (Player & Monster)
class Combatant : public Entity
{
protected:
    // 전투 관련
    int m_damage;
    float m_attackRange;
    float m_attackCooldown;
    int m_attackHitFrame;
    int m_attackBoxWidth;
    int m_attackBoxHeight;
    BoxCollider* m_attackCollider;
    GameObject* m_attackTarget;
    
    // 공격 박스 (방향별)
    struct AttackBox {
        int offsetX, offsetY, width, height;
        AttackBox() : offsetX(0), offsetY(0), width(0), height(0) {}
        AttackBox(int ox, int oy, int w, int h) : offsetX(ox), offsetY(oy), width(w), height(h) {}
    };
    AttackBox m_attackBoxDown;
    AttackBox m_attackBoxUp;
    AttackBox m_attackBoxLeft;
    AttackBox m_attackBoxRight;

public:
    Combatant(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
              const std::wstring& baseDir = L"", const std::wstring& imageName = L"",
              bool isActive = true, bool isInteractive = false,
              ColliderType colliderType = COLLIDER_BOX);
    virtual ~Combatant() override;

    virtual void Init() override;
    virtual void Release() override;

    // 공격 관련 공통 메서드
    void SetupAttackBox(int width, int height, int offsetX = 0, int offsetY = 0);
    void SetAllAttackBoxes(int width, int height,
                          int downOffsetX, int downOffsetY,
                          int upOffsetX, int upOffsetY,
                          int leftOffsetX, int leftOffsetY,
                          int rightOffsetX, int rightOffsetY);
    
    virtual void ProcessAttackHit(int damage);
    virtual void OnAttackEnd();
    
    // 타겟 관리
    void SetAttackTarget(GameObject* target) { m_attackTarget = target; }
    GameObject* GetAttackTarget() const { return m_attackTarget; }

protected:
    void UpdateAttackBoxByDirection(Direction dir);
    void ApplyAttackDamageToTarget(int damage);
};
