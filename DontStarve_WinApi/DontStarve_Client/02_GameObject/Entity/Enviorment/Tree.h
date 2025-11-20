#pragma once
#include "../Entity.h"

class Tree : public Entity {
public:
    Tree(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Tree();

public:
    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    virtual void OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override;
    
protected:
    std::wstring tree_Grade;
    TreeState m_state;
    int m_hp;
    int maxHp;
    float m_hitAnimTimer;
    
}; 
