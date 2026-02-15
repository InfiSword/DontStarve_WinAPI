#pragma once
#include "../GameObject.h"

class Transform;
class SpriteRenderer;
class ResourceManager;

class Item : public GameObject
{
public:
	static void RegisterResources(ResourceManager* rm);

protected:
    // Component 캐싱 (최적화)
    Transform* transform;
    SpriteRenderer* spriteRenderer;
    std::wstring m_description;  // 해당 아이템 설명

public:
    Item(GameObjectType type, GameObjectID id, const std::wstring& name, const std::wstring& desc,
        const std::wstring& resourcePath, const std::wstring& imagePath,
        float x = 0.0f, float y = 0.0f, float pivotX = 0.5f, float pivotY = 0.5f,
        Direction dir = DIR_DOWN, bool isActive = true, bool isInteractive = false);
    virtual ~Item();
    
    virtual void Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Release() override;
    bool CanInteract() const override { return true; }

    // inline 함수
    inline const std::wstring& GetDescription() const { return m_description; }
};
