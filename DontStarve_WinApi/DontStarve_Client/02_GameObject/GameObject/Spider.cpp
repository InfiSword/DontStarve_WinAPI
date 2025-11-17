#include "../../99_Default/pch.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/AnimationClip.h"
#include "../../02_GameObject/Player/Player.h"
#include "../../03_Animation/SpriteSheet.h"
#include "../../../Header/Struct.h"
#include "Spider.h"

Spider::Spider(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity<MonsterState>(GOBJ_MONSTER, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName), m_hp(80), m_hitAnimTimer(0.0f)
{
    m_animator = nullptr;
	maxHp = m_hp;
}

Spider::~Spider() {}

void Spider::Init()
{
	SetActive(true);
	m_direction = DIR_DOWN;
	m_state = MONSTER_IDLE;
	m_animator = new Animator();
	
	OutputDebugStringW((L"Spider: Init 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	
	RegisterAllAnimations(); // Unity Animator 스타일
	UpdateAnimatorState(); // 초기 상태 설정
	
	// 초기 크기 설정 (애니메이션 프레임의 첫 번째 프레임에서 크기 가져오기)
	if (m_animator) {
		const AnimationFrame& frame = m_animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
		
		// Animator 상태 확인
		const SpriteSheet* spriteSheet = m_animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Spider: Animator 초기화 완료 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
		} else {
			OutputDebugStringW((L"Spider: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Spider: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

void Spider::LateInit()
{
}

void Spider::Update(float deltaTime)
{
	if (m_state == MONSTER_HIT) {
		m_hitAnimTimer += deltaTime;
		if (m_hitAnimTimer >= m_animator->GetCurrentClipTotalDuration()) {
			m_state = MONSTER_IDLE; 
			UpdateAnimatorState();
			m_hitAnimTimer = 0.0f; 
		}
	}

	if (m_animator) {
		const AnimationFrame& frame = m_animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
	}
}

void Spider::LateUpdate()
{
}

void Spider::Render(Gdiplus::Graphics* pGraphics)
{
    // RenderManager::RenderGameObject()에서 UpdateAnimation()과 GetBitmap()을 호출하여 렌더링
    // 따라서 GameObject의 Render() 함수는 더 이상 필요하지 않음
}

void Spider::Release()
{
	SafeDelete(m_animator);
}

// Unity Animator 스타일 애니메이션 등록
void Spider::RegisterAllAnimations()
{
	// ResourceManager를 사용하여 리소스 로드
	auto* pRM = ResourceManager::GetInstance();
	
	// SPIDER 애니메이션 등록
	if (m_id == GOID_MONSTER_SPIDER)
	{
		// IDLE 애니메이션들
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		// WALK 애니메이션들
		m_animator->RegisterAnimation(MONSTER_WALK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_down.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_WALK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_up.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_WALK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_side.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_WALK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_side.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		// ATTACK 애니메이션들
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_down.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_up.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_side.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_side.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		// HIT 애니메이션
		m_animator->RegisterAnimation(MONSTER_HIT, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_hit.png"),
			80, 80, 3, 3, 0.1f, m_pivotX, m_pivotY, false);
		
		// DEATH 애니메이션
		m_animator->RegisterAnimation(MONSTER_DEATH, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_death.png"),
			80, 80, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
	}
	else if (m_id == GOID_MONSTER_WARRIOR_SPIDER)
	{
		// WARRIOR SPIDER 애니메이션들
		// IDLE 애니메이션들
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		// WALK 애니메이션들
		m_animator->RegisterAnimation(MONSTER_WALK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_down.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_WALK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_up.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_WALK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_side.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_WALK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_side.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		// ATTACK 애니메이션들
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_down.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_up.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_side.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_side.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		// HIT 애니메이션
		m_animator->RegisterAnimation(MONSTER_HIT, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_hit.png"),
			80, 80, 3, 3, 0.1f, m_pivotX, m_pivotY, false);
		
		// DEATH 애니메이션
		m_animator->RegisterAnimation(MONSTER_DEATH, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_death.png"),
			80, 80, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
	}
	
	OutputDebugStringW(L"Spider: Unity Animator 스타일로 모든 애니메이션 등록 완료\n");
}

// Unity Animator 스타일 상태 업데이트
void Spider::UpdateAnimatorState()
{
	if (m_animator) {
		m_animator->SetState(m_state, m_direction);
	}
}

Gdiplus::Bitmap* Spider::GetBitmap() const
{
    if (!m_animator) return nullptr;
    
    const SpriteSheet* spriteSheet = m_animator->GetSpriteSheet();
    if (!spriteSheet) return nullptr;
    
    return spriteSheet->GetBitmap();
}

void Spider::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}

void Spider::OnPlayerInteraction(Player* player)
{
	player->OnInteraction(this);
}

void Spider::Damaged(int damage)
{
	m_hp -= damage;
	m_state = MONSTER_HIT;
	UpdateAnimatorState(); // Unity Animator가 자동으로 애니메이션 선택
	
	if (m_hp <= 0) {
		m_state = MONSTER_DEATH;
		UpdateAnimatorState();
		
		// 몬스터가 죽었을 때 처리
		// 아이템 드롭이나 경험치 획득 등의 로직 추가
		OutputDebugStringW(L"Spider: 몬스터가 죽었습니다!\n");
	}
} 