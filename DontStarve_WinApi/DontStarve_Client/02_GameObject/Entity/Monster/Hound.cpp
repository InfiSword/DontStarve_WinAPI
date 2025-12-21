#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../../03_Animation/AnimationDefinition.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../Player/Player.h"
#include "Hound.h"

Hound::Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, resourcePath, imageName)
{
	m_hp = 90;
	maxHp = m_hp;
}

Hound::~Hound() {}

void Hound::Init()
{
	Monster::Init();
	
	OutputDebugStringW((L"Hound: Init 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	
	// 초기 크기 설정 (애니메이션 클립에서 첫 번째 프레임으로 크기 설정)
	Animator* animator = GetComponent<Animator>();
	if (animator) {
		const AnimationFrame& frame = animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
		
		// Animator 초기화 확인
		const SpriteSheet* spriteSheet = animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Hound: Animator 초기화 완료 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
		} else {
			OutputDebugStringW((L"Hound: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Hound: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

void Hound::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}

std::vector<AnimationDefinition> Hound::GetAnimationDefinitions() const {
    std::vector<AnimationDefinition> definitions;
    ResourceManager* pRM = ResourceManager::GetInstance();
    
    if (m_id == GOID_MONSTER_HOUNDDOG) {
        // IDLE 애니메이션들
        AnimationDefinition idleDown;
        idleDown.state = static_cast<int>(MONSTER_IDLE);
        idleDown.direction = DIR_DOWN;
        idleDown.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_down.png");
        idleDown.frameWidth = 120;
        idleDown.frameHeight = 100;
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
        idleUp.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_up.png");
        idleUp.frameWidth = 120;
        idleUp.frameHeight = 100;
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
        idleSide.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_side.png");
        idleSide.frameWidth = 120;
        idleSide.frameHeight = 100;
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
        attackDown.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_down.png");
        attackDown.frameWidth = 140;
        attackDown.frameHeight = 120;
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
        attackUp.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_up.png");
        attackUp.frameWidth = 140;
        attackUp.frameHeight = 120;
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
        attackSide.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_side.png");
        attackSide.frameWidth = 140;
        attackSide.frameHeight = 120;
        attackSide.framesPerRow = 8;
        attackSide.totalFrames = 8;
        attackSide.frameDuration = 0.1f;
        attackSide.pivotX = m_pivotX;
        attackSide.pivotY = m_pivotY;
        attackSide.isLoop = false;
        definitions.push_back(attackSide);
        
        attackSide.direction = DIR_RIGHT;
        definitions.push_back(attackSide);
    }
    
    return definitions;
}
