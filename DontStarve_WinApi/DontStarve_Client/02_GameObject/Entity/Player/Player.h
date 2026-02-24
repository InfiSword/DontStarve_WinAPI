#pragma once
#include "../Entity.h"
#include "../../Item/Item.h"

class Inventory;
class ResourceManager;
class BoxCollider;

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

	Item* GetEquippedItem() const { return m_equippedItem; }
	int GetEquippedSlotIndex() const { return m_equippedSlotIndex; };

	void ToggleEquipItem(int slotIndex);

	virtual void Damaged(int damage) override;

	int GetHp() const { return hp; }
	int GetMaxHp() const { return maxHp; }

	void Heal(int amount);

	virtual void RenderDebugOverlay() override;

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
	void OnAttackEnd();

	Inventory* m_inventory;
	GameObject* m_pendingInteractionTarget;  // 이동 후 상호작용할 대상
	GameObject* m_activeInteractionTarget;   // 현재 상호작용 중인 대상 (FinalizePickup용)
	
	GameObject* m_attackTarget;               // 공격할 몬스터 (클릭 시 설정, 사거리 도달 시 ATTACK)
	BoxCollider* m_attackCollider;            // 공격 판정용 (ATTACK 상태 6프레임 시만 활성)

	PlayerState m_state;
	int hp;
	int maxHp;
	bool isMoveToGoal;
	float m_playerSpeed;
	Gdiplus::PointF m_targetWorldPos;
	float m_stopThreshold;

	int m_equippedSlotIndex;
	Item* m_equippedItem;
};
