#include "../../99_Default/pch.h"
#include "Component.h"

Component::Component(GameObject* owner)
	: Object(), m_owner(owner)
{
}

Component::~Component()
{
}

