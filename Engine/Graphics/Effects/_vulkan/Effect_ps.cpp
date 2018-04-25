/****************************************************************************
//	Usagi Engine, Copyright © Vitei, Inc. 2013
****************************************************************************/
#include "Engine/Common/Common.h"
#include "Engine/Core/File/File.h"
#include "Engine/Graphics/Device/GFXDevice.h"
#include API_HEADER(Engine/Graphics/Effects, Effect_ps.h)


namespace usg {



	Effect_ps::Effect_ps()
	{
		m_uStageCount = 0;
	}
	
	Effect_ps::~Effect_ps()
	{

	}

	// FIXME: Remove this old code, just a first pass so that we can take the OpenGL code as it has been defined
	void Effect_ps::GetShaderNames(const char* effectName, U8String &vsOut, U8String &psOut, U8String &gsOut)
	{
		U8String sourceFX = effectName;
		U8String sourcePath("shaders/");
		sourceFX += ".fx";

		File defFile(sourceFX.CStr());

		uint32 uPos = 0;
		char* szText;
		ScratchObj<char> scratchText(szText, (uint32)defFile.GetSize() + 1, FILE_READ_ALIGN);

		defFile.Read(defFile.GetSize(), szText);
		szText[defFile.GetSize()] = '\0';

		while (uPos < defFile.GetSize())
		{
			if (str::CompareLen(&szText[uPos], "PS", 2))
			{
				uPos += 3;
				psOut.CopySingleLine(&szText[uPos]);
				uPos += psOut.Length();
				psOut = sourcePath + psOut;
				continue;
			}

			if (str::CompareLen(&szText[uPos], "VS", 2))
			{
				uPos += 3;
				vsOut.CopySingleLine(&szText[uPos]);
				uPos += vsOut.Length();
				vsOut = sourcePath + vsOut;
				continue;
			}

			if (str::CompareLen(&szText[uPos], "GS", 2))
			{
				uPos += 3;
				gsOut.CopySingleLine(&szText[uPos]);
				uPos += gsOut.Length();
				gsOut = sourcePath + gsOut;
				continue;
			}

			uPos++;
		}
	}


	VkPipelineShaderStageCreateInfo Effect_ps::LoadShader(GFXDevice* pDevice, const U8String &fileName, VkShaderStageFlagBits stage)
	{
		VkPipelineShaderStageCreateInfo shaderStage = {};
		shaderStage.sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
		shaderStage.stage = stage;

		shaderStage.module = pDevice->GetPlatform().GetShaderFromStock(fileName.CStr(), stage);

		ASSERT(shaderStage.module != NULL);
		shaderStage.pName = "main"; // Entry point, could be anything...

		return shaderStage;
	}

	void Effect_ps::Init(GFXDevice* pDevice, const char* szEffectName)
	{
		U8String vertexName;
		U8String pixelName;
		U8String geomName;

		GetShaderNames(szEffectName, vertexName, pixelName, geomName);

		m_uStageCount = 0;
		m_stageCreateInfo[m_uStageCount++] = LoadShader(pDevice, vertexName, VK_SHADER_STAGE_VERTEX_BIT);
		m_stageCreateInfo[m_uStageCount++] = LoadShader(pDevice, pixelName, VK_SHADER_STAGE_FRAGMENT_BIT);

		if (geomName.Length() > 0)
		{
			m_stageCreateInfo[m_uStageCount++] = LoadShader(pDevice, geomName, VK_SHADER_STAGE_GEOMETRY_BIT);
		}
	}

}

