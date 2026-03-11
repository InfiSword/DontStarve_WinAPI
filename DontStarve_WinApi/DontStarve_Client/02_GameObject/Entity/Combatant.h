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
    virtual ~Combatant();

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
    
    // Getter/Setter
    int GetDamage() const { return m_damage; }
    void SetDamage(int damage) { m_damage = damage; }
    float GetAttackRange() const { return m_attackRange; }
    void SetAttackRange(float range) { m_attackRange = range; }
    float GetAttackCooldown() const { return m_attackCooldown; }
    void SetAttackCooldown(float cooldown) { m_attackCooldown = cooldown; }
    int GetAttackHitFrame() const { return m_attackHitFrame; }
    void SetAttackHitFrame(int frame) { m_attackHitFrame = frame; }
    int GetAttackBoxWidth() const { return m_attackBoxWidth; }
    void SetAttackBoxWidth(int width) { m_attackBoxWidth = width; }
    int GetAttackBoxHeight() const { return m_attackBoxHeight; }
    void SetAttackBoxHeight(int height) { m_attackBoxHeight = height; }

protected:
    void UpdateAttackBoxByDirection(Direction dir);
    void ApplyAttackDamageToTarget(int damage);
};
