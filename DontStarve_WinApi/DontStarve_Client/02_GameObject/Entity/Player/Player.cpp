#include "../../../99_Default/pch.h"
#include "Player.h"

#include "../../../01_Manager/InputManager/InputManager.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../01_Manager/ColliderManager/ColliderManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"

#include "../../../02_GameObject/UI/Inventory.h"

#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../../03_Animation/SpriteSheet.h"

#include "../../Item/Ingredient.h"
#include "../../Item/Tool/Tool.h"

namespace {
	constexpr float PLAYER_INTERACTION_SORT_OFFSET = 0.5f;
}

Player::Player(float x, float y, GameObjectID characterID, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_PLAYER, characterID, x, y, 0.5f, 1.0f, DIR_DOWN, resourcePath, imageName, true, true), 
	  hp(100), maxHp(100), m_playerSpeed(300.f), m_stopThreshold(10), 
	  m_equippedSlotIndex(-1), m_equippedItem(nullptr), m_inventory(nullptr), m_currentInteractionTarget(nullptr), m_state(PlayerState::IDLE), isMoveToGoal(false)
{
}

Player::~Player() { Release(); }

void Player::Init()
{
	// 부모 클래스(Entity)의 Init 호출 - Animator가 자동으로 생성됨
	Entity::Init();

	// Animator 생성 및 애니메이션 등록 (AnimationDefinition 패턴 제거)
	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();

		// IDLE
		m_animator->RegisterAnimation((int)PlayerState::IDLE, DIR_DOWN,  pRM->BuildResourcePath(m_resourcePath, L"Idle", L"Wilson_Idle_Down.png"),
			126, 189, 7, 64, 0.03f, m_pivotX, m_pivotY, true);
		m_animator->RegisterAnimation((int)PlayerState::IDLE, DIR_UP,    pRM->BuildResourcePath(m_resourcePath, L"Idle", L"Wilson_Idle_Up.png"),
			128, 193, 7, 64, 0.03f, m_pivotX, m_pivotY, true);
		m_animator->RegisterAnimation((int)PlayerState::IDLE, DIR_LEFT,  pRM->BuildResourcePath(m_resourcePath, L"Idle", L"Wilson_Idle_Side.png"),
			135, 194, 7, 64, 0.03f, m_pivotX, m_pivotY, true);
		m_animator->RegisterAnimation((int)PlayerState::IDLE, DIR_RIGHT, pRM->BuildResourcePath(m_resourcePath, L"Idle", L"Wilson_Idle_Side.png"),
			135, 194, 7, 64, 0.03f, m_pivotX, m_pivotY, true);

		// WALK(RUN)
		m_animator->RegisterAnimation((int)PlayerState::WALK, DIR_DOWN,  pRM->BuildResourcePath(m_resourcePath, L"Run", L"Wilson_Run_Down.png"),
			139, 226, 6, 33, 0.03f, m_pivotX, m_pivotY, true);
		m_animator->RegisterAnimation((int)PlayerState::WALK, DIR_UP,    pRM->BuildResourcePath(m_resourcePath, L"Run", L"Wilson_Run_Up.png"),
			133, 231, 6, 33, 0.03f, m_pivotX, m_pivotY, true);
		m_animator->RegisterAnimation((int)PlayerState::WALK, DIR_LEFT,  pRM->BuildResourcePath(m_resourcePath, L"Run", L"Wilson_Run_Side.png"),
			141, 226, 6, 33, 0.03f, m_pivotX, m_pivotY, true);
		m_animator->RegisterAnimation((int)PlayerState::WALK, DIR_RIGHT, pRM->BuildResourcePath(m_resourcePath, L"Run", L"Wilson_Run_Side.png"),
			141, 226, 6, 33, 0.03f, m_pivotX, m_pivotY, true);

		// PICKUP
		std::wstring pickupPath = pRM->BuildResourcePath(m_resourcePath, L"Interact", L"Interact_wilson_pickup_pst_down.png");
		for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)PlayerState::PICKUP, (Direction)dir, pickupPath,
				127, 201, 6, 20, 0.03f, m_pivotX, m_pivotY, false);
		}

		// CHOP (이벤트 포함)
		std::map<int, std::wstring> chopEvents = { {4, L"chop_hit"} };
		std::wstring chopPath = pRM->BuildResourcePath(m_resourcePath, L"Axe", L"axe_wilson_chop_loop_down.png");
		for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)PlayerState::CHOP, (Direction)dir, chopPath,
				284, 248, 6, 54, 0.03f, m_pivotX + 0.1f, m_pivotY, false, chopEvents);
		}
	}
	
	m_inventory = new Inventory();
	
	UpdateAnimatorState(); // 초기 상태 설정
	
	// 초기 크기 설정
	if (m_animator) {
		const AnimationFrame& frame = m_animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
		
		const SpriteSheet* spriteSheet = m_animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW(L"Player: Animator 초기화 완료 - SpriteSheet 로드됨\n");
		} else {
			OutputDebugStringW(L"Player: Animator 초기화 실패 - SpriteSheet 없음\n");
		}
	} else {
		OutputDebugStringW(L"Player: Animator 생성 실패\n");
	}
	
	// 인벤토리 초기화 추가
	if (m_inventory) 
	{
		std::vector<Gdiplus::RectF> slotRects(INVENTORY_SLOT_COUNT);
		m_inventory->Init(slotRects);	
	} 
	
}

