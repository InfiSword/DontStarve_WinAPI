#include "../../99_Default/pch.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/AnimationClip.h"
#include "../../02_GameObject/Player/Player.h"
#include "../../03_Animation/SpriteSheet.h"
#include "../../../Header/Struct.h"
#include "Monster.h"

Monster::Monster(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity<MonsterState>(GOBJ_MONSTER, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName), m_hp(100), m_hitAnimTimer(0.0f)
{
    m_animator = nullptr;
	maxHp = m_hp;
}

Monster::~Monster() {}

void Monster::Init()
{
	SetActive(true);
	m_direction = DIR_DOWN;
	m_state = MONSTER_IDLE;
	m_animator = new Animator();
	RegisterAllAnimations(); // Unity Animator 스타일
	UpdateAnimatorState(); // 초기 상태 설정
}

void Monster::LateInit()
{
}

void Monster::Update(float deltaTime)
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

void Monster::LateUpdate()
{
}

void Monster::Render(Gdiplus::Graphics* pGraphics)
{
    // RenderManager::RenderGameObject()에서 UpdateAnimation()과 GetBitmap()을 호출하여 렌더링
    // 개별 GameObject의 Render() 함수는 더 이상 필요하지 않음
}

void Monster::Release()
{
	SafeDelete(m_animator);
}

// Unity Animator 스타일 애니메이션 등록
void Monster::RegisterAllAnimations()
{
    // ResourceManager를 사용하여 경로 구성
    auto* pRM = ResourceManager::GetInstance();
    
    // 몬스터 종류에 따른 분기
    if (m_id == GOID_MONSTER_PIG)
    {
        // PIG 애니메이션들
        m_animator->RegisterAnimation(MONSTER_IDLE, DIR_DOWN,
			 pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Action", L"pig_pigman_idle_loop_down.png"), 
			 120, 150, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
        
        m_animator->RegisterAnimation(MONSTER_IDLE, DIR_UP,
            pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Action", L"pig_pigman_idle_loop_up.png"),
            120, 150, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
        
        m_animator->RegisterAnimation(MONSTER_IDLE, DIR_LEFT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Action", L"pig_pigman_idle_loop_side.png"),
            120, 150, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
        
        m_animator->RegisterAnimation(MONSTER_IDLE, DIR_RIGHT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Action", L"pig_pigman_idle_loop_side.png"),
            120, 150, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
        
        // ATTACK 애니메이션
        m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Attack", L"down_pigman_atk_down.png"),
            150, 180, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
        
        m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_UP,
            pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Attack", L"up_pigman_atk_up.png"),
            150, 180, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
        
        m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_LEFT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Attack", L"side_pigman_atk_side.png"),
            150, 180, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
        
        m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_RIGHT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Attack", L"side_pigman_atk_side.png"),
            150, 180, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
        
        // HIT 애니메이션
        m_animator->RegisterAnimation(MONSTER_HIT, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Hit", L"Hit_pigman_hit.png"),
            120, 150, 3, 3, 0.1f, m_pivotX, m_pivotY, false);
        
        // DEATH 애니메이션
        m_animator->RegisterAnimation(MONSTER_DEATH, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Death", L"Death_pigman_death.png"),
            150, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
    }
    else if (m_id == GOID_MONSTER_SPIDER)
    {
        // SPIDER 애니메이션들 - 실제 파일명에 맞게 수정
        // IDLE 애니메이션 (실제로는 idle_01.png 파일 사용)
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
        
        // WALK 애니메이션 (실제 walk 파일들 사용)
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
        
        // ATTACK 애니메이션
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
    else if (m_id == GOID_MONSTER_HOUNDDOG)
    {
        // HOUND 애니메이션들
        // IDLE 애니메이션
        m_animator->RegisterAnimation(MONSTER_IDLE, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_down.png"),
            120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
        
        m_animator->RegisterAnimation(MONSTER_IDLE, DIR_UP,
            pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_up.png"),
            120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
        
        m_animator->RegisterAnimation(MONSTER_IDLE, DIR_LEFT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_side.png"),
            120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
        
        m_animator->RegisterAnimation(MONSTER_IDLE, DIR_RIGHT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_side.png"),
            120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
        
        // ATTACK 애니메이션
        m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_down.png"),
            140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
        
        m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_UP,
            pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_up.png"),
            140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
        
        m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_LEFT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_side.png"),
            140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
        
        m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_RIGHT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_side.png"),
            140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
    }
    else if (m_id == GOID_MONSTER_WARRIOR_SPIDER)
    {
        // WARRIOR SPIDER 애니메이션들
        // IDLE 애니메이션
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
        
        // WALK 애니메이션
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
        
        // ATTACK 애니메이션
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
    else if (m_id == GOID_MONSTER_QUEEN_SPIDER)
    {
        // QUEEN SPIDER 애니메이션들
        // IDLE 애니메이션
        m_animator->RegisterAnimation(MONSTER_IDLE, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_Image.png"),
            120, 120, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
        
        m_animator->RegisterAnimation(MONSTER_IDLE, DIR_UP,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_Image.png"),
            120, 120, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
        
        m_animator->RegisterAnimation(MONSTER_IDLE, DIR_LEFT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_Image.png"),
            120, 120, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
        
        m_animator->RegisterAnimation(MONSTER_IDLE, DIR_RIGHT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_Image.png"),
            120, 120, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
        
        // WALK 애니메이션
        m_animator->RegisterAnimation(MONSTER_WALK, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Walk_spider_queen_walk_loop_side.png"),
            120, 120, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
        
        m_animator->RegisterAnimation(MONSTER_WALK, DIR_UP,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Walk_spider_queen_walk_loop_side.png"),
            120, 120, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
        
        m_animator->RegisterAnimation(MONSTER_WALK, DIR_LEFT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Walk_spider_queen_walk_loop_side.png"),
            120, 120, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
        
        m_animator->RegisterAnimation(MONSTER_WALK, DIR_RIGHT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Walk_spider_queen_walk_loop_side.png"),
            120, 120, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
        
        // ATTACK 애니메이션
        m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_atk_side.png"),
            140, 140, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
        
        m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_UP,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_atk_side.png"),
            140, 140, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
        
        m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_LEFT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_atk_side.png"),
            140, 140, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
        
        m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_RIGHT,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_atk_side.png"),
            140, 140, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
        
        // HIT 애니메이션
        m_animator->RegisterAnimation(MONSTER_HIT, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_hit_side.png"),
            120, 120, 3, 3, 0.1f, m_pivotX, m_pivotY, false);
        
        // DEATH 애니메이션
        m_animator->RegisterAnimation(MONSTER_DEATH, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_death.png"),
            120, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
    }
    
    OutputDebugStringW(L"Monster: Unity Animator 스타일로 모든 애니메이션 등록 완료\n");
}

// Unity Animator 스타일 상태 업데이트
void Monster::UpdateAnimatorState()
{
	if (m_animator) {
		m_animator->SetState(m_state, m_direction);
	}
}

Gdiplus::Bitmap* Monster::GetBitmap() const
{
    if (!m_animator) return nullptr;
    
    const SpriteSheet* spriteSheet = m_animator->GetSpriteSheet();
    if (!spriteSheet) return nullptr;
    
    return spriteSheet->GetBitmap();
}

void Monster::OnInteraction(GameObject* obj)
{
	// 기본 구현
}

void Monster::OnPlayerInteraction(Player* player)
{
	player->OnInteraction(this);
}

void Monster::Damaged(int damage)
{
	m_hp -= damage;
	m_state = MONSTER_HIT;
	UpdateAnimatorState(); // Unity Animator가 자동으로 애니메이션 선택
	
	if (m_hp <= 0) {
		m_state = MONSTER_DEATH;
		UpdateAnimatorState();
		
		// 아이템 드롭 로직
		// 몬스터가 죽으면 고기 아이템 생성
		OutputDebugStringW(L"Monster: 몬스터가 죽었습니다!\n");
	}
}
