#pragma once
#include "EMath.h"
#include <vector>

class Geometry
{
public:
	Geometry(const Elite::FPoint3& pos);
	virtual ~Geometry() = default;

	Geometry(const Geometry&) = delete;
	Geometry(Geometry&&) noexcept = delete;
	Geometry& operator=(const Geometry&) = delete;
	Geometry& operator=(Geometry&&) noexcept = delete;

	virtual std::vector<Elite::FPoint4> GetVerticesWorldPositions() = 0;

protected:
	Elite::FPoint3 m_Position;
};

