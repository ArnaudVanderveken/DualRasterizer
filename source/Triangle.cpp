#include "pch.h"
#include "Triangle.h"

Triangle::Triangle(const Elite::FPoint3& pos, const std::vector<Vertex_Input>& localVertices)
	: Geometry{ pos }
	, m_TriangleVertices{ localVertices }
{
}

std::vector<Elite::FPoint4> Triangle::GetVerticesWorldPositions()
{
	std::vector<Elite::FPoint4> positions;

	for (Vertex_Input& vertex : m_TriangleVertices)
	{
		positions.push_back(Elite::FPoint4{ vertex.Position + Elite::FVector4(Elite::FVector3(m_Position), 0.0f) });
	}

	return positions;
}

const std::vector<Vertex_Input>& Triangle::GetColoredVertices() const
{
	return m_TriangleVertices;
}

void Triangle::SetLocalVertices(const std::vector<Vertex_Input>& vertices)
{
	m_TriangleVertices = vertices;
}
