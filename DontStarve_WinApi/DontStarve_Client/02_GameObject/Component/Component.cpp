#include "../../99_Default/pch.h"
#include "Component.h"

Component::Component(GameObject* owner)
	: m_owner(owner), m_enabled(true)
{
}

Component::~Component()
{
}

