#pragma once
#include "../Combatant.h"
#include "../../Item/Tool/Tool.h"
#include "../../../01_Manager/GameProgressManager/GameProgressManager.h"

class Inventory;

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

class Player : public Combatant
{
public:
	Player(float x, float y, GameObjectID characterID, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
	virtual ~Player() override;

	virtual void Init() override;
	virtual void LateInit() override;
	virtual void Update(float deltaTime) override;
	virtual void LateUpdate() override;
	virtual void RenderDebugOverlay() override;

	void SetTargetPosition(float worldX, float worldY);
	void HandleRightClick(float worldX, float worldY);
	void HandleMovement();
	void FinalizePickup();
	virtual bool OnInteraction(GameObject* obj) override;

	void SetInputEnabled(bool enabled) { m_bInputEnabled = enabled; }
	bool IsInputEnabled() const { return m_bInputEnabled; }

	Inventory* GetInventory() { return m_inventory; }

	PlayerState GetPlayerState() const { return (PlayerState)m_state; }

	Tool* GetEquippedItem() const { return m_equippedItem; }
	int GetEquippedSlotIndex() const { return m_equippedSlotIndex; };

	void ToggleEquipItem(int slotIndex);

	virtual void Damaged(int damage) override;
	virtual void Die() override;

	void Heal(int amount);

	// 상태 저장/복원 (씬 전환용)
	PlayerStateSnapshot SaveState() const;
	void RestoreState(const PlayerStateSnapshot& snapshot);

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
	virtual void OnAttackEnd() override;

	Inventory* m_inventory;
	GameObject* m_pendingInteractionTarget;  // 이동 후 상호작용할 대상
	GameObject* m_activeInteractionTarget;   // 현재 상호작용 중인 대상 (FinalizePickup용)
	
	// m_attackTarget, m_attackCollider는 Combatant에서 상속

	bool isMoveToGoal;
	float m_playerSpeed;
	Gdiplus::PointF m_targetWorldPos;
	float m_stopThreshold;
	// m_attackRange, m_damage는 Combatant에서 상속

	int m_equippedSlotIndex;
	Tool* m_equippedItem;
	bool m_bInputEnabled;
};
