#pragma once
#include "EMath.h"
#include "ERGBColor.h"

struct InputVertex
{
	Elite::FPoint3 Position;
	Elite::FVector2 UV;
	Elite::FVector3 Normal;
	Elite::FVector3 Tangent;
	Elite::RGBColor Color;

	bool operator==(const InputVertex& rhs) const {
		return (Position == rhs.Position && UV == rhs.UV && Normal == rhs.Normal);
	}
};

struct OutputVertex
{
	Elite::FPoint4 Position;
	Elite::FVector2 UV;
	Elite::FVector3 Normal;
	Elite::FVector3 Tangent;
	Elite::RGBColor Color;
	Elite::FVector3 ViewDirection;
};

enum class RenderMode {
	Rasterizer = -1,
	DirectX = 1
};