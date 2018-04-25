/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#ifndef _USG_GRAPHICS_EFFECT_BINDING_PS_H_
#define _USG_GRAPHICS_EFFECT_BINDING_PS_H_
#include "Engine/Common/Common.h"
#include "Engine/Graphics/Primitives/VertexDeclaration.h"
#include API_HEADER(Engine/Graphics/Effects, EffectBinding_ps.h)
#include OS_HEADER(Engine/Graphics/Device, VulkanIncludes.h)

namespace usg {

class Texture;
class Effect;
class GFXDevice;

class EffectBinding_ps
{
public:
	EffectBinding_ps();
	~EffectBinding_ps();

	void Init(GFXDevice* pDevice, EffectHndl pEffect, const VertexDeclaration** ppDecls, uint32 uCount);
	const VkPipelineVertexInputStateCreateInfo& GetVertexInputCreateInfo() const { return m_inputState; }

private:
	PRIVATIZE_COPY(EffectBinding_ps)

	VkVertexInputAttributeDescription*		m_pInputAttribs;
	VkPipelineVertexInputStateCreateInfo	m_inputState;
	VkVertexInputBindingDescription			m_bindingDesc[MAX_VERTEX_BUFFERS];
	const VertexDeclaration*				m_pDecl[MAX_VERTEX_BUFFERS];
	uint32									m_uBuffers;
};

}

#endif
