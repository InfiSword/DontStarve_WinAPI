#pragma once
#include "../Entity.h"
#include "../../Item/Tool/Tool.h"

class Inventory;
class ResourceManager;
class BoxCollider;

enum PlayerState {
	IDLE,               // 대기 상태
	WALK,               // 걷기 상태
	MOVING_TO_TARGET,   // 클릭한 위치로 이동 중
	PICKUP,             // 오브젝트를 집는 중
	CHOP,
	MINE,               // 곡괭이로 채광 중
	ATTACK,
	HIT,
	COUNT,
};

class Player : public Entity
{
public:
	Player(float x, float y, GameObjectID characterID, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
	virtual ~Player();

	virtual void Init() override;
	virtual void LateInit() override;
	virtual void Update(float deltaTime) override;
	virtual void LateUpdate() override;
	virtual void Release() override;

	void SetTargetPosition(float worldX, float worldY);
	void HandleRightClick(float worldX, float worldY);
	void HandleMovement();
	void FinalizePickup();
	virtual bool OnInteraction(GameObject* obj) override;

	Inventory* GetInventory() { return m_inventory; }

	PlayerState GetPlayerState() const { return (PlayerState)m_state; }

	Tool* GetEquippedItem() const { return m_equippedItem; }
	int GetEquippedSlotIndex() const { return m_equippedSlotIndex; };

	void ToggleEquipItem(int slotIndex);

	virtual void Damaged(int damage) override;

	void Heal(int amount);

private:
	// singleTarget=true: 첫 번째 몬스터만 피해, false: 콜라이더와 겹치는 모든 몬스터 피해
	void ApplyAttackDamage(int damage, bool singleTarget = true);

	void TryStartInteraction(float worldX, float worldY);
	void UpdateAnimatorState();
	
	// 애니메이션 이벤트 핸들러 함수들
	void OnPickupEnd();
	void OnChopHit();
	void OnChopEnd();
	void OnMineHit();
	void OnMineEnd();
	void OnAttackHit();
	void OnAttackEnd();

	Inventory* m_inventory;
	GameObject* m_pendingInteractionTarget;  // 이동 후 상호작용할 대상
	GameObject* m_activeInteractionTarget;   // 현재 상호작용 중인 대상 (FinalizePickup용)
	
	GameObject* m_attackTarget;               // 공격할 몬스터 (클릭 시 설정, 사거리 도달 시 ATTACK)
	BoxCollider* m_attackCollider;            // 공격 판정용 (ATTACK 상태 16프레임 시만 활성)

	bool isMoveToGoal;
	float m_playerSpeed;
	Gdiplus::PointF m_targetWorldPos;
	float m_stopThreshold;
	float m_attackRange;
	int m_damage;

	int m_equippedSlotIndex;
	Tool* m_equippedItem;
};
