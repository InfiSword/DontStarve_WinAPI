#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../../03_Animation/AnimationDefinition.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "Spider.h"

Spider::Spider(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, resourcePath, imageName)
{
	m_hp = 80;
	maxHp = m_hp;
}

Spider::~Spider() {}

void Spider::Init()
{
	Monster::Init();
	
	OutputDebugStringW((L"Spider: Init 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	
	// 초기 크기 설정 (애니메이션 클립에서 첫 번째 프레임으로 크기 설정)
	Animator* animator = GetComponent<Animator>();
	if (animator) {
		const AnimationFrame& frame = animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
		
		// Animator 초기화 확인
		const SpriteSheet* spriteSheet = animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Spider: Animator 초기화 완료 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
		} else {
			OutputDebugStringW((L"Spider: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Spider: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

void Spider::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}

void Spider::Damaged(int damage)
{
}

std::vector<AnimationDefinition> Spider::GetAnimationDefinitions() const {
    std::vector<AnimationDefinition> definitions;
    ResourceManager* pRM = ResourceManager::GetInstance();
    
    if (m_id == GOID_MONSTER_SPIDER) {
        // IDLE 애니메이션들
        for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
            AnimationDefinition idle;
            idle.state = static_cast<int>(MONSTER_IDLE);
            idle.direction = static_cast<Direction>(dir);
            idle.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_idle_01.png");
            idle.frameWidth = 80;
            idle.frameHeight = 80;
            idle.framesPerRow = 1;
            idle.totalFrames = 1;
            idle.frameDuration = 0.1f;
            idle.pivotX = m_pivotX;
            idle.pivotY = m_pivotY;
            idle.isLoop = true;
            definitions.push_back(idle);
        }
        
        // WALK 애니메이션들
        AnimationDefinition walkDown;
        walkDown.state = static_cast<int>(MONSTER_WALK);
        walkDown.direction = DIR_DOWN;
        walkDown.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_down.png");
        walkDown.frameWidth = 80;
        walkDown.frameHeight = 80;
        walkDown.framesPerRow = 6;
        walkDown.totalFrames = 6;
        walkDown.frameDuration = 0.1f;
        walkDown.pivotX = m_pivotX;
        walkDown.pivotY = m_pivotY;
        walkDown.isLoop = true;
        definitions.push_back(walkDown);
        
        AnimationDefinition walkUp;
        walkUp.state = static_cast<int>(MONSTER_WALK);
        walkUp.direction = DIR_UP;
        walkUp.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_up.png");
        walkUp.frameWidth = 80;
        walkUp.frameHeight = 80;
        walkUp.framesPerRow = 6;
        walkUp.totalFrames = 6;
        walkUp.frameDuration = 0.1f;
        walkUp.pivotX = m_pivotX;
        walkUp.pivotY = m_pivotY;
        walkUp.isLoop = true;
        definitions.push_back(walkUp);
        
        AnimationDefinition walkSide;
        walkSide.state = static_cast<int>(MONSTER_WALK);
        walkSide.direction = DIR_LEFT;
        walkSide.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_side.png");
        walkSide.frameWidth = 80;
        walkSide.frameHeight = 80;
        walkSide.framesPerRow = 6;
        walkSide.totalFrames = 6;
        walkSide.frameDuration = 0.1f;
        walkSide.pivotX = m_pivotX;
        walkSide.pivotY = m_pivotY;
        walkSide.isLoop = true;
        definitions.push_back(walkSide);
        
        walkSide.direction = DIR_RIGHT;
        definitions.push_back(walkSide);
        
        // ATTACK 애니메이션들
        AnimationDefinition attackDown;
        attackDown.state = static_cast<int>(MONSTER_ATTACK);
        attackDown.direction = DIR_DOWN;
        attackDown.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_down.png");
        attackDown.frameWidth = 100;
        attackDown.frameHeight = 100;
        attackDown.framesPerRow = 8;
        attackDown.totalFrames = 8;
        attackDown.frameDuration = 0.1f;
        attackDown.pivotX = m_pivotX;
        attackDown.pivotY = m_pivotY;
        attackDown.isLoop = false;
        definitions.push_back(attackDown);
        
        AnimationDefinition attackUp;
        attackUp.state = static_cast<int>(MONSTER_ATTACK);
        attackUp.direction = DIR_UP;
        attackUp.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_up.png");
        attackUp.frameWidth = 100;
        attackUp.frameHeight = 100;
        attackUp.framesPerRow = 8;
        attackUp.totalFrames = 8;
        attackUp.frameDuration = 0.1f;
        attackUp.pivotX = m_pivotX;
        attackUp.pivotY = m_pivotY;
        attackUp.isLoop = false;
        definitions.push_back(attackUp);
        
        AnimationDefinition attackSide;
        attackSide.state = static_cast<int>(MONSTER_ATTACK);
        attackSide.direction = DIR_LEFT;
        attackSide.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_side.png");
        attackSide.frameWidth = 100;
        attackSide.frameHeight = 100;
        attackSide.framesPerRow = 8;
        attackSide.totalFrames = 8;
        attackSide.frameDuration = 0.1f;
        attackSide.pivotX = m_pivotX;
        attackSide.pivotY = m_pivotY;
        attackSide.isLoop = false;
        definitions.push_back(attackSide);
        
        attackSide.direction = DIR_RIGHT;
        definitions.push_back(attackSide);
        
        // HIT 애니메이션
        AnimationDefinition hit;
        hit.state = static_cast<int>(MONSTER_HIT);
        hit.direction = DIR_DOWN;
        hit.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_hit.png");
        hit.frameWidth = 80;
        hit.frameHeight = 80;
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
        death.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_death.png");
        death.frameWidth = 80;
        death.frameHeight = 80;
        death.framesPerRow = 8;
        death.totalFrames = 8;
        death.frameDuration = 0.1f;
        death.pivotX = m_pivotX;
        death.pivotY = m_pivotY;
        death.isLoop = false;
        definitions.push_back(death);
    }
    else if (m_id == GOID_MONSTER_WARRIOR_SPIDER) {
        // WARRIOR SPIDER 애니메이션들
        // IDLE 애니메이션들
        for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
            AnimationDefinition idle;
            idle.state = static_cast<int>(MONSTER_IDLE);
            idle.direction = static_cast<Direction>(dir);
            idle.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_idle_01.png");
            idle.frameWidth = 80;
            idle.frameHeight = 80;
            idle.framesPerRow = 1;
            idle.totalFrames = 1;
            idle.frameDuration = 0.1f;
            idle.pivotX = m_pivotX;
            idle.pivotY = m_pivotY;
            idle.isLoop = true;
            definitions.push_back(idle);
        }
        
        // WALK 애니메이션들
        AnimationDefinition walkDown;
        walkDown.state = static_cast<int>(MONSTER_WALK);
        walkDown.direction = DIR_DOWN;
        walkDown.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_down.png");
        walkDown.frameWidth = 80;
        walkDown.frameHeight = 80;
        walkDown.framesPerRow = 6;
        walkDown.totalFrames = 6;
        walkDown.frameDuration = 0.1f;
        walkDown.pivotX = m_pivotX;
        walkDown.pivotY = m_pivotY;
        walkDown.isLoop = true;
        definitions.push_back(walkDown);
        
        AnimationDefinition walkUp;
        walkUp.state = static_cast<int>(MONSTER_WALK);
        walkUp.direction = DIR_UP;
        walkUp.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_up.png");
        walkUp.frameWidth = 80;
        walkUp.frameHeight = 80;
        walkUp.framesPerRow = 6;
        walkUp.totalFrames = 6;
        walkUp.frameDuration = 0.1f;
        walkUp.pivotX = m_pivotX;
        walkUp.pivotY = m_pivotY;
        walkUp.isLoop = true;
        definitions.push_back(walkUp);
        
        AnimationDefinition walkSide;
        walkSide.state = static_cast<int>(MONSTER_WALK);
        walkSide.direction = DIR_LEFT;
        walkSide.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_side.png");
        walkSide.frameWidth = 80;
        walkSide.frameHeight = 80;
        walkSide.framesPerRow = 6;
        walkSide.totalFrames = 6;
        walkSide.frameDuration = 0.1f;
        walkSide.pivotX = m_pivotX;
        walkSide.pivotY = m_pivotY;
        walkSide.isLoop = true;
        definitions.push_back(walkSide);
        
        walkSide.direction = DIR_RIGHT;
        definitions.push_back(walkSide);
        
        // ATTACK 애니메이션들
        AnimationDefinition attackDown;
        attackDown.state = static_cast<int>(MONSTER_ATTACK);
        attackDown.direction = DIR_DOWN;
        attackDown.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_down.png");
        attackDown.frameWidth = 100;
        attackDown.frameHeight = 100;
        attackDown.framesPerRow = 8;
        attackDown.totalFrames = 8;
        attackDown.frameDuration = 0.1f;
        attackDown.pivotX = m_pivotX;
        attackDown.pivotY = m_pivotY;
        attackDown.isLoop = false;
        definitions.push_back(attackDown);
        
        AnimationDefinition attackUp;
        attackUp.state = static_cast<int>(MONSTER_ATTACK);
        attackUp.direction = DIR_UP;
        attackUp.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_up.png");
        attackUp.frameWidth = 100;
        attackUp.frameHeight = 100;
        attackUp.framesPerRow = 8;
        attackUp.totalFrames = 8;
        attackUp.frameDuration = 0.1f;
        attackUp.pivotX = m_pivotX;
        attackUp.pivotY = m_pivotY;
        attackUp.isLoop = false;
        definitions.push_back(attackUp);
        
        AnimationDefinition attackSide;
        attackSide.state = static_cast<int>(MONSTER_ATTACK);
        attackSide.direction = DIR_LEFT;
        attackSide.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_side.png");
        attackSide.frameWidth = 100;
        attackSide.frameHeight = 100;
        attackSide.framesPerRow = 8;
        attackSide.totalFrames = 8;
        attackSide.frameDuration = 0.1f;
        attackSide.pivotX = m_pivotX;
        attackSide.pivotY = m_pivotY;
        attackSide.isLoop = false;
        definitions.push_back(attackSide);
        
        attackSide.direction = DIR_RIGHT;
        definitions.push_back(attackSide);
        
        // HIT 애니메이션
        AnimationDefinition hit;
        hit.state = static_cast<int>(MONSTER_HIT);
        hit.direction = DIR_DOWN;
        hit.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_hit.png");
        hit.frameWidth = 80;
        hit.frameHeight = 80;
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
        death.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_death.png");
        death.frameWidth = 80;
        death.frameHeight = 80;
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