void Player::ToggleEquipItem(int slotIndex)
{
	if (slotIndex < 0 || slotIndex >= INVENTORY_SLOT_COUNT) {
		OutputDebugStringW(L"Player: Invalid slot index for equipment.\n");
		return;
	}

	const ItemSlot& targetSlot = m_inventory->GetSlot(slotIndex);
	if (targetSlot.IsEmpty()) {
		OutputDebugStringW(L"Player: Cannot equip empty slot.\n");
		return;
	}

	// 도구(Tool) 타입만 장착 가능
	Tool* toolItem = dynamic_cast<Tool*>(targetSlot.item);
	if (!toolItem) {
		OutputDebugStringW(L"Player: Cannot equip non-tool item.\n");
		return;
	}

	// 이미 장착된 아이템이 있는 경우
	if (m_equippedSlotIndex != -1) {
		if (m_equippedSlotIndex == slotIndex) { 
			// 같은 아이템을 다시 클릭 -> 장착 해제
			m_equippedSlotIndex = -1;
			m_equippedItem = nullptr;
			OutputDebugStringW(L"Player: Item unequipped.\n");
		}
		else {
			// 다른 아이템 클릭 -> 새로운 아이템 장착
			m_equippedSlotIndex = slotIndex; 
			m_equippedItem = targetSlot.item; 
			OutputDebugStringW((L"Player: Equipped new item from slot " + std::to_wstring(slotIndex) + L"\n").c_str());
		}
	}
	else {
		// 장착된 아이템이 없는 경우 -> 장착 시작
		m_equippedSlotIndex = slotIndex; 
		m_equippedItem = targetSlot.item; 
		OutputDebugStringW((L"Player: Equipped item from slot " + std::to_wstring(slotIndex) + L"\n").c_str());
	}
}

void Player::Damaged(int damage)
{
}

// TODO:
// 애니메이션 시트 단위로 이벤트 설정
//void Player::OnAnimationEvent(int frameIndex, const std::wstring& eventName)
//{
//	if (eventName == L"chop_hit") {
//		if (m_state == PlayerState::CHOP && m_currentInteractionTarget) {
//			// 나무 타입의 오브젝트 ID를 확인
//			GameObjectID objID = m_currentInteractionTarget->GetID();
//			if (objID == GOID_NORMAL_TREE_SHORT || objID == GOID_NORMAL_TREE_NORMAL || objID == GOID_NORMAL_TREE_TALL) {
//				// 도구를 장착했고 파손되지 않았으면 사용
//				Axe* equippedAxe = dynamic_cast<Axe*>(m_equippedItem);
//				if (equippedAxe && !equippedAxe->IsBroken()) {
//					// Tree의 상태를 변경하기 위해서는 캐릭터가 필요 (나중 구현)
//					Tree* tree = dynamic_cast<Tree*>(m_currentInteractionTarget);
//					if (tree) {
//						equippedAxe->ChopTree(tree); // Axe::ChopTree가 Tree::TakeDamage 호출
//					}
//				}
//			}
//		}
//	}
//}

