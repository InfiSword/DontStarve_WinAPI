#pragma once

#include "../../../Header/Enum.h"
#include <map>

// 크래프팅 레시피 엔트리 구조체
struct CraftingRecipeEntry {
	GameObjectID toolID;
	GameObjectID ingredient1ID;
	UINT ingredient1Count;
	GameObjectID ingredient2ID;
	UINT ingredient2Count;
};

// 크래프팅 레시피 정적 테이블 (pch에 포함되지 않음)
static constexpr CraftingRecipeEntry CraftingRecipeTable[] = {
	{ GOID_TOOL_GOLDEN_SCYTHE, GOID_ITEM_NORMAL_TWIGS, 1, GOID_ITEM_NORMAL_TREE_LOG, 1 },
	{ GOID_TOOL_HAM_BAT, GOID_ITEM_NORMAL_TWIGS, 1, GOID_ITEM_NORMAL_TREE_LOG, 1 },
	{ GOID_TOOL_PICKAXE, GOID_ITEM_NORMAL_TWIGS, 1, GOID_ITEM_NORMAL_TREE_LOG, 1 },
	{ GOID_TOOL_RED_AXE, GOID_ITEM_NORMAL_TWIGS, 1, GOID_ITEM_NORMAL_TREE_LOG, 1 },
	{ GOID_TOOL_SPEAR, GOID_ITEM_NORMAL_TWIGS, 1, GOID_ITEM_NORMAL_TREE_LOG, 1 },
	{ GOID_TOOL_SWAP_AXE, GOID_ITEM_NORMAL_ROCK, 1, GOID_ITEM_NORMAL_TWIGS, 2 },
	{ GOID_TOOL_SWAP_SPEAR, GOID_ITEM_NORMAL_TWIGS, 1, GOID_ITEM_NORMAL_TREE_LOG, 1 },
	{ GOID_TOOL_TORCH, GOID_ITEM_NORMAL_TWIGS, 1, GOID_ITEM_NORMAL_TREE_LOG, 1 },
	{ GOID_TOOL_HALBERD, GOID_ITEM_WOOD_2, 1, GOID_ITEM_MEAT, 1 },
	{ GOID_TOOL_HAMMER, GOID_ITEM_WOOD_2, 1, GOID_ITEM_NORMAL_ROCK, 1 },
	// 제작 재료 (CreateIcon)
	{ GOID_ITEM_CUT_NORMAL_STONE, GOID_ITEM_NORMAL_ROCK, 3, GOID_NONE, 0 },
	{ GOID_ITEM_ROPE, GOID_ITEM_CUT_NORMAL_GRASS, 3, GOID_ITEM_NORMAL_TWIGS, 1 },
	{ GOID_ITEM_WOOD_2, GOID_ITEM_NORMAL_TREE_LOG, 2, GOID_NONE, 0 },
	// 요리 (CookIcon)
	{ GOID_ITEM_COOKED_MONSTER_MEAT, GOID_ITEM_MONSTER_MEAT, 3, GOID_NONE, 0 },
	{ GOID_ITEM_COOKED_SMALL_MEAT, GOID_ITEM_SMALL_MEAT, 3, GOID_NONE, 0 },
	{ GOID_ITEM_COOKED_MEAT, GOID_ITEM_MEAT, 3, GOID_NONE, 0 }
};

static constexpr size_t CraftingRecipeCount = sizeof(CraftingRecipeTable) / sizeof(CraftingRecipeEntry);

// 레시피를 맵으로 변환하는 함수 (InventoryManager에서도 사용)
inline void LoadCraftingRecipesFromTable(std::map<GameObjectID, std::map<UINT, UINT>>& recipeMap) {
	recipeMap.clear();
	
	for (size_t i = 0; i < CraftingRecipeCount; ++i) 
	{
		const auto& recipe = CraftingRecipeTable[i];
		
		std::map<UINT, UINT> ingredientMap;
		ingredientMap[recipe.ingredient1ID] = recipe.ingredient1Count;
		if (recipe.ingredient2ID != GOID_NONE) {
			ingredientMap[recipe.ingredient2ID] = recipe.ingredient2Count;
		}

		recipeMap[recipe.toolID] = ingredientMap;
	}
}
