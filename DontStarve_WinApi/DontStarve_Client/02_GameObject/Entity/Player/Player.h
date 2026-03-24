#pragma once
#include "../Combatant.h"
#include "../../Item/Tool/Tool.h"
#include "../../../01_Manager/GameProgressManager/GameProgressManager.h"

class Inventory;

enum PlayerState {
	IDLE = (int)CombatantState::IDLE,
	WALK = (int)CombatantState::WALK,
	ATTACK = (int)CombatantState::ATTACK,
	HIT = (int)CombatantState::HIT,
	DEATH = (int)CombatantState::DEATH,

	PICKUP = (int)CombatantState::MAX_COMMON,
	CHOP,
	MINE,
	MOVING_TO_TARGET,
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
	virtual void Release() override;
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
	GameObject* m_activeInteractionTarget;   // 현재 상호작용 중인 대상

	bool isMoveToGoal;
	float m_playerSpeed;
	Gdiplus::PointF m_targetWorldPos;
	float m_stopThreshold;

	int m_equippedSlotIndex;
	Tool* m_equippedItem;
	bool m_bInputEnabled;
};
