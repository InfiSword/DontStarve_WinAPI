#include "99_Default/pch.h"
#include "IntroNoticeUI.h"
#include "UIImage.h"
#include "UIText.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/InputManager/InputManager.h"

IntroNoticeUI::IntroNoticeUI()
    : UIElement(static_cast<GameObjectID>(GOID_UI_INTRO), L"", L"", NONE, true, true)
{
}

IntroNoticeUI::~IntroNoticeUI()
{
    Release();
}

void IntroNoticeUI::Init()
{
    // 배경 이미지 생성 (중앙)
    UIImage* pBg = new UIImage(
        static_cast<GameObjectID>(GOID_UI_IMAGE),
        700.0f, 550.0f,
        LAYER_UI_BACKGROUND,
        L"Resource/UI/EditorMenu.png",
        0.0f,
        0.5f, 0.5f,
        0.5f, 0.5f,
        0.0f, 0.0f
    );
    m_managedObjects.push_back(pBg);
    ObjectManager::GetInstance()->AddGameObject(pBg);

    // 텍스트 생성
    float startY = -100.0f;
    float lineSpacing = 50.0f;

    std::vector<std::wstring> introTexts = {
        L"안녕하세요! 돈스타브 세계에 오신걸 환영합니다.",
        L"해당 게임의 최종 목표는",
		L"거미 여왕 보스를 클리어 하는 것을 목표로",
        L"게임이 진행됩니다. 거미 여왕을 물리치고",
        L"게임을 클리어 해 보세요!",
        L"",
        L"\"화면 아무곳이나 클릭하면 창이 닫힙니다\""
    };

    for (size_t i = 0; i < introTexts.size(); ++i)
    {
        Gdiplus::Color textColor =  Gdiplus::Color::White;
        float fontSize = 15.0f;

        UIText* pText = new UIText(
            static_cast<GameObjectID>(GOID_UI_TEXT),
            600.0f, 40.0f,
            introTexts[i],
            textColor,
            LAYER_UI_FOREGROUND,
            0.1f,
            L"맑은 고딕",
            fontSize,
            Gdiplus::FontStyleBold,
            Gdiplus::StringAlignmentCenter,
            Gdiplus::StringAlignmentCenter,
            0.5f, 0.5f,
            0.5f, 0.5f,
            0.0f, startY + (i * lineSpacing)
        );
        m_managedObjects.push_back(pText);
        ObjectManager::GetInstance()->AddGameObject(pText);
    }
}

void IntroNoticeUI::Update(float deltaTime)
{
    if (InputManager::GetInstance()->IsLButtonClicked())
    {
        ObjectManager* pOM = ObjectManager::GetInstance();
        for (auto obj : m_managedObjects)
        {
            pOM->RemoveGameObject(obj);
        }
        pOM->RemoveGameObject(this);
    }
}

void IntroNoticeUI::Release()
{
    m_managedObjects.clear();
}
