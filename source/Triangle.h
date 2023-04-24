#pragma once
#include "Geometry.h"
#include "structs.h"
#include "Texture.h"

class Triangle : public Geometry
{
public:
	Triangle(const Elite::FPoint3& pos, const std::vector<Vertex_Input>& localVertices = {});
	~Triangle() override = default;

	Triangle(const Triangle&) = delete;
	Triangle(Triangle&&) noexcept = delete;
	Triangle& operator=(const Triangle&) = delete;
	Triangle& operator=(Triangle&&) noexcept = delete;

	std::vector<Elite::FPoint4> GetVerticesWorldPositions() override final;
	[[nodiscard]] const std::vector<Vertex_Input>& GetColoredVertices() const;

	void SetLocalVertices(const std::vector<Vertex_Input>& newLocalVertices);

protected:
	std::vector<Vertex_Input> m_TriangleVertices;
};

