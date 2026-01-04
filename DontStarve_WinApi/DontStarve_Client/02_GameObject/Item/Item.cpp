#include "../../99_Default/pch.h"
#include "Item.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"

Item::Item(GameObjectType type, GameObjectID id, const std::wstring& name, const std::wstring& desc,
	const std::wstring resourcePath, const std::wstring& imagePath,
	float x, float y, float pivotX, float pivotY, Direction dir, bool isActive, bool isInteractive)
    : GameObject(type, id, resourcePath, imagePath, isActive, isInteractive)
{
    m_name = name;
    m_description = desc;
}

Item::~Item() 
{
  
}



