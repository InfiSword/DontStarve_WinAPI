#pragma once
#include "../Entity.h"
#include "../../Item/Item.h"

class Inventory;
class ResourceManager;

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
	void TryStartInteraction(float worldX, float worldY);
	void FinalizePickup();
	virtual bool OnInteraction(GameObject* obj) override;

	Inventory* GetInventory() { return m_inventory; }

	PlayerState GetPlayerState() const { return (PlayerState)m_state; }

	Item* GetEquippedItem() const { return m_equippedItem; }
	int GetEquippedSlotIndex() const { return m_equippedSlotIndex; };

	void ToggleEquipItem(int slotIndex);

	virtual void Damaged(int damage) override;

private:
	void UpdateAnimatorState();
	void SetDirectionToward(float dx, float dy);
	// 목표까지의 거리로 도착 여부 판정 (이동/상호작용 공통)
	bool IsArrivedAtTarget(float distance, float moveSpeedThisFrame = 0.f) const;

	// 상호작용 가능 여부만 확인 (상태 변경 없이)
	bool CanInteractWith(GameObject* obj) const;
	
	// 애니메이션 이벤트 핸들러 함수들
	void OnPickupEnd();
	void OnChopHit();
	void OnChopEnd();

	Inventory* m_inventory;
	GameObject* m_pendingInteractionTarget;  // 이동 후 상호작용할 대상
	GameObject* m_activeInteractionTarget;   // 현재 상호작용 중인 대상 (FinalizePickup용)

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
