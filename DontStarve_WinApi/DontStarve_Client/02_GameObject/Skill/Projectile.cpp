#include "99_Default/pch.h"
#include "Projectile.h"
#include "../Component/Transform/Transform.h"
#include "../Component/Collider/Collider.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../Entity/Player/Player.h"
#include "../../03_Animation/Animator.h"

Projectile::Projectile(GameObjectID id, bool isActive)
    : Entity(id, 0, 0, 0.5f, 0.5f, DIR_NONE, L"", L"", COLLIDER_CIRCLE, isActive, false),
	m_speed(0.0f), m_maxDistance(0.0f), m_traveledDistance(0.0f), m_damage(0), m_fireDirX(0.0f), m_fireDirY(0.0f)
{
    m_type = GO_TYPE_NONE;
}

Projectile::~Projectile() {}

void Projectile::Init()
{
    Entity::Init();
    m_animator = AddComponent<Animator>(m_spriteRenderer);
}

void Projectile::Update(float deltaTime)
{
    if (!IsEnabled()) return;

    GameObject::Update(deltaTime);

    float moveDist = m_speed * deltaTime;
    if (m_transform)
    {
        m_transform->SetPosition(m_transform->GetX() + m_fireDirX * moveDist, m_transform->GetY() + m_fireDirY * moveDist);
    }
    m_traveledDistance += moveDist;

    if (m_traveledDistance >= m_maxDistance)
    {
        SetActive(false);
        return;
    }

    // 플레이어 충돌 체크
    Player* player = dynamic_cast<Player*>(ObjectManager::GetInstance()->GetPlayer());
    if (player && player->IsEnabled())
    {
        Collider* playerCollider = player->GetComponent<Collider>();
        if (m_entityCollider && playerCollider && m_entityCollider->IntersectsCollider(playerCollider))
        {
            OnHit(player);
        }
    }
}

void Projectile::Fire(float x, float y, float dirX, float dirY, int damage, float speed, float maxDistance)
{
 
	Transform* trans = GetComponent<Transform>();
    if (trans)
    {
        trans->SetPosition(x, y);
        // 위치 변경에 따른 그리드 갱신 (Warp 처리)
        // ObjectManager::GetInstance()->UpdateObjectGridCell(this);
    }

    // 방향 정규화
    float length = sqrtf(dirX * dirX + dirY * dirY);
    if (length > 0.0f) {
        m_fireDirX = dirX / length;
        m_fireDirY = dirY / length;
    }
    else {
        m_fireDirX = 1.0f;
        m_fireDirY = 0.0f;
    }

    m_damage = damage;
    m_speed = speed;
    m_maxDistance = maxDistance;
    m_traveledDistance = 0.0f;

    if (m_animator)
    {
        m_animator->SetState(0, DIR_NONE, true);
    }

    SetActive(true);
}

void Projectile::OnHit(GameObject* target)
{
    Player* player = dynamic_cast<Player*>(target);
    if (player)
    {
        player->Damaged(m_damage);
    }

    SetActive(false);
}
