#pragma once
#include "pch.h"
#include "BaseEffect.h"

class Effect final : public BaseEffect
{
public:
	Effect(ID3D11Device* pDevice, std::wstring assetFile);
	virtual ~Effect();

	Effect(const Effect&) = delete;
	Effect(Effect&&) noexcept = delete;
	Effect& operator=(const Effect&) = delete;
	Effect& operator=(Effect&&) noexcept = delete;

	virtual void SetTechnique(SampleMode renderTechnique) override;

private:

	ID3DX11EffectTechnique* m_pPointTechnique;
	ID3DX11EffectTechnique* m_pLinearTechnique;
	ID3DX11EffectTechnique* m_pAnisotropicTechnique;
};
