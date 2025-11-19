#include "../../99_Default/pch.h"
#include "Player.h"
#include "../../03_Animation/Animator.h"
#include "../../01_Manager/InputManager/InputManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/ColliderManager/ColliderManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../03_Animation/AnimationClip.h"
#include "../../03_Animation/SpriteSheet.h"
#include "../../02_GameObject/UI/Inventory.h"
#include "../../02_GameObject/GameObject/Ingredient.h"
#include "../../02_GameObject/GameObject/Grass.h"
#include "../../02_GameObject/GameObject/BerryBush.h"
#include "../../02_GameObject/GameObject/Sapling.h"
#include "../../02_GameObject/GameObject/Tree.h"
#include "../../02_GameObject/GameObject/Rock.h"
#include "../../02_GameObject/Tool/Axe/Axe.h"
#include "../../02_GameObject/Tool/Tool.h"

Player::Player(float x, float y, GameObjectID characterID, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_PLAYER, characterID, x, y, 0.5f, 1.0f, DIR_DOWN, resourcePath, imageName), 
	  hp(100), maxHp(100), m_playerSpeed(300.f), m_stopThreshold(10), 
	  m_equippedSlotIndex(-1), m_equippedItem(nullptr), m_inventory(nullptr), m_currentInteractionTarget(nullptr)
{
	m_animator = nullptr;
}

Player::~Player() { Release(); }

void Player::Init()
{
	SetActive(true);
	SetInteractive(true); // Player의 상호작용 활성화
	m_direction = DIR_DOWN;
	m_state = PlayerState::IDLE;
	isMoveToGoal = false;		

	m_animator = new Animator();
	m_inventory = new Inventory();

	// Unity Animator 스타일 - 이벤트 콜백 설정
	m_animator->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
		this->OnAnimationEvent(frameIndex, eventName);
		});
	
	RegisterAllAnimations(); // Unity Animator 스타일 애니메이션 등록
	UpdateAnimatorState(); // 초기 상태 설정
	
	// 초기 크기 설정 (애니메이션 프레임의 첫 번째 프레임에서 크기 가져오기)
	if (m_animator) {
		const AnimationFrame& frame = m_animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
		
		// Animator 상태 확인
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
	std::shared_ptr<Tool> toolItem = std::dynamic_pointer_cast<Tool>(targetSlot.item);
	if (!toolItem) {
		OutputDebugStringW(L"Player: Cannot equip non-tool item.\n");
		return;
	}

	// 이미 장착된 아이템이 있는 경우
	if (m_equippedSlotIndex != -1) {
		if (m_equippedSlotIndex == slotIndex) { 
			// 같은 슬롯을 다시 클릭 -> 장착 해제
			m_equippedSlotIndex = -1;
			m_equippedItem = nullptr;
			OutputDebugStringW(L"Player: Item unequipped.\n");
		}
		else {
			// 다른 슬롯 클릭 -> 새로운 아이템 장착
			m_equippedSlotIndex = slotIndex; 
			m_equippedItem = targetSlot.item; 
			OutputDebugStringW((L"Player: Equipped new item from slot " + std::to_wstring(slotIndex) + L"\n").c_str());
		}
	}
	else {
		// 장착된 아이템이 없는 경우 -> 새로 장착
		m_equippedSlotIndex = slotIndex; 
		m_equippedItem = targetSlot.item; 
		OutputDebugStringW((L"Player: Equipped item from slot " + std::to_wstring(slotIndex) + L"\n").c_str());
	}
}

void Player::OnAnimationEvent(int frameIndex, const std::wstring& eventName)
{
	if (eventName == L"chop_hit") {
		if (m_state == PlayerState::CHOP && m_currentInteractionTarget) {
			// 나무 타입인지 ID로 확인
			GameObjectID objID = m_currentInteractionTarget->GetID();
			if (objID == GOID_NORMAL_TREE_SHORT || objID == GOID_NORMAL_TREE_NORMAL || objID == GOID_NORMAL_TREE_TALL) {
				// 장착된 도끼 가져와서 나무 베기
				std::shared_ptr<Axe> equippedAxe = dynamic_pointer_cast<Axe>(m_equippedItem);
				if (equippedAxe && !equippedAxe->IsBroken()) {
					// Tree의 상태를 변경하기 위해 다이나믹 캐스팅 필요 (한 번만)
					Tree* tree = dynamic_cast<Tree*>(m_currentInteractionTarget);
					if (tree) {
						equippedAxe->ChopTree(tree); // Axe::ChopTree가 Tree::TakeDamage 호출
					}
				}
			}
		}
	}
}

