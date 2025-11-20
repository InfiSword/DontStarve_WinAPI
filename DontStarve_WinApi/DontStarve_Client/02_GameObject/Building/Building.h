#pragma once
#include "../Entity/Entity.h"

// Building은 건물의 기본 클래스
class Building : public Entity
{
protected:
    int m_hp;
    int m_maxHp;
    BuildingState m_buildingState;  // Entity의 m_state와 동기화되는 건물상태 관리
    
public:
    Building(GameObjectID id, float x, float y, float pivotX, float pivotY, 
        Direction _dir, const std::wstring& resourcePath = L"",
             const std::wstring& imageName = L"", int hp = 100);
    virtual ~Building();

    // GameObject 인터페이스 구현
    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;
    
    virtual void Damaged(int damage) override;
    virtual void SetTimeState(BuildingState buildingState);
    virtual BuildingState GetTimeState() const;

    // HP 관리
    int GetHP() const { return m_hp; }
    int GetMaxHP() const { return m_maxHp; }
    bool IsDestroyed() const { return m_hp <= 0; }
}; 