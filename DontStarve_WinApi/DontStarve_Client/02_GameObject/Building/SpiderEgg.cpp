#include "99_Default/pch.h"
#include "SpiderEgg.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/AnimationClip.h"
#include "../../02_GameObject/Component/Transform/Transform.h"

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
    Building::Init(); // 부모 클래스 초기화
    
    OutputDebugStringW(L"SpiderEgg: Init 시작\n");
    m_buildingState = BUILDING_NOON;
    
    // Transform 컴포넌트 가져오기
    Transform* transform = GetComponent<Transform>();
    if (!transform) {
        OutputDebugStringW(L"SpiderEgg: Transform component not found!\n");
        return;
    }
    
    // Animator 생성 및 애니메이션 등록 (AnimationDefinition 패턴 제거)
    if (!m_animator) {
        m_animator = AddComponent<Animator>();
    }
    if (m_animator && transform) {
        ResourceManager* pRM = ResourceManager::GetInstance();
        std::wstring imagePath;
        const ResourcePathUtils::ObjectResourceDef* data = pRM->GetObjectResourceInfo(m_id);
        if (data) {
            std::wstring base = data->baseDir;
            if (!base.empty() && base.back() != L'\\' && base.back() != L'/') {
                base += L"\\";
            }
            
            if (m_id == GOID_BUILDING_SPIDER_SMALLEGG) {
                imagePath = base + L"Egg_spider_cocoon_small_Image.png";
            }
            else if (m_id == GOID_BUILDING_SPIDER_NORMALEGG) {
                imagePath = base + L"Egg_spider_cocoon_medium_Image.png";
            }
            else if (m_id == GOID_BUILDING_SPIDER_TALLEGG) {
                imagePath = base + L"Egg_spider_cocoon_large_Image.png";
            }
        }

        if (!imagePath.empty()) {
            m_animator->RegisterAnimation((int)BUILDING_NOON, DIR_DOWN, imagePath,
                80, 80, 1, 1, transform->GetPivotX(), transform->GetPivotY(), true, 0.03f);
            m_animator->SetState((int)m_buildingState, transform->GetDirection());
        }
    }
    
    OutputDebugStringW(L"SpiderEgg: Init 완료\n");
}

void SpiderEgg::LateInit()
{
    OutputDebugStringW(L"SpiderEgg: LateInit 호출\n");
}

void SpiderEgg::Update(float deltaTime)
{
	// 부모 클래스의 Update() 호출하여 컴포넌트 업데이트
	Building::Update(deltaTime);
}

void SpiderEgg::LateUpdate()
{
	// 부모 클래스의 LateUpdate() 호출하여 컴포넌트 업데이트
	Building::LateUpdate();
}

void SpiderEgg::Release()
{
	// SpiderEgg 전용 정리 작업
	m_animator = nullptr;
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	Building::Release();
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
