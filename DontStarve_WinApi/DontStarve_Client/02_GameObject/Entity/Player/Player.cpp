#include "99_Default/pch.h"
#include "Player.h"
#include "../../../01_Manager/InputManager/InputManager.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../02_GameObject/UI/Inventory.h"
#include "../../../03_Animation/Animator.h"
#include "../../Item/Tool/Tool.h"
#include "../../Component/Transform/Transform.h"
#include "../../Component/Collider/Collider.h"

static const float CHOP_PIVOT_X = 0.3f;
static const float CHOP_PIVOT_Y = 0.9f;

Player::Player(float x, float y, GameObjectID characterID, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_PLAYER, characterID, x, y, 0.5f, 1.0f, DIR_DOWN, L"", imageName, true, false),  // baseDir 빈 경우 imageName만 사용(플레이어는 Animator에서 경로 사용)
	hp(100), maxHp(100), m_playerSpeed(300.f), m_stopThreshold(10),
	m_equippedSlotIndex(-1), m_equippedItem(nullptr), m_inventory(nullptr), m_pendingInteractionTarget(nullptr), m_activeInteractionTarget(nullptr), m_state(PlayerState::IDLE), isMoveToGoal(false)
{
}

Player::~Player() { Release(); }

void Player::Init()
{
	Entity::Init();

	// Animator 생성 후 애니메이션 등록 (AnimationDefinition 클래스 제거)
	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}

	ResourceManager* pRM = ResourceManager::GetInstance();
	const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GetID());
	if (!objData) return;
	std::wstring base = objData->baseDir;

	// IDLE
	std::wstring idleDownPath = base + L"\\Idle\\Wilson_Idle_Down.png";
	m_animator->RegisterAnimation((int)PlayerState::IDLE, DIR_DOWN, idleDownPath,
		126, 189, 7, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

	std::wstring idleUpPath = base + L"\\Idle\\Wilson_Idle_Up.png";
	m_animator->RegisterAnimation((int)PlayerState::IDLE, DIR_UP, idleUpPath,
		128, 193, 7, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

	std::wstring idleSidePath = base + L"\\Idle\\Wilson_Idle_Side.png";
	m_animator->RegisterAnimation((int)PlayerState::IDLE, DIR_LEFT, idleSidePath,
		135, 194, 7, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
	m_animator->RegisterAnimation((int)PlayerState::IDLE, DIR_RIGHT, idleSidePath,
		135, 194, 7, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

	// WALK(RUN)
	std::wstring runDownPath = base + L"\\Run\\Wilson_Run_Down.png";
	m_animator->RegisterAnimation((int)PlayerState::WALK, DIR_DOWN, runDownPath,
		139, 226, 6, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

	std::wstring runUpPath = base + L"\\Run\\Wilson_Run_Up.png";
	m_animator->RegisterAnimation((int)PlayerState::WALK, DIR_UP, runUpPath,
		133, 231, 6, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

	std::wstring runSidePath = base + L"\\Run\\Wilson_Run_Side.png";
	m_animator->RegisterAnimation((int)PlayerState::WALK, DIR_LEFT, runSidePath,
		142, 226, 6, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
	m_animator->RegisterAnimation((int)PlayerState::WALK, DIR_RIGHT, runSidePath,
		141, 226, 6, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

	// PICKUP (마지막 프레임에 종료 이벤트)
	const UINT PICKUP_TOTAL_FRAMES = 20;
	const int PICKUP_LAST_FRAME = PICKUP_TOTAL_FRAMES - 1; // 19
	std::wstring pickupPath = base + L"\\Interact\\Interact_wilson_pickup_pst_down.png";
	for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
		m_animator->RegisterAnimation((int)PlayerState::PICKUP, (Direction)dir, pickupPath,
			127, 201, 6, PICKUP_TOTAL_FRAMES, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
		// AnimationClip에 직접 이벤트 등록 및 콜백 설정
		AnimationClip* clip = m_animator->GetAnimationClip((int)PlayerState::PICKUP, (Direction)dir);
		if (clip) {
			clip->AddEventFrame(PICKUP_LAST_FRAME, L"pickup_end");
			// 이벤트 콜백 설정
			clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
				if (eventName == L"pickup_end") {
					this->OnPickupEnd();
				}
				});
		}
	}

	// CHOP (이벤트 적용: chop_hit 시 나무에게 데미지, 마지막 프레임에 종료 이벤트)
	const UINT CHOP_TOTAL_FRAMES = 36;
	const int CHOP_HIT_FRAME = 4;
	const int CHOP_LAST_FRAME = CHOP_TOTAL_FRAMES - 1; // 35
	std::wstring chopPath = base + L"\\Axe\\axe_wilson_chop_loop_down.png";
	for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
		m_animator->RegisterAnimation((int)PlayerState::CHOP, (Direction)dir, chopPath,
			284, 248, 6, CHOP_TOTAL_FRAMES, CHOP_PIVOT_X, CHOP_PIVOT_Y, false, 0.01f);
		// AnimationClip에 직접 이벤트 등록 및 콜백 설정
		AnimationClip* clip = m_animator->GetAnimationClip((int)PlayerState::CHOP, (Direction)dir);
		if (clip) {
			clip->AddEventFrame(CHOP_HIT_FRAME, L"chop_hit");
			clip->AddEventFrame(CHOP_LAST_FRAME, L"chop_end");
			// 이벤트 콜백 설정
			clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
				if (eventName == L"chop_hit") {
					this->OnChopHit();
				}
				else if (eventName == L"chop_end") {
					this->OnChopEnd();
				}
				});
		}
	}

	m_inventory = new Inventory();

	UpdateAnimatorState();

	if (m_inventory)
	{
		std::vector<Gdiplus::RectF> slotRects(INVENTORY_SLOT_COUNT);
		m_inventory->Init(slotRects);
	}

}

void Player::ToggleEquipItem(int slotIndex)
{
	if (slotIndex < 0 || slotIndex >= INVENTORY_SLOT_COUNT)
		return;

	const ItemSlot& targetSlot = m_inventory->GetSlot(slotIndex);
	if (targetSlot.IsEmpty())
		return;

	Tool* toolItem = dynamic_cast<Tool*>(targetSlot.item);
	if (!toolItem)
		return;

	if (m_equippedSlotIndex != -1) {
		if (m_equippedSlotIndex == slotIndex) {
			m_equippedSlotIndex = -1;
			m_equippedItem = nullptr;
		}
		else {
			m_equippedSlotIndex = slotIndex;
			m_equippedItem = targetSlot.item;
		}
	}
	else {
		m_equippedSlotIndex = slotIndex;
		m_equippedItem = targetSlot.item;
	}
}

void Player::Damaged(int damage)
{
}

void Player::UpdateAnimatorState() {

	if (m_animator == nullptr) return;

	m_animator->SetState(static_cast<int>(m_state), this->transform->GetDirection());
}

bool Player::CanInteractWith(GameObject* obj) const
{
	if (!obj || !obj->IsEnabled()) return false;

	GameObjectID objID = obj->GetID();
	GameObjectType objType = obj->GetType();

	switch (objType)
	{
	case GOBJ_NATURAL_ENVIR:
		if (objID == GOID_NORMAL_TREE_SHORT || objID == GOID_NORMAL_TREE_NORMAL || objID == GOID_NORMAL_TREE_TALL) {
			if (!m_equippedItem) return false;
			GameObjectID equippedID = m_equippedItem->GetID();
			return (equippedID == GOID_TOOL_RED_AXE || equippedID == GOID_TOOL_SWAP_AXE);
		}
		// 다른 자연 환경 오브젝트는 상호작용 가능
		return true;
	case GOBJ_ITEM:
		// 아이템은 항상 상호작용 가능
		return true;
	default:
		return false;
	}
}

void Player::SetDirectionToward(float dx, float dy)
{
	Direction dir;
	if (std::abs(dx) > std::abs(dy))
		dir = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
	else
		dir = (dy > 0) ? DIR_DOWN : DIR_UP;
	transform->SetDirection(dir);
}

bool Player::IsArrivedAtTarget(float distance, float moveSpeedThisFrame) const
{
	const float arrivalEpsilon = 1.0f;
	if (distance < arrivalEpsilon) return true;
	if (distance <= m_stopThreshold) return true;
	if (moveSpeedThisFrame > 0.f && distance <= moveSpeedThisFrame) return true;
	return false;
}

void Player::SetTargetPosition(float worldX, float worldY) {

	m_targetWorldPos = Gdiplus::PointF(worldX, worldY);
	m_state = PlayerState::WALK;
	isMoveToGoal = true;

	float dx = worldX - transform->GetX();
	float dy = worldY - transform->GetY();

	Direction newDirection;
	if (std::abs(dx) > std::abs(dy)) {
		newDirection = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
	}
	else {
		newDirection = (dy > 0) ? DIR_DOWN : DIR_UP;
	}

	if (this->transform->GetDirection() != newDirection)
	{
		transform->SetDirection(newDirection);
		UpdateAnimatorState();
	}
}



void Player::Update(float deltaTime)
{
	// 컴포넌트(Animator 등)를 먼저 갱신
	GameObject::Update(deltaTime);

	HandleMovement();

	float moveSpeedThisFrame = m_playerSpeed * deltaTime;

	if (isMoveToGoal)
	{
		float dx = m_targetWorldPos.X - transform->GetX();
		float dy = m_targetWorldPos.Y - transform->GetY();
		float distance = std::sqrt(dx * dx + dy * dy);

		if (IsArrivedAtTarget(distance, moveSpeedThisFrame)) {
			transform->SetX(m_targetWorldPos.X);
			transform->SetY(m_targetWorldPos.Y);
			isMoveToGoal = false;

			// 도착 시 상호작용 처리
			if (m_pendingInteractionTarget && m_pendingInteractionTarget->IsEnabled()) {
				m_activeInteractionTarget = m_pendingInteractionTarget;
				m_pendingInteractionTarget = nullptr;
				SetDirectionToward(dx, dy);
				// OnInteraction의 반환값을 확인하여 실패 시 IDLE 상태로 전환
				if (!OnInteraction(m_activeInteractionTarget)) {
					m_state = PlayerState::IDLE;
					UpdateAnimatorState();
				}
			}
			else {
				// 대기 중인 상호작용이 없거나 유효하지 않으면 IDLE 상태로 전환
				m_pendingInteractionTarget = nullptr;
				m_state = PlayerState::IDLE;
				UpdateAnimatorState();
			}
		}
		else {
			float moveDist = (std::min)(moveSpeedThisFrame, distance);
			transform->SetX(transform->GetX() + (dx / distance) * moveDist);
			transform->SetY(transform->GetY() + (dy / distance) * moveDist);

			// 이동 중에는 기본적으로 WALK 애니메이션을 사용하지만,
			// 이미 PICKUP/CHOP 등 상호작용 애니메이션이 진행 중이면 덮어쓰지 않는다.
			if (m_state == PlayerState::IDLE || m_state == PlayerState::WALK)
			{
				m_state = PlayerState::WALK;
				UpdateAnimatorState();
			}
		}
	}
}

void Player::TryStartInteraction(float worldX, float worldY)
{
	CameraManager* cameraManager = CameraManager::GetInstance();
	if (!cameraManager) return;

	GameObject* target = cameraManager->FindInteractableObjectAtPosition(worldX, worldY);

	if (!target || !CanInteractWith(target) || !target->CanInteract()) {
		return;
	}

	float tx, ty;
	Transform* targetTransform = target->GetComponent<Transform>();
	if (!targetTransform) return;

	// Transform 위치(피벗 기준)로 이동 (콜라이더 중심이 아닌 피벗 위치 사용)
	tx = targetTransform->GetX();
	ty = targetTransform->GetY();

	// 목표 위치 설정 및 이동 시작 (Update에서 도착 시 OnInteraction 호출)
	SetTargetPosition(tx, ty);
	m_pendingInteractionTarget = target;
}


void Player::FinalizePickup()
{
	if (!m_activeInteractionTarget || !m_activeInteractionTarget->IsEnabled()) {
		m_activeInteractionTarget = nullptr;
		return;
	}

	if (!m_inventory) {
		m_activeInteractionTarget = nullptr;
		return;
	}

	GameObjectID objID = m_activeInteractionTarget->GetID();
	GameObjectType objType = m_activeInteractionTarget->GetType();

	bool itemAdded = false;

	if (objType == GOBJ_ITEM) {
		GameObject* itemObj = ObjectManager::GetInstance()->CreateGameObject(objID, 0.0f, 0.0f, nullptr, false);
		Item* item = dynamic_cast<Item*>(itemObj);
		if (item) {
			if (m_inventory->AddItem(item, 1)) {
				itemAdded = true;
				ObjectManager::GetInstance()->RemoveGameObject(m_activeInteractionTarget);
			}
			else {
				// 인벤토리 추가 실패 시 생성된 Item 삭제 (메모리 누수 방지)
				delete item;
			}
		}
	}
	else if (objType == GOBJ_NATURAL_ENVIR) {
		Entity* entity = dynamic_cast<Entity*>(m_activeInteractionTarget);
		GameObjectID itemID = entity ? entity->GetDropItemID() : GOID_NONE;
		int itemCount = entity ? entity->GetDropItemCount() : 0;
		if (itemID != GOID_NONE && itemCount > 0) {
			GameObject* itemObj = ObjectManager::GetInstance()->CreateGameObject(itemID, 0.0f, 0.0f, nullptr, false);
			Item* item = dynamic_cast<Item*>(itemObj);
			if (item) {
				if (m_inventory->AddItem(item, itemCount)) {
					itemAdded = true;
					ObjectManager::GetInstance()->RemoveGameObject(m_activeInteractionTarget);
				}
				else {
					// 인벤토리 추가 실패 시 생성된 Item 삭제 (메모리 누수 방지)
					delete item;
				}
			}
		}
	}

	// 상태 전환은 OnPickupEnd()에서 처리하므로 여기서는 UpdateAnimatorState() 호출하지 않음
	m_activeInteractionTarget = nullptr;
}

void Player::OnPickupEnd()
{
	if (m_state != PlayerState::PICKUP) return;
	// PICKUP 종료 처리: 아이템 획득 및 상태 전환
	FinalizePickup();
	// 상태를 IDLE로 변경하고 애니메이션 업데이트
	m_state = PlayerState::IDLE;
	transform->SetDirection(DIR_DOWN);
	UpdateAnimatorState();
}

void Player::OnChopHit()
{
	if (m_state != PlayerState::CHOP || !m_activeInteractionTarget) return;

	GameObjectID objID = m_activeInteractionTarget->GetID();
	if (objID != GOID_NORMAL_TREE_SHORT && objID != GOID_NORMAL_TREE_NORMAL && objID != GOID_NORMAL_TREE_TALL) return;
	Entity* entity = dynamic_cast<Entity*>(m_activeInteractionTarget);
	if (entity) {
		const int CHOP_DAMAGE = 25;
		entity->Damaged(CHOP_DAMAGE);
		if (entity->IsDead()) m_activeInteractionTarget = nullptr;
	}
}

void Player::OnChopEnd()
{
	if (m_state != PlayerState::CHOP) return;
	// Idle 애니메이션 피벗(0.5f)으로 복구 및 Down 방향으로 설정
	transform->SetPivot(0.5f, transform->GetPivotY());
	transform->SetDirection(DIR_DOWN);
	m_state = PlayerState::IDLE;
	m_activeInteractionTarget = nullptr;
	m_pendingInteractionTarget = nullptr;
	UpdateAnimatorState();
}

bool Player::OnInteraction(GameObject* obj)
{
	// Player 상호작용은 TryStartInteraction 단계에서 이미
	// target->CanInteract(), Player::CanInteractWith(target) 로 검증이 끝났으므로
	// Entity::OnInteraction(GameObject::OnInteraction)을 다시 호출해서
	// "플레이어 자신(this)"의 CanInteract()를 검사할 필요가 없다.
	// (플레이어는 isInteractive=false 이므로 기존 코드는 항상 false를 반환해서
	// PICKUP/CHOP 상태로 진입하지 못했다.)

	GameObjectID objID = obj->GetID();
	GameObjectType objType = obj->GetType();

	switch (objType)
	{
	case GOBJ_NATURAL_ENVIR:
		if (objID == GOID_NORMAL_TREE_SHORT || objID == GOID_NORMAL_TREE_NORMAL || objID == GOID_NORMAL_TREE_TALL) 
		{
			transform->SetDirection(DIR_DOWN);
			m_state = PlayerState::CHOP;
			UpdateAnimatorState();
			return true;				
		}
		else {
			// PICKUP 상태 설정 (애니메이션 이벤트에서 종료 처리)
			m_state = PlayerState::PICKUP;
			UpdateAnimatorState();
			return true;
		}
	case GOBJ_ITEM:
		// PICKUP 상태 설정 (애니메이션 이벤트에서 종료 처리)
		m_state = PlayerState::PICKUP;
		UpdateAnimatorState();
		return true;
	default:
		// 알 수 없는 타입의 경우 IDLE 상태로 전환
		m_state = PlayerState::IDLE;
		UpdateAnimatorState();
		return false;
	}

	return false;
}

void Player::LateUpdate()
{
	// 부모 클래스의 LateUpdate() 호출하여 컴포넌트 업데이트
	GameObject::LateUpdate();
}

void Player::LateInit() {
}

void Player::Release() {
	// Player 전용 정리 작업
	if (m_inventory) {
		delete m_inventory;
		m_inventory = nullptr;
	}
	m_equippedItem = nullptr;
	m_pendingInteractionTarget = nullptr;
	m_activeInteractionTarget = nullptr;

	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	Entity::Release();
}

void Player::HandleRightClick(float worldX, float worldY)
{
	SetTargetPosition(worldX, worldY);
}

// 좌/우클릭 입력 처리. 인벤토리 UI 영역 위 클릭은 월드 상호작용·이동에 사용하지 않음.
void Player::HandleMovement()
{
	InputManager* inputManager = InputManager::GetInstance();
	if (!inputManager)
		return;

	CameraManager* cameraManager = CameraManager::GetInstance();
	if (!cameraManager)
		return;

	bool canStartNewInteraction =
		m_pendingInteractionTarget == nullptr &&
		m_activeInteractionTarget == nullptr &&
		m_state != PlayerState::PICKUP &&
		m_state != PlayerState::CHOP;

	// 좌클릭: 인벤토리 위면 무시, 아니면 해당 월드 좌표로 상호작용 시도(줍기·벌채 등)
	if (inputManager->IsLButtonClicked() && canStartNewInteraction) {
		POINT mousePos = inputManager->GetMousePos();
		if (m_inventory->ContainsScreenPoint(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
			return;
		Gdiplus::PointF worldPos = cameraManager->ScreenToWorld((float)mousePos.x, (float)mousePos.y);
		TryStartInteraction(worldPos.X, worldPos.Y);
	}
	// 우클릭: 인벤토리 위면 이동 안 함(슬롯 위면 장비 토글), 아니면 해당 좌표로 이동
	else if (inputManager->IsRButtonClicked()) {
		m_pendingInteractionTarget = nullptr;
		POINT mousePos = inputManager->GetMousePos();
		if (m_inventory->HandleRightClick(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y), this))
			return;
		Gdiplus::PointF worldPos = cameraManager->ScreenToWorld((float)mousePos.x, (float)mousePos.y);
		HandleRightClick(worldPos.X, worldPos.Y);
	}
}
