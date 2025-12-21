#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../../03_Animation/AnimationDefinition.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "Pig.h"

Pig::Pig(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, resourcePath, imageName)
{
	m_hp = 100;
	maxHp = m_hp;
}

Pig::~Pig() {}

void Pig::Init()
{
	Monster::Init();
	
	OutputDebugStringW((L"Pig: Init 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	
	// 초기 크기 설정 (애니메이션 클립에서 첫 번째 프레임으로 크기 설정)
	Animator* animator = GetComponent<Animator>();
	if (animator) {
		const AnimationFrame& frame = animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
		
		// Animator 초기화 확인
		const SpriteSheet* spriteSheet = animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Pig: Animator 초기화 완료 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
		} else {
			OutputDebugStringW((L"Pig: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Pig: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

void Pig::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}

void Pig::Damaged(int damage)
{
}

std::vector<AnimationDefinition> Pig::GetAnimationDefinitions() const {
    std::vector<AnimationDefinition> definitions;
    ResourceManager* pRM = ResourceManager::GetInstance();
    
    if (m_id == GOID_MONSTER_PIG) {
        // IDLE 애니메이션들
        AnimationDefinition idleDown;
        idleDown.state = static_cast<int>(MONSTER_IDLE);
        idleDown.direction = DIR_DOWN;
        idleDown.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Action", L"pig_pigman_idle_loop_down.png");
        idleDown.frameWidth = 120;
        idleDown.frameHeight = 150;
        idleDown.framesPerRow = 6;
        idleDown.totalFrames = 6;
        idleDown.frameDuration = 0.1f;
        idleDown.pivotX = m_pivotX;
        idleDown.pivotY = m_pivotY;
        idleDown.isLoop = true;
        definitions.push_back(idleDown);
        
        AnimationDefinition idleUp;
        idleUp.state = static_cast<int>(MONSTER_IDLE);
        idleUp.direction = DIR_UP;
        idleUp.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Action", L"pig_pigman_idle_loop_up.png");
        idleUp.frameWidth = 120;
        idleUp.frameHeight = 150;
        idleUp.framesPerRow = 6;
        idleUp.totalFrames = 6;
        idleUp.frameDuration = 0.1f;
        idleUp.pivotX = m_pivotX;
        idleUp.pivotY = m_pivotY;
        idleUp.isLoop = true;
        definitions.push_back(idleUp);
        
        AnimationDefinition idleSide;
        idleSide.state = static_cast<int>(MONSTER_IDLE);
        idleSide.direction = DIR_LEFT;
        idleSide.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Action", L"pig_pigman_idle_loop_side.png");
        idleSide.frameWidth = 120;
        idleSide.frameHeight = 150;
        idleSide.framesPerRow = 6;
        idleSide.totalFrames = 6;
        idleSide.frameDuration = 0.1f;
        idleSide.pivotX = m_pivotX;
        idleSide.pivotY = m_pivotY;
        idleSide.isLoop = true;
        definitions.push_back(idleSide);
        
        idleSide.direction = DIR_RIGHT;
        definitions.push_back(idleSide);
        
        // ATTACK 애니메이션들
        AnimationDefinition attackDown;
        attackDown.state = static_cast<int>(MONSTER_ATTACK);
        attackDown.direction = DIR_DOWN;
        attackDown.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Attack", L"down_pigman_atk_down.png");
        attackDown.frameWidth = 150;
        attackDown.frameHeight = 180;
        attackDown.framesPerRow = 6;
        attackDown.totalFrames = 6;
        attackDown.frameDuration = 0.1f;
        attackDown.pivotX = m_pivotX;
        attackDown.pivotY = m_pivotY;
        attackDown.isLoop = false;
        definitions.push_back(attackDown);
        
        AnimationDefinition attackUp;
        attackUp.state = static_cast<int>(MONSTER_ATTACK);
        attackUp.direction = DIR_UP;
        attackUp.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Attack", L"up_pigman_atk_up.png");
        attackUp.frameWidth = 150;
        attackUp.frameHeight = 180;
        attackUp.framesPerRow = 6;
        attackUp.totalFrames = 6;
        attackUp.frameDuration = 0.1f;
        attackUp.pivotX = m_pivotX;
        attackUp.pivotY = m_pivotY;
        attackUp.isLoop = false;
        definitions.push_back(attackUp);
        
        AnimationDefinition attackSide;
        attackSide.state = static_cast<int>(MONSTER_ATTACK);
        attackSide.direction = DIR_LEFT;
        attackSide.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Attack", L"side_pigman_atk_side.png");
        attackSide.frameWidth = 150;
        attackSide.frameHeight = 180;
        attackSide.framesPerRow = 6;
        attackSide.totalFrames = 6;
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
        hit.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Hit", L"Hit_pigman_hit.png");
        hit.frameWidth = 120;
        hit.frameHeight = 150;
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
        death.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Death", L"Death_pigman_death.png");
        death.frameWidth = 150;
        death.frameHeight = 100;
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
