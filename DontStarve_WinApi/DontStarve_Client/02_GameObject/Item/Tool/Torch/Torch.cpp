#include "99_Default/pch.h"
#include "Torch.h"
#include "../../../Entity/Entity.h"

static float GetTorchDamage(GameObjectID id) {
	switch (id) {
		case GOID_TOOL_TORCH: return 5.0f;
		default:              return 5.0f;
	}
}

Torch::Torch(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName)
	: Tool(id, name, desc, baseDir, imageName, GetTorchDamage(static_cast<GameObjectID>(id)))
{
}

Torch::~Torch()
{
}

void Torch::Use(float durabilityCost)
{
	Tool::Use(durabilityCost);
}
