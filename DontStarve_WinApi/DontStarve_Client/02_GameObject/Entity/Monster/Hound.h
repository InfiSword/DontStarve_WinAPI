#pragma once
#include "Monster.h"

class Hound : public Monster
{
public:
    Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Hound();

    virtual void Init() override;
    virtual void OnInteraction(GameObject* obj) override;
    
    // 애니메이션 정의 제공 (Entity 오버라이드)
    virtual std::vector<AnimationDefinition> GetAnimationDefinitions() const override;
};
