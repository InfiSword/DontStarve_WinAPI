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

// Chop 스프라이트는 Idle/Run과 프레임 크기가 달라(284x248 등), 발 위치 정렬을 위해 별도 피벗 사용. 필요 시 값만 조정.
static const float CHOP_PIVOT_X = 0.3f;
static const float CHOP_PIVOT_Y = 0.9f;

Player::Player(float x, float y, GameObjectID characterID, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_PLAYER, characterID, x, y, 0.5f, 1.0f, DIR_DOWN, L"", imageName, true, false),  // baseDir 빈 경우 imageName만 사용(플레이어는 Animator에서 경로 사용)
	hp(100), maxHp(100), m_playerSpeed(300.f), m_stopThreshold(10),
	m_equippedSlotIndex(-1), m_equippedItem(nullptr), m_inventory(nullptr), m_pendingInteractionTarget(nullptr), m_activeInteractionTarget(nullptr), m_state(PlayerState::IDLE), isMoveToGoal(false), m_pickupElapsed(0.f)
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
		126, 189, 7, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
	
	std::wstring idleUpPath = base + L"\\Idle\\Wilson_Idle_Up.png";
	m_animator->RegisterAnimation((int)PlayerState::IDLE, DIR_UP, idleUpPath,
		128, 193, 7, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
	
	std::wstring idleSidePath = base + L"\\Idle\\Wilson_Idle_Side.png";
	m_animator->RegisterAnimation((int)PlayerState::IDLE, DIR_LEFT, idleSidePath,
		135, 194, 7, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, true, 0.03f);
	m_animator->RegisterAnimation((int)PlayerState::IDLE, DIR_RIGHT, idleSidePath,
		135, 194, 7, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);

	// WALK(RUN)
	std::wstring runDownPath = base + L"\\Run\\Wilson_Run_Down.png";
	m_animator->RegisterAnimation((int)PlayerState::WALK, DIR_DOWN, runDownPath,
		139, 226, 6, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
	
	std::wstring runUpPath = base + L"\\Run\\Wilson_Run_Up.png";
	m_animator->RegisterAnimation((int)PlayerState::WALK, DIR_UP, runUpPath,
		133, 231, 6, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
	
	std::wstring runSidePath = base + L"\\Run\\Wilson_Run_Side.png";
	m_animator->RegisterAnimation((int)PlayerState::WALK, DIR_LEFT, runSidePath,
		142, 226, 6, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, true, 0.03f);
	m_animator->RegisterAnimation((int)PlayerState::WALK, DIR_RIGHT, runSidePath,
		141, 226, 6, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);

	// PICKUP
	std::wstring pickupPath = base + L"\\Interact\\Interact_wilson_pickup_pst_down.png";
	for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
		m_animator->RegisterAnimation((int)PlayerState::PICKUP, (Direction)dir, pickupPath,
			127, 201, 6, 20, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.02f);
	}

	// CHOP (이벤트 적용, Idle과 발 위치 맞춤용 피벗)
	std::map<int, std::wstring> chopEvents = { {4, L"chop_hit"} };
	std::wstring chopPath = base + L"\\Axe\\axe_wilson_chop_loop_down.png";
	for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
		m_animator->RegisterAnimation((int)PlayerState::CHOP, (Direction)dir, chopPath,
			284, 248, 6, 36, CHOP_PIVOT_X, CHOP_PIVOT_Y, false, chopEvents, false, 0.01f);
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

// TODO:
// 애니메이션 이벤트 콜백으로 이벤트 설정
//void Player::OnAnimationEvent(int frameIndex, const std::wstring& eventName)
//{
//	if (eventName == L"chop_hit") {
//		if (m_state == PlayerState::CHOP && m_currentInteractionTarget) {
//			// 나무 타입의 오브젝트 ID를 확인
//			GameObjectID objID = m_currentInteractionTarget->GetID();
//			if (objID == GOID_NORMAL_TREE_SHORT || objID == GOID_NORMAL_TREE_NORMAL || objID == GOID_NORMAL_TREE_TALL) {
//				// 도구가 장착되었고 부서지지 않았으면 사용
//				Axe* equippedAxe = dynamic_cast<Axe*>(m_equippedItem);
//				if (equippedAxe && !equippedAxe->IsBroken()) {
//					// Tree의 상태를 변경하기 위해서는 컴포넌트가 필요 (나중에 구현)
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

	if (m_animator == nullptr) return;

	m_animator->SetState(static_cast<int>(m_state), this->transform->GetDirection());
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
	// 컴포넌트(Animator 등)를 먼저 갱신해 IsAnimationDone()이 이번 프레임 기준으로 판정되게 함
	GameObject::Update(deltaTime);

	// CHOP/PICKUP 종료 시 IDLE 전환 및 상호작용 해제 (HandleMovement보다 앞에서 처리)
		if (m_state == PlayerState::CHOP) {
			if (m_animator && m_animator->IsAnimationDone()) {
				// Idle 애니메이션 피벗(0.5f)으로 복구 및 Idle Down 애니메이션으로 전환
				transform->SetPivot(0.5f, transform->GetPivotY());
				transform->SetDirection(DIR_DOWN);
				m_state = PlayerState::IDLE;
				m_activeInteractionTarget = nullptr;
				m_pendingInteractionTarget = nullptr;
				UpdateAnimatorState();
			}
		}
	else if (m_state == PlayerState::PICKUP) {
		m_pickupElapsed += deltaTime;
		if ((m_animator && m_animator->IsAnimationDone()) || m_pickupElapsed >= 1.5f) {
			FinalizePickup();
			m_state = PlayerState::IDLE;
			m_pickupElapsed = 0.f;
			UpdateAnimatorState();
		}
	}

	HandleMovement();

	float moveSpeedThisFrame = m_playerSpeed * deltaTime;

	if (isMoveToGoal)
	{
		float dx = m_targetWorldPos.X - transform->GetX();
		float dy = m_targetWorldPos.Y - transform->GetY();
		float distance = std::sqrt(dx * dx + dy * dy);
		correctValue = 0;

		if (IsArrivedAtTarget(distance, moveSpeedThisFrame)) {
			transform->SetX(m_targetWorldPos.X);
			transform->SetY(m_targetWorldPos.Y);
			isMoveToGoal = false;

			bool hasValidPending = m_pendingInteractionTarget && m_pendingInteractionTarget->IsEnabled();
			Transform* targetTransform = hasValidPending ? m_pendingInteractionTarget->GetComponent<Transform>() : nullptr;
			if (targetTransform) {
				m_activeInteractionTarget = m_pendingInteractionTarget;
				m_pendingInteractionTarget = nullptr;
				SetDirectionToward(dx, dy);
				OnInteraction(m_activeInteractionTarget);
			}
			else {
				m_pendingInteractionTarget = nullptr;
				m_state = PlayerState::IDLE;
				UpdateAnimatorState();
			}
		}
		else {
			float moveDist = (std::min)(moveSpeedThisFrame, distance);
			transform->SetX(transform->GetX() + (dx / distance) * moveDist);
			transform->SetY(transform->GetY() + (dy / distance) * moveDist);
			m_state = PlayerState::WALK;
			UpdateAnimatorState();
		}
	}
}

void Player::TryStartInteraction(float worldX, float worldY)
{
	CameraManager* cameraManager = CameraManager::GetInstance();
	if (!cameraManager) return;

	GameObject* target = cameraManager->FindInteractableObjectAtPosition(worldX, worldY);
	if (!target) return;

	float tx, ty;
	Transform* targetTransform = target->GetComponent<Transform>();
	if (!targetTransform) return;
	
	// Transform 위치(피벗 기준)로 이동 (콜라이더 중심이 아닌 피벗 위치 사용)
	tx = targetTransform->GetX();
	ty = targetTransform->GetY();

	SetTargetPosition(tx, ty);
	m_pendingInteractionTarget = target;
}

void Player::FinalizePickup()
{
	if (!m_activeInteractionTarget || !m_activeInteractionTarget->IsEnabled()) {
		m_activeInteractionTarget = nullptr;
		return;
	}

	GameObjectID objID = m_activeInteractionTarget->GetID();
	GameObjectType objType = m_activeInteractionTarget->GetType();

	if (objType == GOBJ_ITEM) {
		GameObject* itemObj = ObjectManager::GetInstance()->CreateGameObject(objID, 0.0f, 0.0f, nullptr, false);
		Item* item = dynamic_cast<Item*>(itemObj);
		if (item) {
			if (m_inventory->AddItem(item, 1)) {
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
					ObjectManager::GetInstance()->RemoveGameObject(m_activeInteractionTarget);
				}
				else {
					// 인벤토리 추가 실패 시 생성된 Item 삭제 (메모리 누수 방지)
					delete item;
				}
			}
		}
	}

	m_activeInteractionTarget = nullptr;
	transform->SetDirection(DIR_DOWN);
	UpdateAnimatorState();
}

