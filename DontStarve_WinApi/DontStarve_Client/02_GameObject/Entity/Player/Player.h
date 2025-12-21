#pragma once
#include  "../Entity.h"
#include "../../Item/Item.h"

class Inventory;

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
	void SetInteractionTarget(GameObject* obj); 
	void HandleClickInteraction(float worldX, float worldY);
	void HandleRightClick(float worldX, float worldY);
	
	void HandleMovement();
	virtual void OnInteraction(GameObject* obj) override;
	void FinalizeInteraction();

	Inventory* GetInventory() { return m_inventory; }
	void SetInventory(std::vector<Gdiplus::RectF> rectSize);

	PlayerState GetPlayerState() const { return (PlayerState)m_state; }
	void SetPlayerState(PlayerState newState) { m_state = newState; }

	float GetInteractionRadius() const { return m_stopThreshold + 10.0f; }
	Item* GetEquippedItem() const { return m_equippedItem; }
	int GetEquippedSlotIndex() const { return m_equippedSlotIndex; };
	
	// 상호작용 중인지 확인
	bool IsInteracting() const { return m_currentInteractionTarget != nullptr && (m_state == PlayerState::PICKUP || m_state == PlayerState::CHOP); }
	
	// 상호작용 대상 오브젝트 반환
	GameObject* GetInteractionTarget() const { return m_currentInteractionTarget; }

	void ToggleEquipItem(int slotIndex);
	//void OnAnimationEvent(int frameIndex, const std::wstring& eventName);

	virtual void Damaged(int damage) override;

	virtual std::vector<AnimationDefinition> GetAnimationDefinitions() const override;

private:
	void UpdateAnimatorState();

	Inventory* m_inventory;
	GameObject* m_currentInteractionTarget;

	PlayerState m_state;
	int hp;
	int maxHp;
	bool isMoveToGoal;

	int correctValue;

	float m_playerSpeed;
	Gdiplus::PointF m_targetWorldPos;
	float m_stopThreshold;

	int m_equippedSlotIndex;
	Item* m_equippedItem;
};
