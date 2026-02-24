#include "99_Default/pch.h"
#include "Player.h"
#include "../../../01_Manager/InputManager/InputManager.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../02_GameObject/UI/Inventory.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../Item/Tool/Tool.h"
#include "../../Component/Transform/Transform.h"
#include "../../Component/Collider/Collider.h"
#include "../../Component/Collider/BoxCollider.h"
#include "../../../01_Manager/ColliderManager/ColliderManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../Entity/Monster/Monster.h"

static const float CHOP_PIVOT_X = 0.3f;
static const float CHOP_PIVOT_Y = 0.9f;
static const float MINE_PIVOT_X = 0.5f;
static const float MINE_PIVOT_Y = 0.9f;
static const float ATTACK_RANGE = 80.0f;  // 몬스터와 이 거리 이내면 공격 시작
// 기본 IDLE 이미지 크기 (Wilson_Idle_Down: 126x189, pivot 0.5/1.0 → 로컬 왼쪽위 -63, -189)
static const int IDLE_FRAME_WIDTH = 126;
static const int IDLE_FRAME_HEIGHT = 189;
// 공격 콜라이더 (로컬): 바라보는 방향에 따른 전방 박스 (offsetX, offsetY, width, height)
static const int ATTACK_BOX_W = 80, ATTACK_BOX_H = 60;
static const int ATTACK_BOX_DOWN[]  = { -40,    0, ATTACK_BOX_W, ATTACK_BOX_H };  // 발 앞
static const int ATTACK_BOX_UP[]    = { -40,  -60, ATTACK_BOX_W, ATTACK_BOX_H };  // 머리 쪽
static const int ATTACK_BOX_LEFT[]  = { -80,  -30, ATTACK_BOX_W, ATTACK_BOX_H };  // 왼쪽
static const int ATTACK_BOX_RIGHT[] = {   0,  -30, ATTACK_BOX_W, ATTACK_BOX_H };  // 오른쪽

