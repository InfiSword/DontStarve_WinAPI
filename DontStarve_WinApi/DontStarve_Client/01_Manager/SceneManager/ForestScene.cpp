#include "99_Default/pch.h"
#include "ForestScene.h"
#include "../../02_GameObject/UI/IntroNoticeUI.h"
#include "../ObjectManager/ObjectManager.h"

ForestScene::ForestScene()
    : GameScene()
{
}

ForestScene::~ForestScene()
{
}

void ForestScene::Init(const MapData* mapData)
{
    GameScene::Init(mapData);
    
    // ForestScene 전용 초기화 로직
    IntroNoticeUI* introUI = new IntroNoticeUI();
    introUI->Init();
    ObjectManager::GetInstance()->AddGameObject(introUI);

    OutputDebugStringW(L"ForestScene: 초기화 완료\n");
}
