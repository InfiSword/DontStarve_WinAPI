#pragma once
#include "../GameObject/Entity.h"
#include "../GameObject/Item.h"

class Inventory;

class Player : public Entity<PlayerState>
{
public:
	Player(float x, float y, GameObjectID characterID, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
	virtual ~Player();

	virtual void Init() override;
	virtual void LateInit() override;
	virtual void Update(float deltaTime) override;
	virtual void LateUpdate() override;
	virtual void Render(Gdiplus::Graphics* pGraphics) override; // RenderManager에서 처리
	virtual void Release() override;

	// Unity Animator 스타일로 최적화된 애니메이션 메소드들
	virtual void UpdateAnimation(float deltaTime) override;
	virtual Gdiplus::Bitmap* GetBitmap() const override;

	void SetTargetPosition(float worldX, float worldY);
	void SetInteractionTarget(GameObject* obj); // 상호작용 대상 설정
	
	// 클릭 위치에서 상호작용 가능한 오브젝트 찾기 및 상호작용 시작
	void HandleClickInteraction(float worldX, float worldY);
	
	// 우클릭 처리 (이동 취소 또는 카메라 이동)
	void HandleRightClick(float worldX, float worldY);
	
	// 입력 처리 및 이동 관리
	void HandleMovement();
	virtual void OnInteraction(GameObject* obj) override;
	void FinalizeInteraction();

	Inventory* GetInventory() { return m_inventory; }
	void SetInventory(std::vector<Gdiplus::RectF> rectSize);

	PlayerState GetPlayerState() const { return m_state; }
	void SetPlayerState(PlayerState newState) { m_state = newState; }

	float GetInteractionRadius() const { return m_stopThreshold + 10.0f; }
	std::shared_ptr<Item> GetEquippedItem() const { return m_equippedItem; }
	int GetEquippedSlotIndex() const { return m_equippedSlotIndex; };
	
	// 상호작용 중인지 확인 (렌더링 순서 조정용)
	bool IsInteracting() const { return m_currentInteractionTarget != nullptr && (m_state == PlayerState::PICKUP || m_state == PlayerState::CHOP); }
	
	// 상호작용 대상 오브젝트 가져오기 (렌더링 순서 조정용)
	GameObject* GetInteractionTarget() const { return m_currentInteractionTarget; }

	void ToggleEquipItem(int slotIndex);
	void OnAnimationEvent(int frameIndex, const std::wstring& eventName);

private:
	// Unity Animator 스타일로 변경 - 애니메이션 등록
	void RegisterAllAnimations();

	// Unity Animator 스타일 - 상태만 설정하면 자동으로 애니메이션 선택
	void UpdateAnimatorState();
	
	// 캐릭터 ID에 따른 스프라이트 경로 설정
	void SetCharacterSpritePath(GameObjectID characterID);

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
	std::shared_ptr<Item> m_equippedItem;
};