Player::Player(float x, float y, GameObjectID characterID, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_PLAYER, characterID, x, y, 0.5f, 1.0f, DIR_DOWN, L"", imageName, true, false),
	hp(100), maxHp(100), m_playerSpeed(300.f), m_stopThreshold(10),
	m_equippedSlotIndex(-1), m_equippedItem(nullptr), m_inventory(nullptr), m_pendingInteractionTarget(nullptr), m_activeInteractionTarget(nullptr), m_attackTarget(nullptr), m_attackCollider(nullptr), m_state(PlayerState::IDLE), isMoveToGoal(false)
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

	// MINE (곡괭이 채광, Down 방향 고정 — Chop과 동일 패턴)
	const UINT MINE_TOTAL_FRAMES = 39;
	const int MINE_HIT_FRAME = 4;
	const int MINE_LAST_FRAME = MINE_TOTAL_FRAMES - 1; // 50
	std::wstring pickaxePath = base + L"\\Pickaxe\\pickaxe_wilson_pickaxe_loop_down.png";
	for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
		m_animator->RegisterAnimation((int)PlayerState::MINE, (Direction)dir, pickaxePath,
			/*311*/0, /*360*/ 0, 6, MINE_TOTAL_FRAMES, MINE_PIVOT_X, MINE_PIVOT_Y, false, 0.01f);
		AnimationClip* clip = m_animator->GetAnimationClip((int)PlayerState::MINE, (Direction)dir);
		if (clip) {
			clip->AddEventFrame(MINE_HIT_FRAME, L"mine_hit");
			clip->AddEventFrame(MINE_LAST_FRAME, L"mine_end");
			clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
				if (eventName == L"mine_hit") {
					this->OnMineHit();
				}
				else if (eventName == L"mine_end") {
					this->OnMineEnd();
				}
				});
		}
	}

	// ATTACK (4열 36프레임, 6번째 프레임에 attack_hit, 마지막에 attack_end)
	const UINT ATTACK_TOTAL_FRAMES = 36;
	const int ATTACK_HIT_FRAME = 5;
	const int ATTACK_LAST_FRAME = ATTACK_TOTAL_FRAMES - 1;
	std::wstring attackDownPath = base + L"\\Attack\\Wilson_Attack_down.png";
	std::wstring attackUpPath = base + L"\\Attack\\Wilson_Attack_up.png";
	std::wstring attackSidePath = base + L"\\Attack\\Wilson_Attack_side.png";
	m_animator->RegisterAnimation((int)PlayerState::ATTACK, DIR_DOWN, attackDownPath,
		0, 0, 4, ATTACK_TOTAL_FRAMES, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
	m_animator->RegisterAnimation((int)PlayerState::ATTACK, DIR_UP, attackUpPath,
		0, 0, 4, ATTACK_TOTAL_FRAMES, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
	m_animator->RegisterAnimation((int)PlayerState::ATTACK, DIR_LEFT, attackSidePath,
		0, 0, 4, ATTACK_TOTAL_FRAMES, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f, false);
	m_animator->RegisterAnimation((int)PlayerState::ATTACK, DIR_RIGHT, attackSidePath,
		0, 0, 4, ATTACK_TOTAL_FRAMES, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
	for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
		AnimationClip* clip = m_animator->GetAnimationClip((int)PlayerState::ATTACK, (Direction)dir);
		if (clip) {
			clip->AddEventFrame(ATTACK_HIT_FRAME, L"attack_hit");
			clip->AddEventFrame(ATTACK_LAST_FRAME, L"attack_end");
			clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
				if (eventName == L"attack_hit") this->OnAttackHit();
				else if (eventName == L"attack_end") this->OnAttackEnd();
				});
		}
	}

	// 기본 idle 이미지 크기의 콜라이더 (Pig와 동일하게 상호작용/클릭 감지 등에 사용)
	BoxCollider* bodyCollider = AddComponent<BoxCollider>();
	if (bodyCollider) {
		int left = -(IDLE_FRAME_WIDTH / 2);
		int top = -IDLE_FRAME_HEIGHT;
		bodyCollider->SetBoundingBox(left, top, IDLE_FRAME_WIDTH, IDLE_FRAME_HEIGHT);
		bodyCollider->SetColliderEnabled(true);
	}

	// 공격 판정용 콜라이더 (기본 비활성, ATTACK 6프레임 시에만 활성)
	m_attackCollider = AddComponent<BoxCollider>();
	if (m_attackCollider) {
		m_attackCollider->SetBoundingBox(-40, 0, 80, 60);
		m_attackCollider->SetColliderEnabled(false);
	}

	if (!m_inventory) {
		m_inventory = new Inventory();
	}

	UpdateAnimatorState();

	if (m_inventory)
		m_inventory->Init();
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
	if (damage <= 0) return;
	hp = (std::max)(0, hp - damage);
	if (hp <= 0) {
		m_isDead = true;
		Die();
	}
}

void Player::Heal(int amount)
{
	if (amount <= 0) return;
	hp = (std::min)(maxHp, hp + amount);
}

