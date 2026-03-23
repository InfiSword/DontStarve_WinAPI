#include "99_Default/pch.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../01_Manager/RenderManager/RenderManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../03_Animation/AnimationClip.h"
#include "../../03_Animation/SpriteSheet.h"
#include "Ingredient.h"

Ingredient::Ingredient(GameObjectID id, const std::wstring& name, const std::wstring& desc, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
    : Item(id, name, desc, resourcePath, imageName, x, y, pivotX, pivotY, DIR_DOWN, true, true)
{

}

Ingredient::~Ingredient()
{
}

void Ingredient::Init()
{
    Item::Init();
}

void Ingredient::LateInit()
{
}

void Ingredient::Update(float deltaTime)
{
	// 부모 클래스의 Update() 호출하여 컴포넌트 업데이트
	Item::Update(deltaTime);
}

void Ingredient::LateUpdate()
{
	// 부모 클래스의 LateUpdate() 호출하여 컴포넌트 업데이트
	GameObject::LateUpdate();
}

void Ingredient::Release()
{
	// Ingredient 전용 정리 작업
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	Item::Release();
}

