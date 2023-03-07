#include "pch.h"
#include "ECamera.h"
#include <SDL.h>

namespace Elite
{
	Camera::Camera(float width, float height, const FPoint3& position, const FVector3& viewForward, float fovAngle) :
		m_Width{ width },
		m_Height{ height },
		m_Fov(tanf((fovAngle* float(E_TO_RADIANS)) / 2.f)),
		m_Position{ position },
		m_ViewForward{GetNormalized(-viewForward)}
	{
		//Calculate initial matrices based on given parameters (position & target)
		CalculateLookAt();

		// Create ProjectionMatrix
		const float aspectRatio = m_Width / m_Height;

		m_ProjectionMatrix = Elite::FMatrix4{ 1 / aspectRatio / m_Fov, 0,         0,                        0,
								              0,                       1 / m_Fov, 0,                        0,
								              0,                       0,         m_Far / (m_Far - m_Near), -(m_Far * m_Near) / (m_Far - m_Near),
								              0,                       0,         1,                        0 };
	}

	void Camera::Update(float elapsedSec)
	{
		const float movementSpeed{ 3.f };

		//Capture Input (absolute) Rotation & (relative) Movement
		//*************
		//Keyboard Input
		const uint8_t* pKeyboardState = SDL_GetKeyboardState(0);
		float keyboardSpeed = pKeyboardState[SDL_SCANCODE_LSHIFT] ? m_KeyboardMoveSensitivity * m_KeyboardMoveMultiplier : m_KeyboardMoveSensitivity;
		m_RelativeTranslation.x = (pKeyboardState[SDL_SCANCODE_D] - pKeyboardState[SDL_SCANCODE_A]) * keyboardSpeed * elapsedSec * movementSpeed;
		m_RelativeTranslation.y = 0;
		m_RelativeTranslation.z = (pKeyboardState[SDL_SCANCODE_W] - pKeyboardState[SDL_SCANCODE_S]) * keyboardSpeed * elapsedSec * movementSpeed;
		m_RelativeTranslation.x += (pKeyboardState[SDL_SCANCODE_RIGHT] - pKeyboardState[SDL_SCANCODE_LEFT]) * keyboardSpeed * elapsedSec * movementSpeed;
		m_RelativeTranslation.z += (pKeyboardState[SDL_SCANCODE_UP] - pKeyboardState[SDL_SCANCODE_DOWN]) * keyboardSpeed * elapsedSec * movementSpeed;

		//Mouse Input
		int x, y = 0;
		uint32_t mouseState = SDL_GetRelativeMouseState(&x, &y);
		if (mouseState == SDL_BUTTON_LMASK)
		{
			m_RelativeTranslation.z += y * m_MouseMoveSensitivity * elapsedSec;
			m_AbsoluteRotation.y -= x * m_MouseRotationSensitivity;
		}
		else if (mouseState == SDL_BUTTON_RMASK)
		{
			m_AbsoluteRotation.x -= y * m_MouseRotationSensitivity;
			m_AbsoluteRotation.y += x * m_MouseRotationSensitivity;

		}
		else if (mouseState == (SDL_BUTTON_LMASK | SDL_BUTTON_RMASK))
		{
			m_RelativeTranslation.y -= y * m_MouseMoveSensitivity * elapsedSec;
		}

		//Update LookAt (view2world & world2view matrices)
		//*************
		CalculateLookAt();
	}

	const FMatrix4& Camera::GetWorldViewProjection() const
	{
		return m_ProjectionMatrix * m_WorldToView * m_TransformMatrix;
	}

	void Camera::CalculateLookAt()
	{
		//FORWARD (zAxis) with YAW applied
		FMatrix3 yawRotation = MakeRotationY(-m_AbsoluteRotation.y * float(E_TO_RADIANS));
		FVector3 zAxis = yawRotation * m_ViewForward;
		zAxis.x *= -1.f;
		zAxis.y *= -1.f;

		//Calculate RIGHT (xAxis) based on transformed FORWARD
		FVector3 xAxis = GetNormalized(Cross(FVector3{ 0.f,1.f,0.f }, zAxis));

		//FORWARD with PITCH applied (based on xAxis)
		FMatrix3 pitchRotation = MakeRotation(-m_AbsoluteRotation.x * float(E_TO_RADIANS), xAxis);
		zAxis = pitchRotation * zAxis;

		//Calculate UP (yAxis)
		FVector3 yAxis = Cross(zAxis, xAxis);

		//Translate based on transformed axis
		m_Position += m_RelativeTranslation.x * xAxis;
		m_Position += m_RelativeTranslation.y * yAxis;
		m_Position += m_RelativeTranslation.z * zAxis;

		//Construct View2World Matrix
		m_ViewToWorld =
		{
			FVector4{xAxis},
			FVector4{yAxis},
			FVector4{zAxis},
			FVector4{m_Position.x, m_Position.y, m_Position.z, 1.f}
		};

		//Construct World2View Matrix
		m_WorldToView = Inverse(m_ViewToWorld);
	}
}