void Player::RegisterAllAnimations() {
    OutputDebugStringW(L"Player: RegisterAllAnimations 시작\n");
    
    // Unity Animator 스타일 - Animator에 모든 애니메이션 등록
    
    // ResourceManager를 사용하여 경로 구성
	ResourceManager* pRM = ResourceManager::GetInstance();
    
    // IDLE 애니메이션들
    std::wstring idleDownPath = pRM->BuildResourcePath(resourcePath, L"Idle", L"Wilson_Idle_Down.png");

    m_animator->RegisterAnimation(PlayerState::IDLE, DIR_DOWN,
        idleDownPath,
        126, 189, 7, 64, 0.03f, m_pivotX, m_pivotY, true);
        
    OutputDebugStringW(L"Player: IDLE_DOWN 애니메이션 등록 완료\n");
        
    m_animator->RegisterAnimation(PlayerState::IDLE, DIR_UP,
        pRM->BuildResourcePath(resourcePath, L"Idle", L"Wilson_Idle_Up.png"),
        128, 193, 7, 64, 0.03f, m_pivotX, m_pivotY, true);
        
    m_animator->RegisterAnimation(PlayerState::IDLE, DIR_LEFT,
        pRM->BuildResourcePath(resourcePath, L"Idle", L"Wilson_Idle_Side.png"),
		135, 194, 7, 64, 0.03f, m_pivotX, m_pivotY, true);
        
    m_animator->RegisterAnimation(PlayerState::IDLE, DIR_RIGHT,
        pRM->BuildResourcePath(resourcePath, L"Idle", L"Wilson_Idle_Side.png"),
        135, 194, 7, 64, 0.03f, m_pivotX, m_pivotY, true);

    // WALK 애니메이션들
    m_animator->RegisterAnimation(PlayerState::WALK, DIR_DOWN,
        pRM->BuildResourcePath(resourcePath, L"Run", L"Wilson_Run_Down.png"),
        139, 226, 6, 33, 0.03f, m_pivotX, m_pivotY, true);
        
    m_animator->RegisterAnimation(PlayerState::WALK, DIR_UP,
        pRM->BuildResourcePath(resourcePath, L"Run", L"Wilson_Run_Up.png"),
        133, 231, 6, 33, 0.03f, m_pivotX, m_pivotY, true);
        
    m_animator->RegisterAnimation(PlayerState::WALK, DIR_LEFT,
        pRM->BuildResourcePath(resourcePath, L"Run", L"Wilson_Run_Side.png"),
        141, 226, 6, 33, 0.03f, m_pivotX, m_pivotY, true);
        
    m_animator->RegisterAnimation(PlayerState::WALK, DIR_RIGHT,
        pRM->BuildResourcePath(resourcePath, L"Run", L"Wilson_Run_Side.png"),
        141, 226, 6, 33, 0.03f, m_pivotX, m_pivotY, true);

    // PICKUP 애니메이션들 (모든 방향에 대해 등록)
    m_animator->RegisterAnimation(PlayerState::PICKUP, DIR_DOWN,
        pRM->BuildResourcePath(resourcePath, L"Interact", L"Interact_wilson_pickup_pst_down.png"),
        127, 201, 6, 20, 0.03f, m_pivotX, m_pivotY, false);
        
    m_animator->RegisterAnimation(PlayerState::PICKUP, DIR_UP,
        pRM->BuildResourcePath(resourcePath, L"Interact", L"Interact_wilson_pickup_pst_down.png"),
        127, 201, 6, 20, 0.03f, m_pivotX, m_pivotY, false);
        
    m_animator->RegisterAnimation(PlayerState::PICKUP, DIR_LEFT,
        pRM->BuildResourcePath(resourcePath, L"Interact", L"Interact_wilson_pickup_pst_down.png"),
        127, 201, 6, 20, 0.03f, m_pivotX, m_pivotY, false);
        
    m_animator->RegisterAnimation(PlayerState::PICKUP, DIR_RIGHT,
        pRM->BuildResourcePath(resourcePath, L"Interact", L"Interact_wilson_pickup_pst_down.png"),
        127, 201, 6, 20, 0.03f, m_pivotX, m_pivotY, false);

    // CHOP 애니메이션 (이벤트 포함)
    std::map<int, std::wstring> chopEvents = {{4, L"chop_hit"}};
    m_animator->RegisterAnimation(PlayerState::CHOP, DIR_DOWN,
        pRM->BuildResourcePath(resourcePath, L"Axe", L"axe_wilson_chop_loop_down.png"),
        284, 248, 6, 54, 0.03f, m_pivotX + 0.1f, m_pivotY, false, chopEvents);
        
    m_animator->RegisterAnimation(PlayerState::CHOP, DIR_UP,
        pRM->BuildResourcePath(resourcePath, L"Axe", L"axe_wilson_chop_loop_down.png"),
        284, 248, 6, 54, 0.03f, m_pivotX + 0.1f, m_pivotY, false, chopEvents);
        
    m_animator->RegisterAnimation(PlayerState::CHOP, DIR_LEFT,
        pRM->BuildResourcePath(resourcePath, L"Axe", L"axe_wilson_chop_loop_down.png"),
        284, 248, 6, 54, 0.03f, m_pivotX + 0.1f, m_pivotY, false, chopEvents);
        
    m_animator->RegisterAnimation(PlayerState::CHOP, DIR_RIGHT,
        pRM->BuildResourcePath(resourcePath, L"Axe", L"axe_wilson_chop_loop_down.png"),
        284, 248, 6, 54, 0.03f, m_pivotX + 0.1f, m_pivotY, false, chopEvents);
}

