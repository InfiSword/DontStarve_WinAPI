#include "../../99_Default/pch.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/AnimationClip.h"
#include "../../02_GameObject/Player/Player.h"
#include "../../03_Animation/SpriteSheet.h"
#include "../../../Header/Struct.h"
#include "Tree.h"

Tree::Tree(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity<TreeState>(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName), m_hp(100),  m_hitAnimTimer(0.0f)
{
	m_animator = nullptr;
	maxHp = m_hp;
}

Tree::~Tree() {}

void Tree::Init()
{
	SetActive(true);
	SetInteractive(true); // Tree는 상호작용 가능
	m_direction = DIR_DOWN;
	m_state = TREE_IDLE;
	m_animator = new Animator();
	
	OutputDebugStringW((L"Tree: Init 시작 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	
	RegisterAllAnimations(); // Unity Animator 스타일
	UpdateAnimatorState(); // 초기 상태 설정
	
	// 초기 크기 설정 (애니메이션 등록 후 첫 프레임에서 크기 가져오기)
	if (m_animator) {
		const AnimationFrame& frame = m_animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
		
		// Animator 상태 확인
		const SpriteSheet* spriteSheet = m_animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Tree: Animator 초기화 성공 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 있음\n").c_str());
		} else {
			OutputDebugStringW((L"Tree: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Tree: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

void Tree::LateInit()
{
}

void Tree::Update(float deltaTime)
{
	if (m_state == TREE_CHOP) {
		m_hitAnimTimer += deltaTime;
		if (m_hitAnimTimer >= m_animator->GetCurrentClipTotalDuration()) {
			m_state = TREE_IDLE; 
			UpdateAnimatorState();
			m_hitAnimTimer = 0.0f; 
		}
	}

	if (m_animator) {
		const AnimationFrame& frame = m_animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
	}
	UpdateAnimation(deltaTime);
}

void Tree::LateUpdate()
{
}

void Tree::Render(Gdiplus::Graphics* pGraphics)
{
    // RenderManager::RenderGameObject()에서 UpdateAnimation()과 GetBitmap()을 호출하여 렌더링
    // 개별 GameObject의 Render() 함수는 더 이상 필요하지 않음
}

void Tree::Release()
{
	SafeDelete(m_animator);
}

// Unity Animator 스타일 애니메이션 등록
void Tree::RegisterAllAnimations()
{
	// ResourceManager를 사용하여 경로 구성
	auto* pRM = ResourceManager::GetInstance();
	
	// 나무 종류에 따른 분기
	if (m_id == GOID_NORMAL_TREE_NORMAL)
	{
		tree_Grade = L"Normal";
		
		// IDLE 애니메이션 (실제 파일명 사용)
		m_animator->RegisterAnimation(TREE_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_TREE_NORMAL, L"", L"evergreen_evergreen_short_idle_normal_01.png"),
			384, 571, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		// CHOP 애니메이션  
		m_animator->RegisterAnimation(TREE_CHOP, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_TREE_NORMAL, L"", L"Tree1_evergreen_normal_chop_normal.png"),
			399, 581, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
		
		// FALL 애니메이션
		m_animator->RegisterAnimation(TREE_FALL, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_TREE_NORMAL, L"", L"Tree1_evergreen_normal_fallleft_normal.png"),
			722, 713, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
	}
	else if (m_id == GOID_NORMAL_TREE_TALL)
	{
		tree_Grade = L"Tall";
		
		// IDLE 애니메이션 (실제 파일명 사용)
		m_animator->RegisterAnimation(TREE_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_TREE_TALL, L"", L"evergreen_evergreen_short_idle_tall_01.png"),
			500, 743, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		// CHOP 애니메이션 (실제 파일명 사용)
		m_animator->RegisterAnimation(TREE_CHOP, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_TREE_TALL, L"", L"Tree1_evergreen_chop_tall.png"),
			533, 776, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
		
		// FALL 애니메이션 (실제 파일명 사용)
		m_animator->RegisterAnimation(TREE_FALL, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_TREE_TALL, L"", L"Tree1_evergreen_fallleft_tall.png"),
			934, 996, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
	}
	else if (m_id == GOID_NORMAL_TREE_SHORT)
	{
		tree_Grade = L"Short";
		
		// IDLE 애니메이션 (실제 파일명 사용)
		m_animator->RegisterAnimation(TREE_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_TREE_SHORT, L"", L"evergreen_evergreen_short_idle_short_01.png"),
			181, 467, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		// CHOP 애니메이션 (실제 파일명 사용)
		m_animator->RegisterAnimation(TREE_CHOP, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_TREE_SHORT, L"", L"Tree1_evergreen_short_chop_short.png"),
			195, 472, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
		
		// FALL 애니메이션 (실제 파일명 사용)
		m_animator->RegisterAnimation(TREE_FALL, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_TREE_SHORT, L"", L"Tree1_evergreen_short_fallleft_short.png"),
			565, 526, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
	}

	
	OutputDebugStringW(L"Tree: Unity Animator 스타일로 모든 애니메이션 등록 완료\n");
}

// Unity Animator 스타일 상태 업데이트
void Tree::UpdateAnimatorState()
{
	if (m_animator) {
		m_animator->SetState(m_state, m_direction);
	}
}

void Tree::UpdateAnimation(float deltaTime) 
{
	if (m_animator)
		m_animator->Update(deltaTime);
}


Gdiplus::Bitmap* Tree::GetBitmap() const
{
    if (!m_animator) return nullptr;
    
    const SpriteSheet* spriteSheet = m_animator->GetSpriteSheet();
    if (!spriteSheet) return nullptr;
    
    return spriteSheet->GetBitmap();
}

void Tree::OnInteraction(GameObject* obj)
{
	// 기본 구현
}

void Tree::OnPlayerInteraction(Player* player)
{
	player->OnInteraction(this);
}

void Tree::Damaged(int damage)
{
	m_hp -= damage;
	m_state = TREE_CHOP;
	UpdateAnimatorState(); // Unity Animator가 자동으로 애니메이션 선택
	
	if (m_hp <= 0) {
		m_state = TREE_FALL;
		UpdateAnimatorState();
		
		// 아이템 드롭 로직
		// 나무가 쓰러지면 로그 아이템 생성
		OutputDebugStringW(L"Tree: 나무가 쓰러졌습니다!\n");
	}
}