void Player::UpdateAnimatorState() {

	if (m_animator == nullptr) return;

	m_animator->SetState(static_cast<int>(m_state), this->transform->GetDirection());
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
	GameObject::Update(deltaTime);

	HandleMovement();

	// ATTACK 상태: 방향에 따라 공격 콜라이더 위치 갱신, 6프레임(인덱스 5)일 때만 활성화
	if (m_state == PlayerState::ATTACK && m_attackCollider && m_animator && transform) {
		Direction dir = transform->GetDirection();
		if (dir == DIR_DOWN) m_attackCollider->SetBoundingBox(ATTACK_BOX_DOWN[0], ATTACK_BOX_DOWN[1], ATTACK_BOX_DOWN[2], ATTACK_BOX_DOWN[3]);
		else if (dir == DIR_UP) m_attackCollider->SetBoundingBox(ATTACK_BOX_UP[0], ATTACK_BOX_UP[1], ATTACK_BOX_UP[2], ATTACK_BOX_UP[3]);
		else if (dir == DIR_LEFT) m_attackCollider->SetBoundingBox(ATTACK_BOX_LEFT[0], ATTACK_BOX_LEFT[1], ATTACK_BOX_LEFT[2], ATTACK_BOX_LEFT[3]);
		else m_attackCollider->SetBoundingBox(ATTACK_BOX_RIGHT[0], ATTACK_BOX_RIGHT[1], ATTACK_BOX_RIGHT[2], ATTACK_BOX_RIGHT[3]);

		int frameIdx = m_animator->GetCurrentFrameIndex();
		m_attackCollider->SetColliderEnabled(frameIdx == 5);
	}
	else if (m_attackCollider) {
		m_attackCollider->SetColliderEnabled(false);
	}

	float moveSpeedThisFrame = m_playerSpeed * deltaTime;

	if (isMoveToGoal)
	{
		float dx = m_targetWorldPos.X - transform->GetX();
		float dy = m_targetWorldPos.Y - transform->GetY();
		float distance = std::sqrt(dx * dx + dy * dy);

		// 공격 대상으로 이동 중일 때: 사거리 내면 이동 중단 후 ATTACK
		if (m_attackTarget && m_attackTarget->IsEnabled()) {
			Transform* targetT = m_attackTarget->GetComponent<Transform>();
			if (targetT) {
				float ax = targetT->GetX() - transform->GetX();
				float ay = targetT->GetY() - transform->GetY();
				float distToTarget = std::sqrt(ax * ax + ay * ay);
				if (distToTarget <= ATTACK_RANGE) {
					isMoveToGoal = false;
					Direction faceDir = (std::abs(ax) > std::abs(ay)) ? (ax > 0 ? DIR_RIGHT : DIR_LEFT) : (ay > 0 ? DIR_DOWN : DIR_UP);
					transform->SetDirection(faceDir);
					m_state = PlayerState::ATTACK;
					UpdateAnimatorState();
					return;
				}
			}
		}

		// 도착 여부 판정 (인라인화: IsArrivedAtTarget)
		const float arrivalEpsilon = 1.0f;
		bool isArrived = (distance < arrivalEpsilon) || (distance <= m_stopThreshold) || (moveSpeedThisFrame > 0.f && distance <= moveSpeedThisFrame);
		
		if (isArrived) {
			transform->SetX(m_targetWorldPos.X);
			transform->SetY(m_targetWorldPos.Y);
			isMoveToGoal = false;

			// 공격 대상으로 도착한 경우: 방향만 맞추고 ATTACK
			if (m_attackTarget && m_attackTarget->IsEnabled()) {
				Transform* targetT = m_attackTarget->GetComponent<Transform>();
				if (targetT) {
					float ax = targetT->GetX() - transform->GetX();
					float ay = targetT->GetY() - transform->GetY();
					Direction faceDir = (std::abs(ax) > std::abs(ay)) ? (ax > 0 ? DIR_RIGHT : DIR_LEFT) : (ay > 0 ? DIR_DOWN : DIR_UP);
					transform->SetDirection(faceDir);
					m_state = PlayerState::ATTACK;
					UpdateAnimatorState();
					return;
				}
			}

			// 도착 시 상호작용 처리
			if (m_pendingInteractionTarget && m_pendingInteractionTarget->IsEnabled()) {
				m_activeInteractionTarget = m_pendingInteractionTarget;
				m_pendingInteractionTarget = nullptr;
				// 방향 설정 (인라인화: SetDirectionToward)
				Direction dir;
				if (std::abs(dx) > std::abs(dy))
					dir = (dx > 0) ? DIR_RIGHT : DIR_LEFT;
				else
					dir = (dy > 0) ? DIR_DOWN : DIR_UP;
				transform->SetDirection(dir);
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

	// CHOP/MINE/PICKUP 진행 중인 경우
	if (m_state == PlayerState::CHOP || m_state == PlayerState::MINE || m_state == PlayerState::PICKUP) {
		if (target != nullptr && target == m_activeInteractionTarget) {
			// 현재 상호작용 중인 대상과 동일 → 애니메이션 유지 (재시작 안 함)
			return;
		}
		// 다른 대상 또는 빈 공간 클릭 → 현재 상호작용 중단
		m_activeInteractionTarget = nullptr;
		m_pendingInteractionTarget = nullptr;
		m_state = PlayerState::IDLE;
		transform->SetPivot(0.5f, transform->GetPivotY());
		UpdateAnimatorState();
	}

	// 이동 중 대기 중인 상호작용 초기화
	m_pendingInteractionTarget = nullptr;

	// 상호작용 가능 여부 확인
	bool canInteract = false;
	if (target && target->IsEnabled()) {
		GameObjectID objID = target->GetID();
		GameObjectType objType = target->GetType();
		switch (objType) {
		case GOBJ_NATURAL_ENVIR:
			if (objID == GOID_NORMAL_TREE_SHORT || objID == GOID_NORMAL_TREE_NORMAL || objID == GOID_NORMAL_TREE_TALL) {
				if (m_equippedItem) {
					GameObjectID equippedID = m_equippedItem->GetID();
					canInteract = (equippedID == GOID_TOOL_RED_AXE || equippedID == GOID_TOOL_SWAP_AXE);
				}
			}
			else if (objID == GOID_NORMAL_ROCK || objID == GOID_GOLD_ROCK) {
				if (m_equippedItem && m_equippedItem->GetID() == GOID_TOOL_PICKAXE)
					canInteract = true;
			}
			else {
				canInteract = true;
			}
			break;
		case GOBJ_ITEM:
			canInteract = true;
			break;
		case GOBJ_MONSTER:
			canInteract = true;  // 몬스터 클릭 시 공격 대상으로 이동
			break;
		default:
			canInteract = false;
			break;
		}
	}

	if (!target || !canInteract || !target->CanInteract()) {
		return;
	}

	Transform* targetTransform = target->GetComponent<Transform>();
	if (!targetTransform) return;

	SetTargetPosition(targetTransform->GetX(), targetTransform->GetY());
	if (target->GetType() == GOBJ_MONSTER) {
		m_attackTarget = target;
	}
	else {
		m_pendingInteractionTarget = target;
	}
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
	if (objID != GOID_NORMAL_TREE_SHORT && objID != GOID_NORMAL_TREE_NORMAL && objID != GOID_NORMAL_TREE_TALL)
		return;
	Entity* entity = dynamic_cast<Entity*>(m_activeInteractionTarget);
	if (!entity) return;
	Tool* axe = dynamic_cast<Tool*>(m_equippedItem);
	int damage = axe ? (int)axe->GetDamage() : 10;
	entity->Damaged(damage);
	if (entity->IsDead()) m_activeInteractionTarget = nullptr;
}

void Player::OnChopEnd()
{
	if (m_state != PlayerState::CHOP) return;

	transform->SetPivot(0.5f, transform->GetPivotY());
	transform->SetDirection(DIR_DOWN);
	m_state = PlayerState::IDLE;
	m_activeInteractionTarget = nullptr;
	m_pendingInteractionTarget = nullptr;

	UpdateAnimatorState();
}

void Player::OnMineHit()
{
	if (m_state != PlayerState::MINE || !m_activeInteractionTarget) return;

	GameObjectID objID = m_activeInteractionTarget->GetID();
	if (objID != GOID_NORMAL_ROCK && objID != GOID_GOLD_ROCK)
		return;
	Entity* entity = dynamic_cast<Entity*>(m_activeInteractionTarget);
	if (!entity) return;
	Tool* pickaxe = dynamic_cast<Tool*>(m_equippedItem);
	int damage = pickaxe ? (int)pickaxe->GetDamage() : 10;
	entity->Damaged(damage);
	if (entity->IsDead()) m_activeInteractionTarget = nullptr;
}

void Player::OnMineEnd()
{
	if (m_state != PlayerState::MINE) return;

	transform->SetPivot(0.5f, transform->GetPivotY());
	transform->SetDirection(DIR_DOWN);
	m_state = PlayerState::IDLE;
	m_activeInteractionTarget = nullptr;
	m_pendingInteractionTarget = nullptr;

	UpdateAnimatorState();
}

void Player::OnAttackHit()
{
	if (m_state != PlayerState::ATTACK || !m_attackCollider || !m_attackCollider->IsEnabled()) return;

	int damage = 10;
	Tool* tool = dynamic_cast<Tool*>(m_equippedItem);
	if (tool) damage = (int)tool->GetDamage();

	std::vector<GameObject*> hitTargets;
	ColliderManager::GetInstance()->GetObjectsIntersecting(m_attackCollider, hitTargets);
	for (GameObject* obj : hitTargets) {
		Monster* m = dynamic_cast<Monster*>(obj);
		if (m && m->IsEnabled()) m->Damaged(damage);
	}
}

void Player::OnAttackEnd()
{
	if (m_state != PlayerState::ATTACK) return;
	if (m_attackCollider) m_attackCollider->SetColliderEnabled(false);
	m_state = PlayerState::IDLE;
	m_attackTarget = nullptr;
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
		if (objID == GOID_NORMAL_ROCK || objID == GOID_GOLD_ROCK)
		{
			transform->SetDirection(DIR_DOWN);
			m_state = PlayerState::MINE;
			UpdateAnimatorState();
			return true;
		}
		{
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
	if (GetHp() <= 0)
		return;

	InputManager* inputManager = InputManager::GetInstance();
	if (!inputManager)
		return;

	CameraManager* cameraManager = CameraManager::GetInstance();
	if (!cameraManager)
		return;

	// Space: 현재 방향으로 공격 (CHOP/MINE/PICKUP/ATTACK 중이 아닐 때만)
	if (inputManager->IsKeyPressed(VK_SPACE)) {
		if (m_state != PlayerState::CHOP && m_state != PlayerState::MINE && m_state != PlayerState::PICKUP && m_state != PlayerState::ATTACK) {
			m_state = PlayerState::ATTACK;
			UpdateAnimatorState();
		}
	}

	if (inputManager->IsLButtonClicked()) {
		POINT mousePos = inputManager->GetMousePos();
		if (m_inventory->ContainsScreenPoint(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y)))
			return;

		Gdiplus::PointF worldPos = cameraManager->ScreenToWorld((float)mousePos.x, (float)mousePos.y);
		TryStartInteraction(worldPos.X, worldPos.Y);
	}
	else if (inputManager->IsRButtonClicked()) {
		m_pendingInteractionTarget = nullptr;
		POINT mousePos = inputManager->GetMousePos();
		if (m_inventory->HandleRightClick(static_cast<float>(mousePos.x), static_cast<float>(mousePos.y), this))
			return;
		Gdiplus::PointF worldPos = cameraManager->ScreenToWorld((float)mousePos.x, (float)mousePos.y);
		HandleRightClick(worldPos.X, worldPos.Y);
	}
}

void Player::RenderDebugOverlay()
{
	// ATTACK 상태일 때 공격 콜라이더 위치를 시각화 (방향에 따라 이미 Update에서 박스가 설정됨)
	if (m_state != PlayerState::ATTACK || !m_attackCollider || !transform) return;

	CameraManager* cameraManager = CameraManager::GetInstance();
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!cameraManager || !renderManager) return;

	RECT worldBox = m_attackCollider->GetWorldBoundingBox();
	Gdiplus::PointF screenTopLeft = cameraManager->WorldToScreen((float)worldBox.left, (float)worldBox.top);
	Gdiplus::PointF screenBottomRight = cameraManager->WorldToScreen((float)worldBox.right, (float)worldBox.bottom);
	float w = screenBottomRight.X - screenTopLeft.X;
	float h = screenBottomRight.Y - screenTopLeft.Y;
	Gdiplus::RectF rect(screenTopLeft.X, screenTopLeft.Y, w, h);

	Gdiplus::Color fillColor(50, 255, 165, 0);   // 반투명 주황
	Gdiplus::Color lineColor(255, 255, 165, 0); // 주황 외곽선
	renderManager->AddFillRectangleCommand(rect, fillColor, LAYER_DEBUG_OVERLAY, 9998.0f);
	renderManager->AddDrawCommand(rect, lineColor, 2.0f, LAYER_DEBUG_OVERLAY, 9999.0f);
}