void Player::UpdateAnimatorState() {
    // Unity Animator 스타일 - 상태와 방향만 설정하면 자동으로 애니메이션 선택
	if (m_animator) {
		m_animator->SetState(m_state, m_direction);	
	}

	const SpriteSheet* spriteSheet = m_animator->GetSpriteSheet();
	if(spriteSheet != nullptr)
		m_bitmap = spriteSheet->GetBitmap();
}



void Player::SetTargetPosition(float worldX, float worldY) {
	m_targetWorldPos = Gdiplus::PointF(worldX, worldY);
	m_state = PlayerState::WALK; // MOVING_TO_TARGET 대신 WALK로 설정하여 즉시 WALK 애니메이션 시작
	isMoveToGoal = true; // 이동 시작 플래그 설정
	
	// 마우스 클릭 시점에 즉시 방향 설정
	float dx = worldX - this->m_x;
	float dy = worldY - this->m_y;
	
	Direction newDirection;
	if (std::abs(dx) > std::abs(dy)) {
		newDirection = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
	}
	else {
		newDirection = (dy > 0) ? DIR_DOWN : DIR_UP;
	}
	
	// 방향이 바뀌었을 때만 업데이트
	if (m_direction != newDirection) {
		m_direction = newDirection;
		UpdateAnimatorState();
	}
}



void Player::Update(float deltaTime) 
{
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
			isMoveToGoal = false; // 목표 위치 도착 시 즉시 false로 설정
			
			// 목표 위치에 도착했을 때 상호작용 대상이 있으면 상호작용 시작
			if (m_currentInteractionTarget && m_currentInteractionTarget->CanInteract()) {
				// 상호작용 대상과의 방향 설정
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

			// 이동 중에도 방향 유지 (SetTargetPosition에서 설정된 방향 사용)
			// 방향 설정은 SetTargetPosition에서 이미 처리됨
			// 여기서는 애니메이션 상태만 업데이트
		}
	}
	else if (m_state == PlayerState::CHOP || m_state == PlayerState::PICKUP) { 
		// PICKUP 애니메이션 완료 감지 (애니메이션이 완전히 끝났을 때)
		if (m_state == PlayerState::PICKUP && m_animator && m_animator->IsAnimationDone()) {
			FinalizeInteraction();
			m_state = PlayerState::IDLE;
			UpdateAnimatorState(); 
		}
		// CHOP 상태는 애니메이션 이벤트로 처리되므로 여기서는 상태만 유지
	}

	// 도끼 제작 코드 예시
	//if (InputManager::GetInstance()->IsKeyPressed('Q')) {

	//		std::map<UINT, UINT> requiredMaterials;
	//requiredMaterials[GOID_ITEM_NORMAL_TWIGS] = 2; // 나뭇가지 2개
	//requiredMaterials[GOID_ITEM_NORMAL_ROCK] = 2;  // 돌 2개

	//std::shared_ptr<Item> axeItemDef = ObjectManager::GetInstance()->CreateItem(GOID_ITEM_AXE);

	//	if (!axeItemDef) {
	//		return;
	//	}

	//	// 플레이어 인벤토리에 재료가 충분한지 확인
	//	if (GetInventory()->CheckHasEnoughItems(requiredMaterials)) {
	//	
	//		//재료 소모 
	//		if (GetInventory()->ConsumeItems(requiredMaterials)) {

	//			// 제작된 도끼 아이템 인벤토리에 추가) -> 예비
	//			if (GetInventory()->AddItem(axeItemDef, 1)) { // 도끼 1개 추가				

	//			}
	//		}
	//	}
	//
	//}

	// 애니메이션 업데이트 (프레임 계산)
	UpdateAnimation(deltaTime);
	
	// 현재 프레임 정보로 크기 업데이트
	if (m_animator) {
		const AnimationFrame& frame = m_animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
	}
	
	GameObject::Update(deltaTime);
}

