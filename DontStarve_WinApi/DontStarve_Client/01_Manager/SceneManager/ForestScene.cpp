#include "99_Default/pch.h"
#include "ForestScene.h"

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
    
    // ForestScene 전용 초기화 로직 (필요 시 추가)
    OutputDebugStringW(L"ForestScene: 초기화 완료\n");
}
