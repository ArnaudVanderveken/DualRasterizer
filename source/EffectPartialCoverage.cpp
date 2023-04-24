#include "pch.h"
#include <sstream>
#include "EffectPartialCoverage.h"

EffectPartialCoverage::EffectPartialCoverage(ID3D11Device* pDevice, const std::wstring& assetFile)
	: BaseEffect{ pDevice, assetFile }
{
	m_pPointTechnique = m_pEffect->GetTechniqueByName("PointTechnique");
	m_pLinearTechnique = m_pEffect->GetTechniqueByName("LinearTechnique");
	m_pAnisotropicTechnique = m_pEffect->GetTechniqueByName("AnisotropicTechnique");

	if (!m_pPointTechnique->IsValid())
		std::wcout << L"Point Technique not valid\n";

	if (!m_pLinearTechnique->IsValid())
		std::wcout << L"Linear Technique not valid\n";

	if (!m_pAnisotropicTechnique->IsValid())
		std::wcout << L"Anisotropic Technique not valid\n";

	m_pActiveTechnique = m_pPointTechnique;
}

EffectPartialCoverage::~EffectPartialCoverage()
{
	m_pPointTechnique->Release();
	m_pLinearTechnique->Release();
	m_pAnisotropicTechnique->Release();
}

void EffectPartialCoverage::SetTechnique(SampleMode renderTechnique)
{
	switch (renderTechnique)
	{
	case SampleMode::point:
		m_pActiveTechnique = m_pPointTechnique;
		break;

	case SampleMode::linear:
		m_pActiveTechnique = m_pLinearTechnique;
		break;

	case SampleMode::anisotropic:
		m_pActiveTechnique = m_pAnisotropicTechnique;
		break;

	default:
		break;
	}
}
