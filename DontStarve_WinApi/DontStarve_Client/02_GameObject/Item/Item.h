#pragma once
#include "../GameObject.h"

class Transform;
class SpriteRenderer;
class Collider;

class Item : public GameObject
{
protected:
    std::wstring m_itemName;
    std::wstring m_description;
    
    Transform* m_transform;
    SpriteRenderer* m_spriteRenderer;

public:
    Item(GameObjectID id, const std::wstring& name, const std::wstring& desc,
         const std::wstring& baseDir = L"", const std::wstring& imageName = L"",
         float x = 0, float y = 0, float pivotX = 0.5f, float pivotY = 0.5f,
         Direction _dir = DIR_DOWN, bool isActive = true, bool isInteractive = true);
    virtual ~Item();

    virtual void Init() override;
    virtual void Render() override;
    virtual void Release() override;

    virtual bool OnInteraction(GameObject* obj) override;

    // Item은 데미지를 받지 않으므로 빈 구현
    virtual void Damaged(int damage) override { }

    const std::wstring& GetItemName() const { return m_itemName; }
    const std::wstring& GetDescription() const { return m_description; }
    
    // Transform 접근자 
    Transform* GetTransform() const { return m_transform; }
    SpriteRenderer* GetSpriteRenderer() const { return m_spriteRenderer; }
};