void Player::UpdateAnimatorState() {
	if (m_animator) {
		m_animator->SetState(static_cast<int>(m_state), m_direction);	
	}

	if (m_animator) {
		const SpriteSheet* spriteSheet = m_animator->GetSpriteSheet();
		if(spriteSheet != nullptr)
			m_orignalBitmap = spriteSheet->GetBitmap();
	}
}

void Player::SetTargetPosition(float worldX, float worldY) {
	m_targetWorldPos = Gdiplus::PointF(worldX, worldY);
	m_state = PlayerState::WALK; // MOVING_TO_TARGET 대신 WALK로 설정하여 기본 WALK 애니메이션 재생
	isMoveToGoal = true; // 이동 목표 플래그 설정
	
	// 마우스 클릭 위치에서 방향 계산
	float dx = worldX - this->m_x;
	float dy = worldY - this->m_y;
	
	Direction newDirection;
	if (std::abs(dx) > std::abs(dy)) {
		newDirection = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
	}
	else {
		newDirection = (dy > 0) ? DIR_DOWN : DIR_UP;
	}
	
	// 방향이 변경되었으면 애니메이션 업데이트
	if (m_direction != newDirection) {
		m_direction = newDirection;
		UpdateAnimatorState();
	}
}



void Player::Update(float deltaTime) 
{
	HandleMovement();
	
	float moveSpeedThisFrame = m_playerSpeed * deltaTime;

	if ((m_state == PlayerState::WALK && isMoveToGoal) || isMoveToGoal)
	{
		float dx = m_targetWorldPos.X - this->m_x;
		float dy = m_targetWorldPos.Y - this->m_y;
		float distance = std::sqrt(dx * dx + dy * dy);
		isMoveToGoal = true;
		correctValue = 0;

		if (distance <= m_stopThreshold) { 
			this->m_x = m_targetWorldPos.X;
			this->m_y = m_targetWorldPos.Y;
			isMoveToGoal = false; // 목표 위치 도달 시 플래그 false로 설정
			
			// 목표 위치에 상호작용 가능한 오브젝트가 있으면 상호작용 시작
			if (m_currentInteractionTarget && m_currentInteractionTarget->CanInteract()) {
				// 상호작용 방향 계산
				float objDx = m_currentInteractionTarget->GetX() - this->m_x;
				float objDy = m_currentInteractionTarget->GetY() - this->m_y;
				
				Direction interactionDirection;
				if (std::abs(objDx) > std::abs(objDy)) {
					interactionDirection = (objDx > 0) ? DIR_RIGHT : DIR_LEFT;
				}
				else {
					interactionDirection = (objDy > 0) ? DIR_DOWN : DIR_UP;
				}
				
				m_direction = interactionDirection;
				UpdateAnimatorState();
				
				// 상호작용 시작
				OnInteraction(m_currentInteractionTarget);
			}
			else {
				m_state = PlayerState::IDLE; 
				UpdateAnimatorState(); // IDLE 애니메이션으로 전환
			}
		}
		else { 
			float moveX = (dx / distance) * moveSpeedThisFrame;
			float moveY = (dy / distance) * moveSpeedThisFrame;
			this->m_x += moveX;
			this->m_y += moveY;
			m_state = PlayerState::WALK; 
			UpdateAnimatorState(); 
		}
	}
	else if (m_state == PlayerState::CHOP || m_state == PlayerState::PICKUP) 		
	{
		if (m_state == PlayerState::PICKUP && m_animator && m_animator->IsAnimationDone()) 
		{
			FinalizeInteraction();
			m_state = PlayerState::IDLE;
			UpdateAnimatorState();
		}
	}

	// 건물 제작 코드 주석
	//if (InputManager::GetInstance()->IsKeyPressed('Q')) {

	//		std::map<UINT, UINT> requiredMaterials;
	//requiredMaterials[GOID_ITEM_NORMAL_TWIGS] = 2; // 나뭇가지 2개
	//requiredMaterials[GOID_ITEM_NORMAL_ROCK] = 2;  // 돌 2개

	//GameObject* itemObj = ObjectManager::GetInstance()->CreateGameObject(GOID_ITEM_AXE, 0.0f, 0.0f, nullptr, false);
	//Item* axeItemDef = dynamic_cast<Item*>(itemObj);

	//	if (!axeItemDef) {
	//		return;
	//	}

	//	// 플레이어 인벤토리에 재료가 충분한지 확인
	//	if (GetInventory()->CheckHasEnoughItems(requiredMaterials)) {
	//	
	//		//재료 소모 
	//		if (GetInventory()->ConsumeItems(requiredMaterials)) {

	//			// 제작된 아이템을 인벤토리에 추가) -> 완료
	//			if (GetInventory()->AddItem(axeItemDef, 1)) { // 도구 1개 추가				

	//			}
	//		}
	//	}
	//
	//}

	// 애니메이션 업데이트 (컴포넌트 방식)
	// 애니메이션 업데이트는 GameObject::Update()에서 컴포넌트의 Update()를 통해 자동으로 처리됨
	
	// 현재 프레임의 크기로 크기 업데이트
	if (m_animator) {
		const AnimationFrame& frame = m_animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
	}
	
	GameObject::Update(deltaTime);
}

