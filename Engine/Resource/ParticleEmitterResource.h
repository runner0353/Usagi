/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_PARTICLE_EMITTER_RESOURCE_H_
#define _USG_PARTICLE_EMITTER_RESOURCE_H_
#include "Engine/Common/Common.h"
#include "Engine/Memory/MemHeap.h"
#include "Engine/Particles/Scripted/EffectGroup.pb.h"
#include "Engine/Particles/Scripted/ScriptEmitter.pb.h"
#include "Engine/Particles/Scripted/EmitterShapes.pb.h"
#include "Engine/Resource/ResourceBase.h"
#include "Engine/Resource/ResourcePak.pb.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include "Engine/Core/stl/vector.h"

namespace usg{


class ParticleEmitterResource : public ResourceBase
{
public:
	ParticleEmitterResource();
    virtual ~ParticleEmitterResource();

	bool Load(GFXDevice* pDevice, const vector<RenderPassHndl>& renderPasses, const char* szFileName);
	bool Load(GFXDevice* pDevice, const vector<RenderPassHndl>& renderPasses, const ParticleEmitterPak& pak, const void* pData, const char* szPackPath);

	const particles::EmitterEmission& GetEmissionDetails() const { return m_emissionDef;  }
	const usg::particles::EmitterShapeDetails& GetShapeDetails() const { return m_shapeDef; }
	PipelineStateHndl GetPipeline(const RenderPassHndl& renderPass) const;
	DescriptorSetLayoutHndl GetDescriptorLayout() const { return m_descriptorLayout;  }

	static EffectHndl GetEffect(GFXDevice* pDevice, ::usg::particles::ParticleType eParticleType);

private:
	void Load(GFXDevice* pDevice, const vector<RenderPassHndl>& renderPasses);

protected:

	particles::EmitterEmission			m_emissionDef;
	particles::EmitterShapeDetails		m_shapeDef;
	EffectHndl							m_pEffect;

	struct PipelineSet
	{
		RenderPassHndl		renderPass;
		PipelineStateHndl	pipeline;
	};

	vector<PipelineSet>					m_pipelines;
	DescriptorSetLayoutHndl				m_descriptorLayout;
};

}

#endif // _USG_EMITTER_RESOURCE_H_
