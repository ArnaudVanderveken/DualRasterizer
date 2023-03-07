#pragma once
#include "pch.h"
#include "BaseEffect.h"

class EffectPartialCoverage final : public BaseEffect
{
public:
	EffectPartialCoverage(ID3D11Device* pDevice, std::wstring assetFile);
	virtual ~EffectPartialCoverage();

	EffectPartialCoverage(const EffectPartialCoverage&) = delete;
	EffectPartialCoverage(EffectPartialCoverage&&) noexcept = delete;
	EffectPartialCoverage& operator=(const EffectPartialCoverage&) = delete;
	EffectPartialCoverage& operator=(EffectPartialCoverage&&) noexcept = delete;

	virtual void SetTechnique(SampleMode renderTechnique) override;

private:

	ID3DX11EffectTechnique* m_pPointTechnique;
	ID3DX11EffectTechnique* m_pLinearTechnique;
	ID3DX11EffectTechnique* m_pAnisotropicTechnique;
};
