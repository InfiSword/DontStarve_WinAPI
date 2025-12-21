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
#include "../../../03_Animation/AnimationDefinition.h"

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

std::vector<AnimationDefinition> Player::GetAnimationDefinitions() const {
    OutputDebugStringW(L"Player: GetAnimationDefinitions 시작\n");
    
    std::vector<AnimationDefinition> definitions;
    
    // ResourceManager를 사용하여 리소스 로드
    ResourceManager* pRM = ResourceManager::GetInstance();
    
    // IDLE 애니메이션들
    AnimationDefinition idleDown;
    idleDown.state = static_cast<int>(PlayerState::IDLE);
    idleDown.direction = DIR_DOWN;
    idleDown.imagePath = pRM->BuildResourcePath(m_resourcePath, L"Idle", L"Wilson_Idle_Down.png");
    idleDown.frameWidth = 126;
    idleDown.frameHeight = 189;
    idleDown.framesPerRow = 7;
    idleDown.totalFrames = 64;
    idleDown.frameDuration = 0.03f;
    idleDown.pivotX = m_pivotX;
    idleDown.pivotY = m_pivotY;
    idleDown.isLoop = true;
    definitions.push_back(idleDown);
    
    AnimationDefinition idleUp;
    idleUp.state = static_cast<int>(PlayerState::IDLE);
    idleUp.direction = DIR_UP;
    idleUp.imagePath = pRM->BuildResourcePath(m_resourcePath, L"Idle", L"Wilson_Idle_Up.png");
    idleUp.frameWidth = 128;
    idleUp.frameHeight = 193;
    idleUp.framesPerRow = 7;
    idleUp.totalFrames = 64;
    idleUp.frameDuration = 0.03f;
    idleUp.pivotX = m_pivotX;
    idleUp.pivotY = m_pivotY;
    idleUp.isLoop = true;
    definitions.push_back(idleUp);
    
    AnimationDefinition idleLeft;
    idleLeft.state = static_cast<int>(PlayerState::IDLE);
    idleLeft.direction = DIR_LEFT;
    idleLeft.imagePath = pRM->BuildResourcePath(m_resourcePath, L"Idle", L"Wilson_Idle_Side.png");
    idleLeft.frameWidth = 135;
    idleLeft.frameHeight = 194;
    idleLeft.framesPerRow = 7;
    idleLeft.totalFrames = 64;
    idleLeft.frameDuration = 0.03f;
    idleLeft.pivotX = m_pivotX;
    idleLeft.pivotY = m_pivotY;
    idleLeft.isLoop = true;
    definitions.push_back(idleLeft);
    
    AnimationDefinition idleRight;
    idleRight.state = static_cast<int>(PlayerState::IDLE);
    idleRight.direction = DIR_RIGHT;
    idleRight.imagePath = pRM->BuildResourcePath(m_resourcePath, L"Idle", L"Wilson_Idle_Side.png");
    idleRight.frameWidth = 135;
    idleRight.frameHeight = 194;
    idleRight.framesPerRow = 7;
    idleRight.totalFrames = 64;
    idleRight.frameDuration = 0.03f;
    idleRight.pivotX = m_pivotX;
    idleRight.pivotY = m_pivotY;
    idleRight.isLoop = true;
    definitions.push_back(idleRight);

    // WALK 애니메이션들
    AnimationDefinition walkDown;
    walkDown.state = static_cast<int>(PlayerState::WALK);
    walkDown.direction = DIR_DOWN;
    walkDown.imagePath = pRM->BuildResourcePath(m_resourcePath, L"Run", L"Wilson_Run_Down.png");
    walkDown.frameWidth = 139;
    walkDown.frameHeight = 226;
    walkDown.framesPerRow = 6;
    walkDown.totalFrames = 33;
    walkDown.frameDuration = 0.03f;
    walkDown.pivotX = m_pivotX;
    walkDown.pivotY = m_pivotY;
    walkDown.isLoop = true;
    definitions.push_back(walkDown);
    
    AnimationDefinition walkUp;
    walkUp.state = static_cast<int>(PlayerState::WALK);
    walkUp.direction = DIR_UP;
    walkUp.imagePath = pRM->BuildResourcePath(m_resourcePath, L"Run", L"Wilson_Run_Up.png");
    walkUp.frameWidth = 133;
    walkUp.frameHeight = 231;
    walkUp.framesPerRow = 6;
    walkUp.totalFrames = 33;
    walkUp.frameDuration = 0.03f;
    walkUp.pivotX = m_pivotX;
    walkUp.pivotY = m_pivotY;
    walkUp.isLoop = true;
    definitions.push_back(walkUp);
    
    AnimationDefinition walkLeft;
    walkLeft.state = static_cast<int>(PlayerState::WALK);
    walkLeft.direction = DIR_LEFT;
    walkLeft.imagePath = pRM->BuildResourcePath(m_resourcePath, L"Run", L"Wilson_Run_Side.png");
    walkLeft.frameWidth = 141;
    walkLeft.frameHeight = 226;
    walkLeft.framesPerRow = 6;
    walkLeft.totalFrames = 33;
    walkLeft.frameDuration = 0.03f;
    walkLeft.pivotX = m_pivotX;
    walkLeft.pivotY = m_pivotY;
    walkLeft.isLoop = true;
    definitions.push_back(walkLeft);
    
    AnimationDefinition walkRight;
    walkRight.state = static_cast<int>(PlayerState::WALK);
    walkRight.direction = DIR_RIGHT;
    walkRight.imagePath = pRM->BuildResourcePath(m_resourcePath, L"Run", L"Wilson_Run_Side.png");
    walkRight.frameWidth = 141;
    walkRight.frameHeight = 226;
    walkRight.framesPerRow = 6;
    walkRight.totalFrames = 33;
    walkRight.frameDuration = 0.03f;
    walkRight.pivotX = m_pivotX;
    walkRight.pivotY = m_pivotY;
    walkRight.isLoop = true;
    definitions.push_back(walkRight);

    // PICKUP 애니메이션들 (모든 방향에서 동일한 이미지 사용)
    std::wstring pickupPath = pRM->BuildResourcePath(m_resourcePath, L"Interact", L"Interact_wilson_pickup_pst_down.png");
    for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
        AnimationDefinition pickup;
        pickup.state = static_cast<int>(PlayerState::PICKUP);
        pickup.direction = static_cast<Direction>(dir);
        pickup.imagePath = pickupPath;
        pickup.frameWidth = 127;
        pickup.frameHeight = 201;
        pickup.framesPerRow = 6;
        pickup.totalFrames = 20;
        pickup.frameDuration = 0.03f;
        pickup.pivotX = m_pivotX;
        pickup.pivotY = m_pivotY;
        pickup.isLoop = false;
        definitions.push_back(pickup);
    }

    // CHOP 애니메이션 (이벤트 포함)
    std::map<int, std::wstring> chopEvents = {{4, L"chop_hit"}};
    std::wstring chopPath = pRM->BuildResourcePath(m_resourcePath, L"Axe", L"axe_wilson_chop_loop_down.png");
    for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
        AnimationDefinition chop;
        chop.state = static_cast<int>(PlayerState::CHOP);
        chop.direction = static_cast<Direction>(dir);
        chop.imagePath = chopPath;
        chop.frameWidth = 284;
        chop.frameHeight = 248;
        chop.framesPerRow = 6;
        chop.totalFrames = 54;
        chop.frameDuration = 0.03f;
        chop.pivotX = m_pivotX + 0.1f;
        chop.pivotY = m_pivotY;
        chop.isLoop = false;
        chop.events = chopEvents;
        definitions.push_back(chop);
    }
    
    OutputDebugStringW((L"Player: GetAnimationDefinitions 완료 - " + std::to_wstring(definitions.size()) + L"개 애니메이션 정의 반환\n").c_str());
    return definitions;
}