// 상호작용 애니메이션 완료 후 실제 아이템 획득/제거 로직
void Player::FinalizeInteraction() {
	if (!m_currentInteractionTarget || !m_currentInteractionTarget->CanInteract()) {
		m_currentInteractionTarget = nullptr;
		return;
	}

	bool itemCollected = false;
	GameObjectID objID = m_currentInteractionTarget->GetID();
	GameObjectType objType = m_currentInteractionTarget->GetType();
	
	// 오브젝트 타입과 ID에 따른 상호작용 분기 로직
	if (objType == GOBJ_ITEM) {
		// 기존 Ingredient 아이템 처리
		std::shared_ptr<Item> itemObj = ObjectManager::GetInstance()->CreateItem(objID);
		if (itemObj && m_inventory->AddItem(itemObj, 1)) 
		{
			ObjectManager::GetInstance()->RemoveGameObject(m_currentInteractionTarget);
			itemCollected = true;
		}
	}
	else if (objType == GOBJ_NATURAL_ENVIR) {
		// 자연 환경 오브젝트 처리 (BerryBush, Grass, Sapling)
		// Entity의 가상 함수를 사용하여 아이템 정보 가져오기
		Entity<GrassState>* entity = dynamic_cast<Entity<GrassState>*>(m_currentInteractionTarget);
		if (entity) {
			GameObjectID itemID = entity->GetDropItemID();
			int itemCount = entity->GetDropItemCount();
			
			// 아이템이 설정되어 있는 경우에만 처리
			if (itemID != GOID_NONE && itemCount > 0) {
				// 해당 아이템 생성 및 인벤토리에 추가
				std::shared_ptr<Item> itemObj = ObjectManager::GetInstance()->CreateItem(itemID);
				if (itemObj && m_inventory->AddItem(itemObj, itemCount)) 
				{
					ObjectManager::GetInstance()->RemoveGameObject(m_currentInteractionTarget);
					itemCollected = true;
				}
			}
		}
	}

	m_currentInteractionTarget = nullptr; // 상호작용 대상 초기화
	
	// pickup 완료 시 Direction을 DIR_DOWN으로 설정
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

	// m_currentInteractionTarget은 이미 SetInteractionTarget에서 설정됨
	// 여기서는 다시 설정하지 않음

	// 오브젝트 타입에 따른 상호작용 처리
	GameObjectID objID = obj->GetID();
	GameObjectType objType = obj->GetType();
	
	// 나무 타입인지 확인 (ID 기반으로 빠른 체크)
	if (objID == GOID_NORMAL_TREE_SHORT || objID == GOID_NORMAL_TREE_NORMAL || objID == GOID_NORMAL_TREE_TALL) { 
		m_state = PlayerState::CHOP; // 상태만 설정
		correctValue = 10;
		UpdateAnimatorState(); // Unity Animator가 자동으로 애니메이션 선택
	}
	else if (objType == GOBJ_NATURAL_ENVIR) {
		// 자연 환경 오브젝트들 (BerryBush, Grass, Sapling)은 PICKUP
		m_state = PlayerState::PICKUP; // 상태만 설정  
		correctValue = 10;
		UpdateAnimatorState(); // Unity Animator가 자동으로 애니메이션 선택
	}
	else {
		// 그 외의 상호작용 가능한 오브젝트들은 PICKUP
		m_state = PlayerState::PICKUP; // 상태만 설정  
		correctValue = 10;
		UpdateAnimatorState(); // Unity Animator가 자동으로 애니메이션 선택
	}
}

