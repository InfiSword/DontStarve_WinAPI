#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../../03_Animation/AnimationDefinition.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "Boss_SpiderQueen.h"

Boss_SpiderQueen::Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, resourcePath, imageName), m_bossPhase(1), m_specialAttackCooldown(0.0f)
{
	// 보스 특성 초기화
	m_hp = 200; // 일반 스파이더보다 높은 체력
	maxHp = m_hp;
}

Boss_SpiderQueen::~Boss_SpiderQueen() {}

void Boss_SpiderQueen::Init()
{
	Monster::Init(); // 부모 클래스 초기화
	
	// 보스 특성 초기화
	m_bossPhase = 1;
	m_specialAttackCooldown = 0.0f;
	
	OutputDebugStringW((L"Boss_SpiderQueen: 보스 초기화 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
}

void Boss_SpiderQueen::OnInteraction(GameObject* obj)
{
    // 보스 상호작용
}

std::vector<AnimationDefinition> Boss_SpiderQueen::GetAnimationDefinitions() const {
    std::vector<AnimationDefinition> definitions;
    ResourceManager* pRM = ResourceManager::GetInstance();
    
    if (m_id == GOID_MONSTER_QUEEN_SPIDER) {
        // IDLE 애니메이션들
        for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
            AnimationDefinition idle;
            idle.state = static_cast<int>(MONSTER_IDLE);
            idle.direction = static_cast<Direction>(dir);
            idle.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_Image.png");
            idle.frameWidth = 120;
            idle.frameHeight = 120;
            idle.framesPerRow = 1;
            idle.totalFrames = 1;
            idle.frameDuration = 0.1f;
            idle.pivotX = m_pivotX;
            idle.pivotY = m_pivotY;
            idle.isLoop = true;
            definitions.push_back(idle);
        }
        
        // WALK 애니메이션들
        std::wstring walkPath = pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Walk_spider_queen_walk_loop_side.png");
        for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
            AnimationDefinition walk;
            walk.state = static_cast<int>(MONSTER_WALK);
            walk.direction = static_cast<Direction>(dir);
            walk.imagePath = walkPath;
            walk.frameWidth = 120;
            walk.frameHeight = 120;
            walk.framesPerRow = 6;
            walk.totalFrames = 6;
            walk.frameDuration = 0.1f;
            walk.pivotX = m_pivotX;
            walk.pivotY = m_pivotY;
            walk.isLoop = true;
            definitions.push_back(walk);
        }
        
        // ATTACK 애니메이션들
        std::wstring attackPath = pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_atk_side.png");
        for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
            AnimationDefinition attack;
            attack.state = static_cast<int>(MONSTER_ATTACK);
            attack.direction = static_cast<Direction>(dir);
            attack.imagePath = attackPath;
            attack.frameWidth = 140;
            attack.frameHeight = 140;
            attack.framesPerRow = 8;
            attack.totalFrames = 8;
            attack.frameDuration = 0.1f;
            attack.pivotX = m_pivotX;
            attack.pivotY = m_pivotY;
            attack.isLoop = false;
            definitions.push_back(attack);
        }
        
        // HIT 애니메이션
        AnimationDefinition hit;
        hit.state = static_cast<int>(MONSTER_HIT);
        hit.direction = DIR_DOWN;
        hit.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_hit_side.png");
        hit.frameWidth = 120;
        hit.frameHeight = 120;
        hit.framesPerRow = 3;
        hit.totalFrames = 3;
        hit.frameDuration = 0.1f;
        hit.pivotX = m_pivotX;
        hit.pivotY = m_pivotY;
        hit.isLoop = false;
        definitions.push_back(hit);
        
        // DEATH 애니메이션
        AnimationDefinition death;
        death.state = static_cast<int>(MONSTER_DEATH);
        death.direction = DIR_DOWN;
        death.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_death.png");
        death.frameWidth = 120;
        death.frameHeight = 120;
        death.framesPerRow = 8;
        death.totalFrames = 8;
        death.frameDuration = 0.1f;
        death.pivotX = m_pivotX;
        death.pivotY = m_pivotY;
        death.isLoop = false;
        definitions.push_back(death);
    }
    
    return definitions;
}

void Boss_SpiderQueen::Damaged(int damage)
{
	m_hp -= damage;
	m_state = MONSTER_HIT;
	
	// 보스 페이즈 체크
	if (m_hp <= maxHp * 0.5f && m_bossPhase == 1) {
		m_bossPhase = 2;
		OutputDebugStringW(L"Boss_SpiderQueen: 보스 페이즈가 2 단계로 전환!\n");
	}
	
	if (m_hp <= 0) {
		m_state = MONSTER_DEATH;
		
		// 보스 처치 시 특별한 보상
		OutputDebugStringW(L"Boss_SpiderQueen: 보스가 처치되었습니다!\n");
	}
}