void Player::UpdateAnimatorState() {
    // Unity Animator 스타일 - 상태와 방향만 설정하면 자동으로 애니메이션 전환
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
	// 입력 처리 및 상호작용 시작 (먼저 처리)
	HandleMovement();
	
	float moveSpeedThisFrame = m_playerSpeed * deltaTime;

	if ((m_state == PlayerState::WALK && isMoveToGoal) || isMoveToGoal) {
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
			UpdateAnimatorState(); // WALK 애니메이션으로 전환

			// 이동 중에 방향 변경 (SetTargetPosition에서 이미 처리한 경우)
			// 방향 변경은 SetTargetPosition에서 이미 처리됨
			// 여기서는 애니메이션 상태만 업데이트
		}
	}
	else if (m_state == PlayerState::CHOP || m_state == PlayerState::PICKUP) { 
		// PICKUP 애니메이션 완료 대기 (애니메이션이 끝나면 상호작용 완료)
		if (m_state == PlayerState::PICKUP && m_animator && m_animator->IsAnimationDone()) {
			FinalizeInteraction();
			m_state = PlayerState::IDLE;
			UpdateAnimatorState(); 
		}
		// CHOP 같은 애니메이션은 이벤트로 처리되기 때문에 여기서 상태만 유지
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
	
	// 오브젝트 타입과 ID에 따라 상호작용 처리 방법
	if (objType == GOBJ_ITEM) {
		// 일반 Ingredient 아이템 처리
		// 인벤토리 아이템은 ObjectManager에 추가하지 않음 (addToManager = false)
		GameObject* itemObj = ObjectManager::GetInstance()->CreateGameObject(objID, 0.0f, 0.0f, nullptr, false);
		Item* item = dynamic_cast<Item*>(itemObj);
		if (item && m_inventory->AddItem(item, 1)) 
		{
			ObjectManager::GetInstance()->RemoveGameObject(m_currentInteractionTarget);
			itemCollected = true;
		}
	}
	else if (objType == GOBJ_NATURAL_ENVIR) {
		// 자연 환경 오브젝트 처리 (BerryBush, Grass, Sapling)
		// Entity의 가상 함수를 사용하여 드롭 아이템 정보 가져오기
		Entity* entity = dynamic_cast<Entity*>(m_currentInteractionTarget);
		if (entity) {
			GameObjectID itemID = entity->GetDropItemID();
			int itemCount = entity->GetDropItemCount();
			
			// 드롭 아이템이 설정되어 있는 경우에만 처리
			if (itemID != GOID_NONE && itemCount > 0) {
				// 해당 아이템을 생성하여 인벤토리에 추가
				// 인벤토리 아이템은 ObjectManager에 추가하지 않음 (addToManager = false)
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

	m_currentInteractionTarget = nullptr; // 상호작용 대상 초기화
	
	// pickup 완료 후 Direction을 DIR_DOWN으로 설정
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

	// m_currentInteractionTarget는 이미 SetInteractionTarget에서 설정됨
	// 여기서 다시 설정하지 않음

	// 오브젝트 타입에 따라 상호작용 처리
	GameObjectID objID = obj->GetID();
	GameObjectType objType = obj->GetType();
	
	// 나무 타입인지 확인 (ID 기반으로 정확히 체크)
	if (objID == GOID_NORMAL_TREE_SHORT || objID == GOID_NORMAL_TREE_NORMAL || objID == GOID_NORMAL_TREE_TALL) { 
		m_state = PlayerState::CHOP; // 상태 변경
		correctValue = 10;
		UpdateAnimatorState(); // Unity Animator에서 자동으로 애니메이션 전환
	}
	else if (objType == GOBJ_NATURAL_ENVIR) {
		// 자연 환경 오브젝트는 (BerryBush, Grass, Sapling)은 PICKUP
		m_state = PlayerState::PICKUP; // 상태 변경  
		correctValue = 10;
		UpdateAnimatorState(); // Unity Animator에서 자동으로 애니메이션 전환
	}
	else {
		// 기타 상호작용 가능한 오브젝트는 PICKUP
		m_state = PlayerState::PICKUP; // 상태 변경  
		correctValue = 10;
		UpdateAnimatorState(); // Unity Animator에서 자동으로 애니메이션 전환
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
			
			// 클릭 상호작용 처리
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
