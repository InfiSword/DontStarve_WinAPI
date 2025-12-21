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
	void SetInteractionTarget(GameObject* obj); // 상호작용 대상 설정
	
	// 클릭 위치에서 상호작용 가능한 오브젝트 찾아서 상호작용 시작
	void HandleClickInteraction(float worldX, float worldY);
	
	// 우클릭 처리 (이동 처리 또는 아이템 이동)
	void HandleRightClick(float worldX, float worldY);
	
	// 입력 처리 및 이동 처리
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
	
	// 상호작용 중인지 확인 (애니메이션 재생 중인지)
	bool IsInteracting() const { return m_currentInteractionTarget != nullptr && (m_state == PlayerState::PICKUP || m_state == PlayerState::CHOP); }
	
	// 상호작용 대상 오브젝트 반환 (애니메이션 재생 중인지)
	GameObject* GetInteractionTarget() const { return m_currentInteractionTarget; }

	void ToggleEquipItem(int slotIndex);
	//void OnAnimationEvent(int frameIndex, const std::wstring& eventName);

	virtual void Damaged(int damage) override;

	// 애니메이션 정의 제공 (Entity 오버라이드)
	virtual std::vector<AnimationDefinition> GetAnimationDefinitions() const override;

private:
	void UpdateAnimatorState();

	// TODO:
	// 나중에 플레이어가 많아지면, 클래스로 따로 관리해야 함
	//void SetCharacterSpritePath(GameObjectID characterID);

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