void Player::LateUpdate()
{
}

void Player::LateInit() {
}

void Player::Render(Gdiplus::Graphics* pGraphics) {
    
}

void Player::Release() {
	if (m_inventory) {
		delete m_inventory;
		m_inventory = nullptr;
	}

	if (m_animator) { delete m_animator; m_animator = nullptr; }
}



void Player::UpdateAnimation(float deltaTime) {
	if (m_animator) {
		m_animator->Update(deltaTime);
	}
}

Gdiplus::Bitmap* Player::GetBitmap() const
{
    if (!m_bitmap) {
		OutputDebugStringW(L"Player: GetBitmap - m_bitmap이 null입니다.\n");
		return nullptr;
	}
    
    OutputDebugStringW(L"Player: GetBitmap - 저장된 비트맵 반환 성공\n");
    return m_bitmap;
}

void Player::HandleClickInteraction(float worldX, float worldY)
{
	// ObjectManager를 통해 클릭한 위치의 오브젝트 찾기
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) return;
	
	// 이미지 테두리 기반으로 정확한 오브젝트 찾기
	GameObject* targetItem = objectManager->FindObjectAtPositionWithBounds(worldX, worldY);
	
	if (targetItem) {
		// Item 타입의 오브젝트인지 확인
		if (targetItem->GetType() == GOBJ_ITEM) {
			SetTargetPosition(targetItem->GetX(), targetItem->GetY());
			SetInteractionTarget(targetItem);
			OutputDebugStringW((L"Player: Item 발견 - ID: " + std::to_wstring(targetItem->GetID()) + L"\n").c_str());
		}
		// 자연 환경 오브젝트인지 확인 (BerryBush, Grass, Sapling)
		else if (targetItem->GetType() == GOBJ_NATURAL_ENVIR) {
			GameObjectID objID = targetItem->GetID();
			if (objID == GOID_BERRY_TREE || objID == GOID_NORMAL_GRASS || objID == GOID_NORMAL_SAPLING) {
				SetTargetPosition(targetItem->GetX(), targetItem->GetY());
				SetInteractionTarget(targetItem);
				OutputDebugStringW((L"Player: 자연 환경 오브젝트 발견 - ID: " + std::to_wstring(targetItem->GetID()) + L"\n").c_str());
			}
		}
	}
}

void Player::HandleRightClick(float worldX, float worldY)
{
	// 우클릭으로 해당 위치로 이동 (오브젝트 유무와 관계없이)
	SetTargetPosition(worldX, worldY);
}

void Player::HandleMovement()
{
	InputManager* inputManager = InputManager::GetInstance();
	if (!inputManager) return;
	
	// 좌클릭으로 Item 및 자연 환경 오브젝트 상호작용
	if (inputManager->IsLButtonClicked()) {
		POINT mousePos = inputManager->GetMousePos();
		float mouseX = static_cast<float>(mousePos.x);
		float mouseY = static_cast<float>(mousePos.y);
		
		// 스크린 좌표를 월드 좌표로 변환
		CameraManager* cameraManager = CameraManager::GetInstance();
		if (cameraManager) {
			Gdiplus::PointF worldPos = cameraManager->ScreenToWorld(mouseX, mouseY);
			
			// 클릭 상호작용 처리
			HandleClickInteraction(worldPos.X, worldPos.Y);
		}
	}
	
	// 우클릭으로 이동 취소 또는 카메라 이동
	if (inputManager->IsRButtonClicked()) {
		POINT mousePos = inputManager->GetMousePos();
		float mouseX = static_cast<float>(mousePos.x);
		float mouseY = static_cast<float>(mousePos.y);
		
		// 스크린 좌표를 월드 좌표로 변환
		CameraManager* cameraManager = CameraManager::GetInstance();
		if (cameraManager) {
			Gdiplus::PointF worldPos = cameraManager->ScreenToWorld(mouseX, mouseY);
			
			// 우클릭 처리
			HandleRightClick(worldPos.X, worldPos.Y);
		}
	}
}
