#pragma once

#include "GameObject.h"

class Item : public GameObject
{
public:
    Item(GameObjectType type, GameObjectID id, const std::wstring& name, const std::wstring& desc, const std::wstring resourcePath, const std::wstring& imagePath);
    virtual ~Item(); 

private:
    void LoadBitmap();  // 비트맵 로드 함수
};
