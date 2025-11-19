#include "../../99_Default/pch.h"
#include "SpiderEgg.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/AnimationClip.h"

SpiderEgg::SpiderEgg(GameObjectID id, float x, float y, float pivotX, float pivotY, 
    Direction _dir, const std::wstring& resourcePath,
    const std::wstring& imageName, int hp)
    : Building(id, x, y, pivotX, pivotY, _dir, resourcePath, imageName, hp)
{
    OutputDebugStringW(L"SpiderEgg: 생성자 호출\n");
}

SpiderEgg::~SpiderEgg()
{
    OutputDebugStringW(L"SpiderEgg: 소멸자 호출\n");
}

void SpiderEgg::Init()
{
    OutputDebugStringW(L"SpiderEgg: Init 시작\n");
    
    SetActive(true);
    SetInteractive(true);
    m_state = BUILDING_NOON;
    m_buildingState = BUILDING_NOON;
    m_direction = DIR_DOWN;
    
    pAnimator = AddComponent<Animator>();
    RegisterAllAnimations();
    UpdateAnimatorState();
    
    // 초기 크기 설정
    if (pAnimator) {
        const AnimationFrame& frame = pAnimator->GetCurrentFrame();
        this->m_width = frame.width;
        this->m_height = frame.height;
    }
    
    OutputDebugStringW(L"SpiderEgg: Init 완료\n");
}

void SpiderEgg::LateInit()
{
    OutputDebugStringW(L"SpiderEgg: LateInit 호출\n");
}

void SpiderEgg::Update(float deltaTime)
{
    if (pAnimator) {
        const AnimationFrame& frame = pAnimator->GetCurrentFrame();
        this->m_width = frame.width;
        this->m_height = frame.height;
    }
    // 애니메이션 업데이트는 GameObject::Update()에서 컴포넌트의 Update()를 통해 자동으로 처리됨
}

void SpiderEgg::LateUpdate()
{
}

void SpiderEgg::Release()
{
    OutputDebugStringW(L"SpiderEgg: Release 호출\n");
    // pAnimator는 GameObject::Release()에서 컴포넌트로 해제됨
    for (auto& pair : m_animClips)
    {
        SafeDelete(pair.second);
    }
    m_animClips.clear();
}

void SpiderEgg::Damaged(int damage)
{
    OutputDebugStringW((L"SpiderEgg: Damaged - 데미지: " + std::to_wstring(damage) + L"\n").c_str());
    
    m_hp -= damage;
    if (m_hp <= 0)
    {
        m_hp = 0;
        m_state = BUILDING_DESTROYED;
        m_buildingState = BUILDING_DESTROYED;
        UpdateAnimatorState();
        OutputDebugStringW(L"SpiderEgg: 파괴됨\n");
        // TODO: 파괴 효과 처리
    }
    else if (m_hp <= m_maxHp / 2)
    {
        m_state = BUILDING_DAMAGED;
        m_buildingState = BUILDING_DAMAGED;
        UpdateAnimatorState();
        OutputDebugStringW(L"SpiderEgg: 손상됨\n");
    }
}

void SpiderEgg::SetTimeState(BuildingState buildingState)
{
    m_state = buildingState;
    m_buildingState = buildingState;
    UpdateAnimatorState();
}

BuildingState SpiderEgg::GetTimeState() const
{
    return m_buildingState;
}

std::wstring SpiderEgg::GetAnimKey(BuildingState state)
{
    std::wstring key;
    if (state == BUILDING_NOON) {
        key = L"SpiderEgg_Noon";
    }
    else if (state == BUILDING_NIGHT) {
        key = L"SpiderEgg_Night";
    }
    else if (state == BUILDING_DAMAGED) {
        key = L"SpiderEgg_Damaged";
    }
    else {
        key = L"SpiderEgg_Destroyed";
    }
    return key;
}

// Unity Animator 스타일 애니메이션 등록
void SpiderEgg::RegisterAllAnimations()
{
    OutputDebugStringW((L"SpiderEgg: RegisterAllAnimations 시작 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

    // ResourceManager를 사용하여 리소스 로드
    auto* pRM = ResourceManager::GetInstance();
    
    // 거미 알 타입별로 등록 확인
    if (m_id == GOID_BUILDING_SPIDER_SMALLEGG)
    {
        OutputDebugStringW(L"SpiderEgg: SPIDER SMALL EGG 애니메이션 등록\n");
        pAnimator->RegisterAnimation(BUILDING_NOON, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_BUILDING_SPIDER_SMALLEGG, L"", L"Egg_spider_cocoon_small_Image.png"),
            80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
    }
    else if (m_id == GOID_BUILDING_SPIDER_NORMALEGG)
    {
        OutputDebugStringW(L"SpiderEgg: SPIDER NORMAL EGG 애니메이션 등록\n");
        pAnimator->RegisterAnimation(BUILDING_NOON, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_BUILDING_SPIDER_NORMALEGG, L"", L"Egg_spider_cocoon_medium_Image.png"),
            80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
    }
    else if (m_id == GOID_BUILDING_SPIDER_TALLEGG)
    {
        OutputDebugStringW(L"SpiderEgg: SPIDER TALL EGG 애니메이션 등록\n");
        pAnimator->RegisterAnimation(BUILDING_NOON, DIR_DOWN,
            pRM->BuildObjectResourcePath(GOID_BUILDING_SPIDER_TALLEGG, L"", L"Egg_spider_cocoon_large_Image.png"),
            80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
    }
    else
    {
        OutputDebugStringW((L"SpiderEgg: 알 수 없는 거미 알 ID: " + std::to_wstring(m_id) + L"\n").c_str());
    }
    
    OutputDebugStringW(L"SpiderEgg: Unity Animator 스타일로 모든 애니메이션 등록 완료\n");
}

// Unity Animator 스타일 상태 업데이트
void SpiderEgg::UpdateAnimatorState()
{
    if (pAnimator) {
        pAnimator->SetState(m_state, m_direction);
    }
}


Gdiplus::Bitmap* SpiderEgg::GetBitmap() const
{
    if (!pAnimator) return nullptr;
    
    const SpriteSheet* spriteSheet = pAnimator->GetSpriteSheet();
    if (!spriteSheet) return nullptr;
    
    return spriteSheet->GetBitmap();
}
