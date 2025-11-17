#include "../../99_Default/pch.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/AnimationClip.h"
#include "../../02_GameObject/Player/Player.h"
#include "../../03_Animation/SpriteSheet.h"
#include "Sapling.h"

Sapling::Sapling(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity<GrassState>(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName)
{
	m_animator = nullptr;
	m_dropItemID = GOID_ITEM_NORMAL_TWIGS;  // 기본적으로 twigs 아이템 제공
	m_dropItemCount = 1;
}

Sapling::~Sapling() {}

void Sapling::Init()
{
	SetActive(true);
	SetInteractive(true); // Sapling의 상호작용 활성화
	m_direction = DIR_DOWN;
	m_state = GRASS_IDLE;
	m_animator = new Animator();
	
	// 기본 아이템 정보 설정
	m_dropItemID = GOID_ITEM_NORMAL_TWIGS;
	m_dropItemCount = 1;
	
	OutputDebugStringW((L"Sapling: Init 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	
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
			OutputDebugStringW((L"Sapling: Animator 초기화 완료 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
		} else {
			OutputDebugStringW((L"Sapling: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Sapling: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

void Sapling::LateInit()
{
}

void Sapling::Update(float deltaTime)
{
	if (m_animator) {
		const AnimationFrame& frame = m_animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
	}
	UpdateAnimation(deltaTime);
}

void Sapling::LateUpdate()
{
}

void Sapling::Render(Gdiplus::Graphics* pGraphics)
{
    // RenderManager::RenderGameObject()에서 UpdateAnimation()과 GetBitmap()을 호출하여 렌더링
    // 따라서 GameObject의 Render() 함수는 더 이상 필요하지 않음
}

void Sapling::Release()
{
	SafeDelete(m_animator);
}

// Unity Animator 스타일 애니메이션 등록
void Sapling::RegisterAllAnimations()
{
	// ResourceManager를 사용하여 리소스 로드
	auto* pRM = ResourceManager::GetInstance();
	
	// 새싹 전용 애니메이션 등록
	if (m_id == GOID_NORMAL_SAPLING)
	{
		// IDLE 애니메이션 (새싹 기본)
		m_animator->RegisterAnimation(GRASS_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_SAPLING, L"", L"sapling.png"),
			143, 200, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		// PICK 애니메이션 (새싹 수확 시)
		//m_animator->RegisterAnimation(GRASS_PICK, DIR_DOWN,
		//	pRM->BuildObjectResourcePath(GOID_NORMAL_SAPLING, L"", L"sapling.png"),
		//	48, 64, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
	}
	else
	{
		// 기본값 (새싹)
		m_animator->RegisterAnimation(GRASS_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_NORMAL_SAPLING, L"", L"sapling.png"),
			48, 64, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		//m_animator->RegisterAnimation(GRASS_PICK, DIR_DOWN,
		//	pRM->BuildObjectResourcePath(GOID_NORMAL_SAPLING, L"", L"sapling.png"),
		//	48, 64, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
	}
	
	OutputDebugStringW(L"Sapling: Unity Animator 스타일로 모든 애니메이션 등록 완료\n");
}

// Unity Animator 스타일 상태 업데이트
void Sapling::UpdateAnimatorState()
{
	if (m_animator) {
		m_animator->SetState(m_state, m_direction);
	}
}

void Sapling::UpdateAnimation(float deltaTime) 
{
	if (m_animator)
		m_animator->Update(deltaTime);
}

Gdiplus::Bitmap* Sapling::GetBitmap() const
{
    if (!m_animator) return nullptr;
    
    const SpriteSheet* spriteSheet = m_animator->GetSpriteSheet();
    if (!spriteSheet) return nullptr;
    
    return spriteSheet->GetBitmap();
}

void Sapling::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}

void Sapling::OnPlayerInteraction(Player* player)
{
	if (GetActive() && CanInteract()) {
		// 플레이어가 이 Sapling과 상호작용하도록 요청
		player->OnInteraction(this);
	}
}

void Sapling::SetDropItem(GameObjectID itemID, int count)
{
	m_dropItemID = itemID;
	m_dropItemCount = count;
}
