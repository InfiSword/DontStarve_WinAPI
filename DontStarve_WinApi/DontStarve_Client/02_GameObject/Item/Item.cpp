#include "99_Default/pch.h"
#include "Item.h"
#include "../Component/Transform/Transform.h"
#include "../Component/Sprite/SpriteRenderer.h"
#include "../Component/Collider/BoxCollider.h"
#include "../Component/Collider/CircleCollider.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

Item::Item(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
	const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType, bool isActive, bool isInteractive)
	: GameObject(id, x, y, pivotX, pivotY, dir, baseDir, imageName, colliderType, isActive, isInteractive),
	m_transform(nullptr), m_spriteRenderer(nullptr), m_itemCollider(nullptr)
{
	m_type = GO_TYPE_ITEM;

	// Transform 컴포넌트 추가
	m_transform = AddComponent<Transform>();
	m_transform->SetPosition(x, y);
	m_transform->SetDirection(dir);

	// SpriteRenderer 컴포넌트 추가
	m_spriteRenderer = AddComponent<SpriteRenderer>();
	m_spriteRenderer->SetLayer(LAYER_WORLD_OBJECT);

	if (!imageName.empty())
	{
		ResourceManager* pRM = ResourceManager::GetInstance();
		std::wstring fullPath = ResourcePathUtils::BuildResourcePath(baseDir, imageName);
		if (!fullPath.empty()) {
			// 로드 시점에 피벗 전달
			if (auto sprite = pRM->LoadSprite(fullPath, { pivotX, pivotY })) {
				m_spriteRenderer->SetSprite(sprite);
			}
		}
	}

	m_itemName = DataTable::GetItemInfo(id)->name;
	m_description = DataTable::GetItemInfo(id)->desc;
}

Item::~Item()
{
}

void Item::Init()
{
	GameObject::Init();

	if (!m_transform) m_transform = GetComponent<Transform>();
	if (!m_spriteRenderer) m_spriteRenderer = GetComponent<SpriteRenderer>();

	// 콜라이더 캐싱 한 번 더 체크
	if (!m_itemCollider) {
		m_itemCollider = GetComponent<BoxCollider>();
		if (!m_itemCollider) m_itemCollider = GetComponent<CircleCollider>();
	}
}

void Item::Render()
{
	if (!IsEnabled() || !m_transform) return;

	if (m_spriteRenderer && m_spriteRenderer->IsEnabled()) {
		m_spriteRenderer->Render();
	}
}

void Item::Release()
{
	m_transform = nullptr;
	m_spriteRenderer = nullptr;
	m_itemCollider = nullptr;

	GameObject::Release();
}

bool Item::OnInteraction(GameObject* obj)
{
	if (!IsEnabled() || !obj)
		return false;

	return obj->OnInteraction(this);
}

Gdiplus::RectF Item::GetBounds()
{
	if (!m_isBoundsDirty) return m_cachedBounds;

	if (!m_transform) {
		m_cachedBounds = { 0,0,0,0 };
		m_isBoundsDirty = false;
		return m_cachedBounds;
	}

	float w = 32, h = 32, px = 0.5f, py = 0.5f;

	if (auto sprite = m_spriteRenderer->GetSpriteHandle()) {
		w = sprite->sourceRect.Width; h = sprite->sourceRect.Height;
		px = sprite->pivot.X; py = sprite->pivot.Y;
	}

	w *= m_transform->GetScaleX(); h *= m_transform->GetScaleY();

	m_cachedBounds = { m_transform->GetX() - w * px, m_transform->GetY() - h * py, w, h };
	m_isBoundsDirty = false;
	return m_cachedBounds;
}
