#pragma once
#include "pch.h"
#include "structs.h"

class BaseEffect abstract
{
public:
	BaseEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
	virtual ~BaseEffect();

	BaseEffect(const BaseEffect&) = delete;
	BaseEffect(BaseEffect&&) noexcept = delete;
	BaseEffect& operator=(const BaseEffect&) = delete;
	BaseEffect& operator=(BaseEffect&&) noexcept = delete;

	[[nodiscard]] ID3DX11Effect* GetEffect() const;
	[[nodiscard]] ID3DX11EffectTechnique* GetTechnique() const;
	[[nodiscard]] ID3DX11EffectVariable* GetVariableByName(LPCSTR name) const;

	virtual void SetTechnique(SampleMode renderTechnique) = 0;

protected:
	ID3DX11Effect* m_pEffect;
	ID3DX11EffectShaderResourceVariable* m_pShaderResourceVariable;
	ID3DX11EffectTechnique* m_pActiveTechnique;

private:
	static ID3DX11Effect* LoadEffect(ID3D11Device* pDevice, const std::wstring& assetFile);
};
