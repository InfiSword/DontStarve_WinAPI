#include "99_Default/pch.h"
#include "IceProjectile.h"
#include "../Component/Collider/CircleCollider.h"
#include "../Component/Sprite/SpriteRenderer.h"
#include "../Component/Sprite/Sprite.h"
#include "../../03_Animation/Animator.h"
#include "../Entity/Player/Player.h"

std::unordered_map<Player*, IceProjectile::SlowTintState> IceProjectile::s_slowTintStates;

IceProjectile::IceProjectile()
	: Projectile(GOID_NONE, false)
{
	m_entityCollider = AddComponent<CircleCollider>();
}

IceProjectile::~IceProjectile() {}

void IceProjectile::Init()
{
	Projectile::Init();

    if (spriteRenderer) {
        spriteRenderer->SetLayer(LAYER_WORLD_EFFECT);
    }

	CircleCollider* circleCol = dynamic_cast<CircleCollider*>(m_entityCollider);
    if (circleCol) {
        circleCol->SetObjectCollider(0, 0, 28.0f);
    }

	if (m_animator) {
		// 아이스 하운드 발사체 애니메이션 (가로 시트 기준)
		m_animator->RegisterAnimation(0, DIR_NONE, L"Resource\\Objects\\Monster\\Hound\\Ice_Hound\\IceHound_ice_ProjectTile.png",
			0, 0, 7, 33, 0.5f, 0.5f, true, 0.03f);
	}
}

void IceProjectile::Fire(float x, float y, float dirX, float dirY, int damage, float speed, float maxDistance)
{
	Projectile::Fire(x, y, dirX, dirY, damage, speed, maxDistance);

}

void IceProjectile::OnHit(GameObject* target)
{
	Player* player = dynamic_cast<Player*>(target);
	if (player) {
		const float slowDuration = 2.5f;
		const float slowModifier = 0.55f;
		player->SetSlow(slowDuration, slowModifier);
		ApplySlowTint(player, slowDuration);
	}
	Projectile::OnHit(target);
}

void IceProjectile::ApplySlowTint(Player* player, float duration)
{
	if (!player) return;

	auto it = s_slowTintStates.find(player);
	if (it != s_slowTintStates.end()) {
		it->second.remainingTime = (std::max)(it->second.remainingTime, duration);
		return;
	}

	SpriteRenderer* sr = player->GetComponent<SpriteRenderer>();
	std::shared_ptr<Sprite> spriteHandle = sr ? sr->GetSpriteHandle() : nullptr;

	SlowTintState state;
	state.m_isSlowTintApplied = true;
	state.m_originalTintColor = spriteHandle ? spriteHandle->tintColor : Gdiplus::Color(255, 255, 255, 255);
	state.remainingTime = duration;

	if (spriteHandle) {
		spriteHandle->tintColor = Gdiplus::Color(255, 160, 220, 255);
	}

	s_slowTintStates[player] = state;

	player->StartCoroutine([player](float deltaTime) {
		return IceProjectile::UpdateSlowTintCoroutine(player, deltaTime);
		});
}

bool IceProjectile::UpdateSlowTintCoroutine(Player* player, float deltaTime)
{
	auto it = s_slowTintStates.find(player);
	if (it == s_slowTintStates.end()) return false;

	if (!player || !player->IsEnabled()) {
		s_slowTintStates.erase(it);
		return false;
	}

	it->second.remainingTime -= deltaTime;
	if (it->second.remainingTime > 0.0f) return true;

	SpriteRenderer* sr = player->GetComponent<SpriteRenderer>();
	std::shared_ptr<Sprite> spriteHandle = sr ? sr->GetSpriteHandle() : nullptr;
	if (spriteHandle && it->second.m_isSlowTintApplied) {
		spriteHandle->tintColor = it->second.m_originalTintColor;
	}

	s_slowTintStates.erase(it);
	return false;
}
