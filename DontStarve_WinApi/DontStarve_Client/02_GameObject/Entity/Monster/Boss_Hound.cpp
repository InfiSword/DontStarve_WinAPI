#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../../03_Animation/AnimationDefinition.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "Boss_Hound.h"

Boss_Hound::Boss_Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, resourcePath, imageName), m_bossPhase(1), m_specialAttackCooldown(0.0f)
{
	// 보스 특성 초기화
	m_hp = 150; // 일반 하운드보다 높은 체력
	maxHp = m_hp;
	
	// Hound 타입 설정
	if (id == GOID_MONSTER_REDHOUNDDOG) {
		m_houndType = L"Red";
	} else if (id == GOID_MONSTER_ICEHOUNDDOG) {
		m_houndType = L"Ice";
	}
}

Boss_Hound::~Boss_Hound() {}

void Boss_Hound::Init()
{
	Monster::Init(); // 부모 클래스 초기화
	
	// 보스 특성 초기화
	m_bossPhase = 1;
	m_specialAttackCooldown = 0.0f;
	
	OutputDebugStringW((L"Boss_Hound: " + m_houndType + L" 보스 초기화 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
}

void Boss_Hound::OnInteraction(GameObject* obj)
{
    // 보스 상호작용
}

std::vector<AnimationDefinition> Boss_Hound::GetAnimationDefinitions() const {
    std::vector<AnimationDefinition> definitions;
    ResourceManager* pRM = ResourceManager::GetInstance();
    
    if (m_id == GOID_MONSTER_REDHOUNDDOG) {
        // RED HOUND 보스 애니메이션들
        // IDLE 애니메이션들
        AnimationDefinition idleDown;
        idleDown.state = static_cast<int>(MONSTER_IDLE);
        idleDown.direction = DIR_DOWN;
        idleDown.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_idle_down.png");
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
        idleUp.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_idle_up.png");
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
        idleSide.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_idle_side.png");
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
        attackDown.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_atk_down.png");
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
        attackUp.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_atk_up.png");
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
        attackSide.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_atk_side.png");
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
    else if (m_id == GOID_MONSTER_ICEHOUNDDOG) {
        // ICE HOUND 보스 애니메이션들
        // IDLE 애니메이션들
        AnimationDefinition idleDown;
        idleDown.state = static_cast<int>(MONSTER_IDLE);
        idleDown.direction = DIR_DOWN;
        idleDown.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_idle_down.png");
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
        idleUp.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_idle_up.png");
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
        idleSide.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_idle_side.png");
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
        attackDown.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_atk_down.png");
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
        attackUp.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_atk_up.png");
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
        attackSide.imagePath = pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_atk_side.png");
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

void Boss_Hound::Damaged(int damage)
{
	m_hp -= damage;
	m_state = MONSTER_HIT;
	
	// 보스 페이즈 체크
	if (m_hp <= maxHp * 0.5f && m_bossPhase == 1) {
		m_bossPhase = 2;
		OutputDebugStringW((L"Boss_Hound: " + m_houndType + L" 보스 페이즈가 2 단계로 전환!\n").c_str());
	}
	
	if (m_hp <= 0) {
		m_state = MONSTER_DEATH;
		
		// 보스 처치 시 특별한 보상
		OutputDebugStringW((L"Boss_Hound: " + m_houndType + L" 보스가 처치되었습니다!\n").c_str());
	}
}
