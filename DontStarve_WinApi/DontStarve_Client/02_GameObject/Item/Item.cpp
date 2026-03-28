#include "99_Default/pch.h"
#include "Item.h"
#include "../Component/Transform/Transform.h"
#include "../Component/Sprite/SpriteRenderer.h"
#include "../Component/Collider/BoxCollider.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

Item::Item(GameObjectID id, const std::wstring& name, const std::wstring& desc,
           const std::wstring& baseDir, const std::wstring& imageName,
           float x, float y, float pivotX, float pivotY, Direction _dir, bool isActive, bool isInteractive)
    : GameObject(id, L"", imageName, isActive, isInteractive),
      m_itemName(name), m_description(desc),
      m_transform(nullptr), m_spriteRenderer(nullptr), m_collider(nullptr)
{
    m_type = GO_TYPE_ITEM;
    
    // Transform 컴포넌트 추가
    m_transform = AddComponent<Transform>();
    m_transform->SetPosition(x, y);
    m_transform->SetDirection(_dir);

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
    
    // 기본 콜라이더 추가 (아이템 픽업용)
    m_collider = AddComponent<BoxCollider>();
}

Item::~Item()
{
}

void Item::Init()
{
    GameObject::Init();
    
    // 컴포넌트 캐싱 (이미 생성자에서 했지만 안전을 위해)
    if (!m_transform) m_transform = GetComponent<Transform>();
    if (!m_spriteRenderer) m_spriteRenderer = GetComponent<SpriteRenderer>();
    if (!m_collider) m_collider = GetComponent<Collider>();
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
    m_collider = nullptr;
    
    GameObject::Release();
}

bool Item::OnInteraction(GameObject* obj)
{
    if (!IsEnabled() || !obj)
        return false;

    // 대부분의 아이템은 자신과 상호작용을 시도한 객체(주로 Player)에게 상호작용 처리를 넘김
    return obj->OnInteraction(this);
}
