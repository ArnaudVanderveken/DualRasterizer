#pragma once

struct Vertex_Input
{
	Elite::FPoint4 Position{};
	Elite::FVector2 UV{};
	Elite::FVector3 Normal{};
	Elite::FVector3 Tangent{};
	Elite::FVector3 viewDirection{};
};

enum class SampleMode
{
	point, linear, anisotropic, SIZE
};

enum class CullMode
{
	backface, frontface, none, SIZE
};

enum class RasterMode

{
	hardware, software, SIZE
};