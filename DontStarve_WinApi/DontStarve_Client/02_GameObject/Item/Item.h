#pragma once
#include "../Entity/Entity.h"

class Item : public Entity
{
protected:
    std::wstring m_itemName;
    std::wstring m_description;
    
public:
    Item(GameObjectID id, const std::wstring& name, const std::wstring& desc,
         const std::wstring& baseDir = L"", const std::wstring& imageName = L"",
         float x = 0, float y = 0, float pivotX = 0.5f, float pivotY = 0.5f,
         Direction _dir = DIR_DOWN, bool isActive = true, bool isInteractive = true);
    virtual ~Item();

    virtual void Init() override;
    virtual void Release() override;

    virtual void Damaged(int damage) override {  }

    const std::wstring& GetItemName() const { return m_itemName; }
    const std::wstring& GetDescription() const { return m_description; }
};
