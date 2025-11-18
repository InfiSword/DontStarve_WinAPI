#include "../../99_Default/pch.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../02_GameObject/Player/Player.h"
#include "../../01_Manager/RenderManager/RenderManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../03_Animation/AnimationClip.h"
#include "../../03_Animation/SpriteSheet.h"
#include "Ingredient.h"

Ingredient::Ingredient(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
    : Item(GOBJ_ITEM, id, L"Ingredient", L"An ingredient item", resourcePath, imageName)
{
    SetPosition(x, y);
    SetPivot(pivotX, pivotY);
    SetActive(true);
}

Ingredient::~Ingredient()
{
}

void Ingredient::Init()
{
	SetInteractive(true); // Ingredient는 상호작용 가능
}

void Ingredient::LateInit()
{
}

void Ingredient::Update(float deltaTime)
{
}

void Ingredient::LateUpdate()
{
}

void Ingredient::Release()
{
}


void Ingredient::OnPlayerInteraction(Player* player) 
{
    if (GetActive()) {
        player->OnInteraction(this); 
        // 플레이어가 이 Ingredient를 인벤토리에서하도록 요청
    }
}
