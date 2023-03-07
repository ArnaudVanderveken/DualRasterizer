/*=============================================================================*/
// Copyright 2021 Elite Engine 2.0
// Authors: Thomas Goussaert
/*=============================================================================*/
// ECamera.h: Base Camera Implementation with movement
/*=============================================================================*/

#pragma once
#include "pch.h"
#include "EMath.h"

namespace Elite
{
	class Camera
	{
	public:

		Camera(float width, float height, const FPoint3& position = { 0.f, 0.f, 0.f }, const FVector3& viewForward = { 0.f, 0.f, -1.f }, float fovAngle = 45.f);
		~Camera() = default;

		Camera(const Camera&) = delete;
		Camera(Camera&&) noexcept = delete;
		Camera& operator=(const Camera&) = delete;
		Camera& operator=(Camera&&) noexcept = delete;

		void Update(float elapsedSec);

		void SetWorldMatrix(const FMatrix4& matrix) { m_TransformMatrix = matrix; }
		const FMatrix4& GetWorldToView() const { return m_WorldToView; }
		const FMatrix4& GetViewToWorld() const { return m_ViewToWorld; }
		const FMatrix4& GetWorldViewProjection() const;
		const FMatrix4& GetProjectionMatrix() const { return m_ProjectionMatrix; }

		const float GetFov() const { return m_Fov; }

		float GetFar() const { return m_Far; }
		float GetNear() const { return m_Near; }

		FVector3 GetPosition() const { return FVector3(m_Position); }

	private:
		void CalculateLookAt();

		float m_Fov{};

		const float m_KeyboardMoveSensitivity{ 1.f };
		const float m_KeyboardMoveMultiplier{ 10.f };
		const float m_MouseRotationSensitivity{ .1f };
		const float m_MouseMoveSensitivity{ 2.f };

		const float m_Far{100.f};
		const float m_Near{0.1f};

		const float m_Width;
		const float m_Height;

		FPoint2 m_AbsoluteRotation{}; //Pitch(x) & Yaw(y) only
		FPoint3 m_RelativeTranslation{};

		FPoint3 m_Position{};
		const FVector3 m_ViewForward{};

		FMatrix4 m_TransformMatrix{};
		FMatrix4 m_WorldToView{};
		FMatrix4 m_ViewToWorld{};
		FMatrix4 m_ProjectionMatrix;
	};
}
