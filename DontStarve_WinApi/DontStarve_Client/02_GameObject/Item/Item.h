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
	Collider* m_itemCollider;

public:
    Item(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
           const std::wstring& baseDir = L"",
           const std::wstring& imageName = L"",
           ColliderType colliderType = COLLIDER_BOX, bool isActive = true, bool isInteractive = true);
    virtual ~Item();

    virtual void Init() override;
    virtual void Render() override;
    virtual void Release() override;

    virtual bool OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override { }

	virtual Gdiplus::RectF GetBounds() override;

	// 객체의 메인(몸통) 콜라이더를 반환하도록 오버라이딩
	virtual Collider* GetMainCollider() const override {
		return m_itemCollider;
	}

	virtual void SetMainCollider(Collider* col) override {
		if (!m_itemCollider) m_itemCollider = col;
	}

    const std::wstring& GetItemName() const { return m_itemName; }
    const std::wstring& GetDescription() const { return m_description; }
};
