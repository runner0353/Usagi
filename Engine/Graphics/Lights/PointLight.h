/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
//	Description: A visible light
*****************************************************************************/
#pragma once

#ifndef USG_GRAPHICS_LIGHT_POINT_LIGHT_H
#define USG_GRAPHICS_LIGHT_POINT_LIGHT_H
#include "Engine/Common/Common.h"
#include "Engine/Maths/Sphere.h"
#include "Engine/Graphics/Color.h"
#include "Engine/Graphics/Device/DescriptorSet.h"
#include "Engine/Graphics/Effects/ConstantSet.h"
#include "Light.h"

namespace usg {

class Camera;
class AABB;
class OmniShadow;

class PointLight : public Light
{
public:
	PointLight();
	virtual ~PointLight(void);

	void Init(GFXDevice* pDevice, Scene* pScene, bool bSupportsShadow) override;
	void CleanUp(GFXDevice* pDevice, Scene* pScene) override;

	const ConstantSet* GetConstantSet() const { return &m_constants; }
	const DescriptorSet* GetDescriptorSet() const { return &m_descriptorSet; }
	const DescriptorSet* GetShadowDescriptorSet();

	virtual void	SetPosition(const Vector4f &position);

	float	GetNear() const			{ return m_fNear; }
	float	GetFar() const			{ return m_fFar; }

	virtual const Vector4f&	GetPosition() const;
	const usg::Sphere&		GetColSphere() const		{ return m_colSphere; }
	void EnableAttenuation(bool bEnable) { m_bAtten = bEnable; }
	bool HasAttenuation() const { return m_bAtten; }

	void SetRange(float fNear, float fFar);
	bool IsInRange(AABB& testBox);
	bool IsInVolume(const Vector4f& vPos) const;

	void GPUUpdate(GFXDevice* pDevice) override;
	void ShadowRender(GFXContext* pContext) override;

protected:

	ConstantSet		m_constants;
	DescriptorSet	m_descriptorSet;
	OmniShadow*		m_pShadow;

	Vector4f		m_position;
	float			m_fNear;
	float			m_fFar;
	bool			m_bAtten;
	usg::Sphere		m_colSphere;	

private:
	PointLight(PointLight &rhs) : Light(LIGHT_TYPE_POS) { ASSERT(false); } 
	PointLight& operator=(PointLight &rhs) { ASSERT(false); return *this; }
};


inline void PointLight::SetPosition(const Vector4f &position)
{
	m_position = position;
	m_colSphere.SetPos( position.v3() );
	m_bDirty = true;
}
 



inline void PointLight::SetRange(float fNear, float fFar)
{
	ASSERT(fFar > fNear);
	m_colSphere.SetRadius(fFar);
	m_fNear = fNear;
	m_fFar = fFar;
	m_bDirty = true;
}

} // namespace usg

#endif // USG_GRAPHICS_LIGHT_POINT_LIGHT_H