// 상호작용 애니메이션 완료 후 아이템을 획득/수집 처리
void Player::FinalizeInteraction() {
	if (!m_currentInteractionTarget || !m_currentInteractionTarget->CanInteract()) {
		m_currentInteractionTarget = nullptr;
		return;
	}

	bool itemCollected = false;
	GameObjectID objID = m_currentInteractionTarget->GetID();
	GameObjectType objType = m_currentInteractionTarget->GetType();
	
	if (objType == GOBJ_ITEM) {
		GameObject* itemObj = ObjectManager::GetInstance()->CreateGameObject(objID, 0.0f, 0.0f, nullptr, false);
		Item* item = dynamic_cast<Item*>(itemObj);
		if (item && m_inventory->AddItem(item, 1)) 
		{
			ObjectManager::GetInstance()->RemoveGameObject(m_currentInteractionTarget);
			itemCollected = true;
		}
	}
	else if (objType == GOBJ_NATURAL_ENVIR) {
		Entity* entity = dynamic_cast<Entity*>(m_currentInteractionTarget);
		if (entity) {
			GameObjectID itemID = entity->GetDropItemID();
			int itemCount = entity->GetDropItemCount();
			
			if (itemID != GOID_NONE && itemCount > 0) {
				GameObject* itemObj = ObjectManager::GetInstance()->CreateGameObject(itemID, 0.0f, 0.0f, nullptr, false);
				Item* item = dynamic_cast<Item*>(itemObj);
				if (item && m_inventory->AddItem(item, itemCount)) 
				{
					ObjectManager::GetInstance()->RemoveGameObject(m_currentInteractionTarget);
					itemCollected = true;
				}
			}
		}
	}

	m_currentInteractionTarget = nullptr; 
	m_direction = DIR_DOWN;
	UpdateAnimatorState();
}

void Player::SetInventory(std::vector<Gdiplus::RectF> rectSize)
{
	m_inventory->Init(rectSize);
}

void Player::SetInteractionTarget(GameObject* obj)
{
	m_currentInteractionTarget = obj;
}

void Player::OnInteraction(GameObject* obj)
{
	if (!obj || !obj->CanInteract())
	{ 
		m_state = PlayerState::IDLE;
		return;
	}

	GameObjectID objID = obj->GetID();
	GameObjectType objType = obj->GetType();
	
	if (objID == GOID_NORMAL_TREE_SHORT || objID == GOID_NORMAL_TREE_NORMAL || objID == GOID_NORMAL_TREE_TALL) { 
		m_state = PlayerState::CHOP; 
		correctValue = 10;
		UpdateAnimatorState();
	}
	else if (objType == GOBJ_NATURAL_ENVIR) {

		m_state = PlayerState::PICKUP; 	
		correctValue = 10;
		UpdateAnimatorState();
	}
	else {
		m_state = PlayerState::PICKUP;  
		correctValue = 10;
		UpdateAnimatorState();
	}
}

