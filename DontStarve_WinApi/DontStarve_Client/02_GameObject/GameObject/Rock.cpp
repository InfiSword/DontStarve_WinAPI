#include "../../99_Default/pch.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/AnimationClip.h"
#include "../../02_GameObject/Player/Player.h"
#include "../../03_Animation/SpriteSheet.h"
#include "../../../Header/Struct.h"
#include "Rock.h"

Rock::Rock(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity<RockState>(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName), m_hp(100), m_hitAnimTimer(0.0f)
{
	m_animator = nullptr;
	maxHp = m_hp;
}

Rock::~Rock() {}

void Rock::Init()
{
	SetActive(true);
	SetInteractive(true); // Rock은 상호작용 가능
	m_direction = DIR_DOWN;
	m_state = ROCK_INTACT;
	m_animator = new Animator();
	RegisterAllAnimations(); // Unity Animator 스타일
	UpdateAnimatorState(); // 초기 상태 설정
	
	// 초기 크기 설정 (애니메이션 등록 후 첫 프레임에서 크기 가져오기)
	if (m_animator) {
		const AnimationFrame& frame = m_animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
	}
}

void Rock::LateInit()
{
}

void Rock::Update(float deltaTime)
{
	/*if (m_state == ROCK_HIT) {
		m_hitAnimTimer += deltaTime;
		if (m_hitAnimTimer >= m_animator->GetCurrentClipTotalDuration()) {
			m_state = ROCK_INTACT;
			UpdateAnimatorState();
			m_hitAnimTimer = 0.0f; 
		}
	}*/

	if (m_animator) {
		const AnimationFrame& frame = m_animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
	}
	UpdateAnimation(deltaTime);
}

void Rock::LateUpdate()
{
}

void Rock::Render(Gdiplus::Graphics* pGraphics)
{
    // RenderManager::RenderGameObject()에서 UpdateAnimation()과 GetBitmap()을 호출하여 렌더링
    // 개별 GameObject의 Render() 함수는 더 이상 필요하지 않음
}

void Rock::Release()
{
	SafeDelete(m_animator);
}

// Unity Animator 스타일 애니메이션 등록
void Rock::RegisterAllAnimations()
{
	// ResourceManager를 사용하여 경로 구성
	auto* pRM = ResourceManager::GetInstance();
	
	// 바위 종류에 따른 분기
	if (m_id == GOID_NORMAL_ROCK)
	{
		// IDLE 애니메이션 (일반 바위)
		m_animator->RegisterAnimation(ROCK_INTACT, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_ROCK, L"", L"rock01-0.png"),
			112, 84, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		// ROCK_CRACKED 애니메이션
		m_animator->RegisterAnimation(ROCK_CRACKED, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_ROCK, L"", L"rock01-1.png"),
			112, 84, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
		
		// BROKEN 애니메이션
		m_animator->RegisterAnimation(ROCK_BROKEN, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_ROCK, L"", L"rock01-2.png"),
			112, 84, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
	}
	else if (m_id == GOID_GOLD_ROCK)
	{
		// IDLE 애니메이션 (금 바위)
		m_animator->RegisterAnimation(ROCK_INTACT, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_GOLD_ROCK, L"", L"rock02-0.png"),
			112, 84, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		// ROCK_CRACKED 애니메이션
		m_animator->RegisterAnimation(ROCK_CRACKED, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_GOLD_ROCK, L"", L"rock02-1.png"),
			112, 84, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
		
		// BROKEN 애니메이션
		m_animator->RegisterAnimation(ROCK_BROKEN, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_GOLD_ROCK, L"", L"rock02-2.png"),
			112, 84, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
	}
	
	OutputDebugStringW(L"Rock: Unity Animator 스타일로 모든 애니메이션 등록 완료\n");
}

// Unity Animator 스타일 상태 업데이트
void Rock::UpdateAnimatorState()
{
	if (m_animator) {
		m_animator->SetState(m_state, m_direction);
	}
}

void Rock::UpdateAnimation(float deltaTime) 
{
	if (m_animator)
		m_animator->Update(deltaTime);
}



Gdiplus::Bitmap* Rock::GetBitmap() const
{
    if (!m_animator) return nullptr;
    
    const SpriteSheet* spriteSheet = m_animator->GetSpriteSheet();
    if (!spriteSheet) return nullptr;
    
    return spriteSheet->GetBitmap();
}

void Rock::OnInteraction(GameObject* obj)
{
	// 기본 구현
}

void Rock::OnPlayerInteraction(Player* player)
{
	player->OnInteraction(this);
}

void Rock::Damaged(int damage)
{
	m_hp -= damage;
	// m_state = ROCK_HIT;
	UpdateAnimatorState(); // Unity Animator가 자동으로 애니메이션 선택
	
	if (m_hp <= 0) {
		m_state = ROCK_BROKEN;
		UpdateAnimatorState();
		
		// 아이템 드롭 로직
		// 바위가 부서지면 돌 조각 아이템 생성
		OutputDebugStringW(L"Rock: 바위가 부서졌습니다!\n");
	}
}
