#pragma once
#include "UIElement.h"
#include <vector>

class UIImage;
class UIText;

class IntroNoticeUI : public UIElement
{
public:
    IntroNoticeUI();
    virtual ~IntroNoticeUI();

    virtual void Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Release() override;

private:
    std::vector<GameObject*> m_managedObjects;
};