void Player::LateUpdate()
{
}

void Player::LateInit() {
}

void Player::Release() {
	if (m_inventory) {
		delete m_inventory;
		m_inventory = nullptr;
	}
}

void Player::HandleClickInteraction(float worldX, float worldY)
{
	// ObjectManager에서 클릭한 위치의 오브젝트 찾기
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) return;
	
	// 범위 내에서 가장 가까운 오브젝트 찾기
	GameObject* targetItem = objectManager->FindObjectAtPositionWithBounds(worldX, worldY);
	
	if (targetItem) {
		// Item 타입의 오브젝트인지 확인
		if (targetItem->GetType() == GOBJ_ITEM) {
			SetTargetPosition(targetItem->GetX(), targetItem->GetY());
			SetInteractionTarget(targetItem);
			OutputDebugStringW((L"Player: Item 추가 - ID: " + std::to_wstring(targetItem->GetID()) + L"\n").c_str());
		}
		// 자연 환경 오브젝트인지 확인 (BerryBush, Grass, Sapling)
		else if (targetItem->GetType() == GOBJ_NATURAL_ENVIR) {
			GameObjectID objID = targetItem->GetID();
			if (objID == GOID_BERRY_TREE || objID == GOID_NORMAL_GRASS || objID == GOID_NORMAL_SAPLING) {
				SetTargetPosition(targetItem->GetX(), targetItem->GetY());
				SetInteractionTarget(targetItem);
				OutputDebugStringW((L"Player: 자연 환경 오브젝트 추가 - ID: " + std::to_wstring(targetItem->GetID()) + L"\n").c_str());
			}
		}
	}
}

void Player::HandleRightClick(float worldX, float worldY)
{
	// 우클릭으로 해당 위치로 이동 (오브젝트 상호작용 없이)
	SetTargetPosition(worldX, worldY);
}

void Player::HandleMovement()
{
	InputManager* inputManager = InputManager::GetInstance();
	if (!inputManager) return;
	
	// 좌클릭으로 Item 또는 자연 환경 오브젝트 상호작용
	if (inputManager->IsLButtonClicked()) {
		POINT mousePos = inputManager->GetMousePos();
		float mouseX = static_cast<float>(mousePos.x);
		float mouseY = static_cast<float>(mousePos.y);
		
		// 화면 좌표를 월드 좌표로 변환
		CameraManager* cameraManager = CameraManager::GetInstance();
		if (cameraManager) {
			Gdiplus::PointF worldPos = cameraManager->ScreenToWorld(mouseX, mouseY);		
			
			HandleClickInteraction(worldPos.X, worldPos.Y);
		}
	}
	
	// 우클릭으로 이동 처리 또는 아이템 이동
	if (inputManager->IsRButtonClicked()) {
		POINT mousePos = inputManager->GetMousePos();
		float mouseX = static_cast<float>(mousePos.x);
		float mouseY = static_cast<float>(mousePos.y);
		
		// 화면 좌표를 월드 좌표로 변환
		CameraManager* cameraManager = CameraManager::GetInstance();
		if (cameraManager) {
			Gdiplus::PointF worldPos = cameraManager->ScreenToWorld(mouseX, mouseY);
			
			// 우클릭 처리
			HandleRightClick(worldPos.X, worldPos.Y);
		}
	}
}


//float Player::GetSortKey(RenderLayer layer) const
//{
//	float baseKey = GameObject::GetSortKey(layer);
//
//	if (!IsInteracting()) {
//		return baseKey;
//	}
//
//	GameObject* target = GetInteractionTarget();
//	if (!target || !target->GetActive()) {
//		return baseKey;
//	}
//
//	RenderLayer interactionLayer = target->GetRenderLayer();
//	float interactionKey = static_cast<float>(interactionLayer) + target->GetY();
//	float adjustedKey = interactionKey + PLAYER_INTERACTION_SORT_OFFSET;
//
//	return (adjustedKey < baseKey) ? adjustedKey : baseKey;
//}
