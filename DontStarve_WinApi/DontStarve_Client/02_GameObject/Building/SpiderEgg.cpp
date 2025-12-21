#include "../../99_Default/pch.h"
#include "SpiderEgg.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/AnimationClip.h"
#include "../../03_Animation/AnimationDefinition.h"

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
    m_buildingState = BUILDING_NOON;
    m_direction = DIR_DOWN;
    
    // 애니메이션 등록은 Animator::Init()에서 자동으로 처리됨
    
    OutputDebugStringW(L"SpiderEgg: Init 완료\n");
}

void SpiderEgg::LateInit()
{
    OutputDebugStringW(L"SpiderEgg: LateInit 호출\n");
}

void SpiderEgg::Update(float deltaTime)
{
}

void SpiderEgg::LateUpdate()
{
}

void SpiderEgg::Release()
{
}

void SpiderEgg::Damaged(int damage)
{
    OutputDebugStringW((L"SpiderEgg: Damaged - 데미지: " + std::to_wstring(damage) + L"\n").c_str());
    
    m_hp -= damage;
    if (m_hp <= 0)
    {
        m_hp = 0;
        m_buildingState = BUILDING_DESTROYED;
        OutputDebugStringW(L"SpiderEgg: 파괴됨\n");
        // TODO: 파괴 효과 처리
    }
    else if (m_hp <= m_maxHp / 2)
    {
        m_buildingState = BUILDING_DAMAGED;
        OutputDebugStringW(L"SpiderEgg: 손상됨\n");
    }
}

void SpiderEgg::SetTimeState(BuildingState buildingState)
{
    m_buildingState = buildingState;
}

BuildingState SpiderEgg::GetTimeState() const
{
    return m_buildingState;
}

std::vector<AnimationDefinition> SpiderEgg::GetAnimationDefinitions() const {
    std::vector<AnimationDefinition> definitions;
    ResourceManager* pRM = ResourceManager::GetInstance();
    
    AnimationDefinition anim;
    anim.state = static_cast<int>(BUILDING_NOON);
    anim.direction = DIR_DOWN;
    anim.frameWidth = 80;
    anim.frameHeight = 80;
    anim.framesPerRow = 1;
    anim.totalFrames = 1;
    anim.frameDuration = 0.1f;
    anim.pivotX = m_pivotX;
    anim.pivotY = m_pivotY;
    anim.isLoop = true;
    
    if (m_id == GOID_BUILDING_SPIDER_SMALLEGG) {
        anim.imagePath = pRM->BuildObjectResourcePath(GOID_BUILDING_SPIDER_SMALLEGG, L"", L"Egg_spider_cocoon_small_Image.png");
    }
    else if (m_id == GOID_BUILDING_SPIDER_NORMALEGG) {
        anim.imagePath = pRM->BuildObjectResourcePath(GOID_BUILDING_SPIDER_NORMALEGG, L"", L"Egg_spider_cocoon_medium_Image.png");
    }
    else if (m_id == GOID_BUILDING_SPIDER_TALLEGG) {
        anim.imagePath = pRM->BuildObjectResourcePath(GOID_BUILDING_SPIDER_TALLEGG, L"", L"Egg_spider_cocoon_large_Image.png");
    }
    else {
        return definitions; // 알 수 없는 ID면 빈 벡터 반환
    }
    
    definitions.push_back(anim);
    return definitions;
}
