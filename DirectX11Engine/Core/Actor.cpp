#include "Actor.h"

void Actor::UpdateWorldMatrix()
{
	DirectX::XMMATRIX TranslationMatrix = DirectX::XMMatrixTranslation(Position.x, Position.y, Position.z);
	DirectX::XMMATRIX RotationMatrix = DirectX::XMMatrixRotationRollPitchYaw(Rotation.x, Rotation.y, Rotation.z);
	DirectX::XMMATRIX ScaleMatrix = DirectX::XMMatrixScaling(Scale.x, Scale.y, Scale.z);

	WorldMatrix = ScaleMatrix * RotationMatrix * TranslationMatrix;
}
