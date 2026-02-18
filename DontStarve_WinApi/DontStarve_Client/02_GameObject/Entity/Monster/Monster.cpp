#include "99_Default/pch.h"
#include  "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../../../01_Manager/SceneManager/SceneManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "Monster.h"

Monster::Monster(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Entity(GOBJ_MONSTER, id, x, y, pivotX, pivotY, DIR_DOWN, baseDir, imageName), m_hp(100), m_hitAnimTimer(0.0f), m_state(MonsterState::MONSTER_IDLE)
{
	maxHp = m_hp;
}

Monster::~Monster() {}

void Monster::Init()
{
	Entity::Init();
	//AddComponent<Animator>();
	//RegisterAllAnimations(); // Unity Animator 스타일
	//UpdateAnimatorState(); // 초기 상태 설정
}

void Monster::LateInit()
{
}

void Monster::Update(float deltaTime)
{
	/*if (m_state == MONSTER_HIT) {
		m_hitAnimTimer += deltaTime;
		Animator* animator = GetComponent<Animator>();
		if (animator && m_hitAnimTimer >= animator->GetCurrentClipTotalDuration()) {
			m_state = MONSTER_IDLE; 
			UpdateAnimatorState();
			m_hitAnimTimer = 0.0f; 
		}
	}

	Animator* animator = GetComponent<Animator>();
	if (animator) {
		const AnimationFrame& frame = animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
	}*/
	
	// 부모 클래스의 Update() 호출하여 컴포넌트 업데이트
	Entity::Update(deltaTime);
}

void Monster::LateUpdate()
{
}

void Monster::Release()
{
	// Monster 전용 정리 작업
	// Animator 컴포넌트는 GameObject::Release()에서 m_components를 통해 자동으로 해제됨
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	Entity::Release();
}

// Unity Animator 스타일 애니메이션 등록
// 각 자식 클래스에서 자신의 애니메이션을 등록하도록 순수 가상 함수로 선언됨
// Monster 클래스에서는 구현하지 않음

// Unity Animator 스타일 상태 업데이트
//void Monster::UpdateAnimatorState()
//{
//	Animator* animator = GetComponent<Animator>();
//	if (animator) {
//		animator->SetState(m_state, m_direction);
//	}
//}
//
//Gdiplus::Bitmap* Monster::GetBitmap() const
//{
//    Animator* animator = GetComponent<Animator>();
//    if (!animator) return nullptr;
//    
//    const SpriteSheet* spriteSheet = animator->GetSpriteSheet();
//    if (!spriteSheet) return nullptr;
//    
//    return spriteSheet->GetBitmap();
//}

void Monster::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}


void Monster::Damaged(int damage)
{
	m_hp -= damage;
	m_state = MONSTER_HIT;
	// UpdateAnimatorState(); // Unity Animator에서 자동으로 애니메이션 전환
	
	if (m_hp <= 0) {
		m_state = MONSTER_DEATH;
		// UpdateAnimatorState();
		
		// 몬스터 처치 이벤트 발생
		SceneType currentScene = SceneManager::GetInstance()->GetCurrentSceneType();
		GameProgressManager::GetInstance()->OnMonsterKilled(GetID(), currentScene);
		
		// 몬스터가 죽었을 때 처리
		// 드롭 아이템이나 경험치 획득 로직 추가 필요
		OutputDebugStringW(L"Monster: 몬스터가 죽었습니다!\n");
		
		// 오브젝트 비활성화 (나중에 ObjectManager에서 제거)
		SetActive(false);
	}
}
