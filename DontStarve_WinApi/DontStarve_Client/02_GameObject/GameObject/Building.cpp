#include "../../99_Default/pch.h"
#include "Building.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/AnimationClip.h"

Building::Building(GameObjectID id, float x, float y, float pivotX, float pivotY, 
    Direction _dir, const std::wstring& resourcePath,
                   const std::wstring& imageName, int hp)
    : Entity<BuildingState>(GOBJ_BUILDING, id, x, y, pivotX, pivotY, _dir, resourcePath, imageName),
      m_hp(hp), m_maxHp(hp), m_buildingState(BUILDING_NOON), pAnimator(nullptr)
{
}

Building::~Building()
{
}

void Building::Init()
{
    SetActive(true);
    m_state = BUILDING_NOON;
    m_buildingState = BUILDING_NOON;
    m_direction = DIR_DOWN;
    pAnimator = new Animator();
    RegisterAllAnimations();
    UpdateAnimatorState();
}

void Building::LateInit()
{
}

void Building::Update(float deltaTime)
{
    if (pAnimator) {
        const AnimationFrame& frame = pAnimator->GetCurrentFrame();
        this->m_width = frame.width;
        this->m_height = frame.height;
    }
    UpdateAnimation(deltaTime);
}

void Building::LateUpdate()
{
}

void Building::Render(Gdiplus::Graphics* pGraphics)
{
    // RenderManager::RenderGameObject()에서 UpdateAnimation()과 GetBitmap()을 호출하여 렌더링
    // 개별 GameObject의 Render() 함수는 더 이상 필요하지 않음
}

void Building::Release()
{
    SafeDelete(pAnimator);
    for (auto& pair : m_animClips)
    {
        SafeDelete(pair.second);
    }
    m_animClips.clear();
}

void Building::Damaged(int damage)
{
    m_hp -= damage;
    if (m_hp <= 0)
    {
        m_hp = 0;
        m_state = BUILDING_DESTROYED;
        m_buildingState = BUILDING_DESTROYED;
        UpdateAnimatorState();
        // TODO: 건물 파괴 처리
    }
    else if (m_hp <= m_maxHp / 2)
    {
        m_state = BUILDING_DAMAGED;
        m_buildingState = BUILDING_DAMAGED;
        UpdateAnimatorState();
    }
}

void Building::SetTimeState(BuildingState buildingState)
{
    m_state = buildingState;
    m_buildingState = buildingState;
    UpdateAnimatorState();
}

BuildingState Building::GetTimeState() const
{
    return m_buildingState;
}

std::wstring Building::GetAnimKey(BuildingState state)
{
    // 기본 구현 - 파생 클래스에서 오버라이드 가능
    std::wstring key;
    if (state == BUILDING_NOON) {
        key = L"Building_Noon";
    }
    else if (state == BUILDING_NIGHT) {
        key = L"Building_Night";
    }
    else if (state == BUILDING_DAMAGED) {
        key = L"Building_Damaged";
    }
    else {
        key = L"Building_Destroyed";
    }
    return key;
}

// Unity Animator 스타일 애니메이션 등록
void Building::RegisterAllAnimations()
{
    OutputDebugStringW((L"Building: RegisterAllAnimations 시작 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

    // 기본 Building 클래스는 기본 애니메이션만 등록
    // 구체적인 건물들은 각각의 파생 클래스에서 처리
    
    OutputDebugStringW(L"Building: 기본 Building 클래스 - 기본 애니메이션 등록\n");
    
    OutputDebugStringW(L"Building: Unity Animator 스타일로 모든 애니메이션 등록 완료\n");
}

// Unity Animator 스타일 상태 업데이트
void Building::UpdateAnimatorState()
{
    if (pAnimator) {
        pAnimator->SetState(m_state, m_direction);
    }
}

void Building::UpdateAnimation(float deltaTime)
{
    if (pAnimator)
        pAnimator->Update(deltaTime);
}

Gdiplus::Bitmap* Building::GetBitmap() const
{
    if (!pAnimator) return nullptr;
    
    const SpriteSheet* spriteSheet = pAnimator->GetSpriteSheet();
    if (!spriteSheet) return nullptr;
    
    return spriteSheet->GetBitmap();
} 