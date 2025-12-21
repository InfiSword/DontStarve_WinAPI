#include "../../99_Default/pch.h"
#include "Entity.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../03_Animation/Animator.h"

Entity::Entity(GameObjectType type, GameObjectID id, float x, float y, float pivotX, float pivotY, Direction _dir,
	const std::wstring& resourcePath, const std::wstring& imageName, bool isActive, bool isInteractive)
    :GameObject(type, id, x, y, pivotX, pivotY, _dir, resourcePath, imageName, isActive, isInteractive),
     m_animator(nullptr)
{

}

Entity::~Entity()
{
}

void Entity::Init()
{
    if (!GetAnimationDefinitions().empty()) {
        m_animator = AddComponent<Animator>();
        if (m_animator) {
            OutputDebugStringW(L"Entity: Animator 애니메이션 컴포넌트 생성 완료\n");
            
            // Animator에 애니메이션 정의를 직접 등록
            auto definitions = GetAnimationDefinitions();
            OutputDebugStringW((L"Entity: Init() - 애니메이션 정의 " + std::to_wstring(definitions.size()) + L"개 받음\n").c_str());
            
            for (const auto& def : definitions) {
                // 애니메이션 등록
                m_animator->RegisterAnimation(def.state, def.direction, def.imagePath,
                                            def.frameWidth, def.frameHeight,
                                            def.framesPerRow, def.totalFrames,
                                            def.frameDuration, def.pivotX, def.pivotY,
                                            def.isLoop, def.events);
            }
            
            OutputDebugStringW((L"Entity: Init() - 애니메이션 등록 완료\n"));
        } else {
            OutputDebugStringW(L"Entity: Animator 애니메이션 컴포넌트 생성 실패\n");
        }
    }
    
    GameObject::Init();
}

// 諛⑺뼢 愿젴 쑀떥由ы떚 븿닔뱾
Direction Entity::GetOppositeDirection(Direction dir)
{
    switch (dir)
    {
    case DIR_UP:    return DIR_DOWN;
    case DIR_DOWN:  return DIR_UP;
    case DIR_LEFT:  return DIR_RIGHT;
    case DIR_RIGHT: return DIR_LEFT;
    default:        return DIR_NONE;
    }
}

// 嫄곕━ 怨꾩궛 쑀떥由ы떚 븿닔뱾
float Entity::CalculateDistance(float x1, float y1, float x2, float y2)
{
    float dx = x2 - x1;
    float dy = y2 - y1;
    return sqrtf(dx * dx + dy * dy);
}

Direction Entity::GetDirectionToTarget(float fromX, float fromY, float toX, float toY)
{
    float dx = toX - fromX;
    float dy = toY - fromY;
    
    // 젅뙎媛믪씠 뜑 겙 諛⑺뼢쓣 슦꽑쟻쑝濡 꽑깮
    if (abs(dx) > abs(dy))
    {
        return (dx > 0) ? DIR_RIGHT : DIR_LEFT;
    }
    else
    {
        return (dy > 0) ? DIR_DOWN : DIR_UP;
    }
}

// 솕硫 踰붿쐞 솗씤 븿닔
bool Entity::IsPositionInScreenBounds(float x, float y)
{
    // 솕硫 醫뚰몴濡 蹂솚븯뿬 솗씤
    Gdiplus::PointF screenPos = CameraManager::GetInstance()->WorldToScreen(x, y);
    
    return (screenPos.X >= 0 && screenPos.X <= WINCX && 
            screenPos.Y >= 0 && screenPos.Y <= WINCY);
}
