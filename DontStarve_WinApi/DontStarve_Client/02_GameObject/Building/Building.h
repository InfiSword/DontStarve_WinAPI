#pragma once
#include "../Entity/Entity.h"

// Building - 건물을 의미하는 기본 클래스
class Building : public Entity
{
protected:
    int m_hp;
    int m_maxHp;
    BuildingState m_buildingState;  // 건물 상태
    
public:
    Building(GameObjectID id, float x, float y, float pivotX, float pivotY, 
        Direction _dir, const std::wstring& resourcePath = L"",
             const std::wstring& imageName = L"", int hp = 100);
    virtual ~Building();

    // GameObject 기본 초기화
    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;
    
    virtual void Damaged(int damage) override;
    virtual void SetTimeState(BuildingState buildingState);
    virtual BuildingState GetTimeState() const;

    // HP 관련
    int GetHP() const { return m_hp; }
    int GetMaxHP() const { return m_maxHp; }
    bool IsDestroyed() const { return m_hp <= 0; }
}; 
