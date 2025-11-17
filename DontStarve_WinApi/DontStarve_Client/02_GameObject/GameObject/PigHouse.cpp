#include "../../99_Default/pch.h"
#include "PigHouse.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/AnimationClip.h"

PigHouse::PigHouse(GameObjectID id, float x, float y, float pivotX, float pivotY, 
    Direction _dir, const std::wstring& resourcePath,
    const std::wstring& imageName, int hp)
    : Building(id, x, y, pivotX, pivotY, _dir, resourcePath, imageName, hp)
{
    OutputDebugStringW(L"PigHouse: 생성자 호출\n");
}

PigHouse::~PigHouse()
{
    OutputDebugStringW(L"PigHouse: 소멸자 호출\n");
}

void PigHouse::Init()
{
    OutputDebugStringW(L"PigHouse: Init 시작\n");
    
    SetActive(true);
    SetInteractive(true);
    m_state = BUILDING_NOON;
    m_buildingState = BUILDING_NOON;
    m_direction = DIR_DOWN;
    
    pAnimator = new Animator();
    RegisterAllAnimations();
    UpdateAnimatorState();
    
    // 초기 크기 설정
    if (pAnimator) {
        const AnimationFrame& frame = pAnimator->GetCurrentFrame();
        this->m_width = frame.width;
        this->m_height = frame.height;
    }
    
    OutputDebugStringW(L"PigHouse: Init 완료\n");
}

void PigHouse::LateInit()
{
    OutputDebugStringW(L"PigHouse: LateInit 호출\n");
}

void PigHouse::Update(float deltaTime)
{
    if (pAnimator) {
        const AnimationFrame& frame = pAnimator->GetCurrentFrame();
        this->m_width = frame.width;
        this->m_height = frame.height;
    }
    UpdateAnimation(deltaTime);
}

void PigHouse::LateUpdate()
{
}

void PigHouse::Render(Gdiplus::Graphics* pGraphics)
{
    // RenderManager::RenderGameObject()에서 UpdateAnimation()과 GetBitmap()을 호출하여 렌더링
    // 개별 GameObject의 Render() 함수는 더 이상 필요하지 않음
}

void PigHouse::Release()
{
    OutputDebugStringW(L"PigHouse: Release 호출\n");
    SafeDelete(pAnimator);
    for (auto& pair : m_animClips)
    {
        SafeDelete(pair.second);
    }
    m_animClips.clear();
}

void PigHouse::Damaged(int damage)
{
    OutputDebugStringW((L"PigHouse: Damaged - 데미지: " + std::to_wstring(damage) + L"\n").c_str());
    
    m_hp -= damage;
    if (m_hp <= 0)
    {
        m_hp = 0;
        m_state = BUILDING_DESTROYED;
        m_buildingState = BUILDING_DESTROYED;
        UpdateAnimatorState();
        OutputDebugStringW(L"PigHouse: 파괴됨\n");
        // TODO: 건물 파괴 처리
    }
    else if (m_hp <= m_maxHp / 2)
    {
        m_state = BUILDING_DAMAGED;
        m_buildingState = BUILDING_DAMAGED;
        UpdateAnimatorState();
        OutputDebugStringW(L"PigHouse: 손상됨\n");
    }
}

void PigHouse::SetTimeState(BuildingState buildingState)
{
    m_state = buildingState;
    m_buildingState = buildingState;
    UpdateAnimatorState();
}

BuildingState PigHouse::GetTimeState() const
{
    return m_buildingState;
}

std::wstring PigHouse::GetAnimKey(BuildingState state)
{
    std::wstring key;
    if (state == BUILDING_NOON) {
        key = L"PigHouse_Noon";
    }
    else if (state == BUILDING_NIGHT) {
        key = L"PigHouse_Night";
    }
    else if (state == BUILDING_DAMAGED) {
        key = L"PigHouse_Damaged";
    }
    else {
        key = L"PigHouse_Destroyed";
    }
    return key;
}

// Unity Animator 스타일 애니메이션 등록
void PigHouse::RegisterAllAnimations()
{
    OutputDebugStringW(L"PigHouse: RegisterAllAnimations 시작\n");

    // ResourceManager를 사용하여 경로 구성
    auto* pRM = ResourceManager::GetInstance();
    
    // PIG HOUSE 애니메이션
    OutputDebugStringW(L"PigHouse: PIG HOUSE 애니메이션 등록\n");
    pAnimator->RegisterAnimation(BUILDING_NOON, DIR_DOWN,
        pRM->BuildObjectResourcePath(GOID_BUILDING_PIGHOUSE, L"", L"pig_house.png"),
        120, 120, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
    
    OutputDebugStringW(L"PigHouse: Unity Animator 스타일로 모든 애니메이션 등록 완료\n");
}

// Unity Animator 스타일 상태 업데이트
void PigHouse::UpdateAnimatorState()
{
    if (pAnimator) {
        pAnimator->SetState(m_state, m_direction);
    }
}

void PigHouse::UpdateAnimation(float deltaTime)
{
    if (pAnimator)
        pAnimator->Update(deltaTime);
}

Gdiplus::Bitmap* PigHouse::GetBitmap() const
{
    if (!pAnimator) return nullptr;
    
    const SpriteSheet* spriteSheet = pAnimator->GetSpriteSheet();
    if (!spriteSheet) return nullptr;
    
    return spriteSheet->GetBitmap();
}
