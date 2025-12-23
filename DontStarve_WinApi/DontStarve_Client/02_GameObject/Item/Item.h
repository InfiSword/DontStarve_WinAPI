#pragma once
#include "../GameObject.h"

class Item : public GameObject
{
public:
    Item(GameObjectType type, GameObjectID id, const std::wstring& name, const std::wstring& desc,
        const std::wstring resourcePath, const std::wstring& imagePath,
        float x = 0.0f, float y = 0.0f, float pivotX = 0.5f, float pivotY = 0.5f,
        Direction dir = DIR_DOWN, bool isActive = true, bool isInteractive = false);
    virtual ~Item(); 

private:
    void LoadBitmap();  
};