void Player::OnInteraction(GameObject* obj)
{
	if (!obj || !obj->IsEnabled()) {
		m_state = PlayerState::IDLE;
		return;
	}

	GameObjectID objID = obj->GetID();
	GameObjectType objType = obj->GetType();

	switch (objType)
	{
	case GOBJ_NATURAL_ENVIR:
		if (objID == GOID_NORMAL_TREE_SHORT || objID == GOID_NORMAL_TREE_NORMAL || objID == GOID_NORMAL_TREE_TALL) {
			// CHOP 애니메이션 피벗으로 변경
			transform->SetPivot(CHOP_PIVOT_X, CHOP_PIVOT_Y);
			m_state = PlayerState::CHOP;
		}
		else {
			m_state = PlayerState::PICKUP;
			m_pickupElapsed = 0.f;
		}
		correctValue = 10;
		UpdateAnimatorState();
		break;
	case GOBJ_ITEM:
		m_state = PlayerState::PICKUP;
		m_pickupElapsed = 0.f;
		correctValue = 10;
		UpdateAnimatorState();
		break;
	default:
		break;
	}
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

void Player::HandleMovement()
{
	InputManager* inputManager = InputManager::GetInstance();
	if (!inputManager)
		return;

	// 상호작용: 좌클릭만. 한 번 상호작용이 시작되면 완료될 때까지 새 진입 불가 (연타/재클릭 크래시 방지)
	bool canStartNewInteraction =
		m_pendingInteractionTarget == nullptr &&
		m_activeInteractionTarget == nullptr &&
		m_state != PlayerState::PICKUP &&
		m_state != PlayerState::CHOP;
	if (inputManager->IsLButtonClicked() && canStartNewInteraction) {
		POINT mousePos = inputManager->GetMousePos();
		CameraManager* cameraManager = CameraManager::GetInstance();
		if (cameraManager) {
			Gdiplus::PointF worldPos = cameraManager->ScreenToWorld((float)mousePos.x, (float)mousePos.y);
			TryStartInteraction(worldPos.X, worldPos.Y);
		}
	}
	// 우클릭: 이동만 (대기 중인 상호작용은 취소)
	if (inputManager->IsRButtonClicked()) {
		m_pendingInteractionTarget = nullptr;
		POINT mousePos = inputManager->GetMousePos();
		float mouseX = static_cast<float>(mousePos.x);
		float mouseY = static_cast<float>(mousePos.y);
		CameraManager* cameraManager = CameraManager::GetInstance();
		if (cameraManager) {
			Gdiplus::PointF worldPos = cameraManager->ScreenToWorld(mouseX, mouseY);
			HandleRightClick(worldPos.X, worldPos.Y);
		}
	}
}
