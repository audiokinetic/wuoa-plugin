/*******************************************************************************
The content of this file includes portions of the AUDIOKINETIC Wwise Technology
released in source code form as part of the SDK installer package.

Commercial License Usage

Licensees holding valid commercial licenses to the AUDIOKINETIC Wwise Technology
may use this file in accordance with the end user license agreement provided
with the software or, alternatively, in accordance with the terms contained in a
written agreement between you and Audiokinetic Inc.

Apache License Usage

Alternatively, this file may be used under the Apache License, Version 2.0 (the
"Apache License"); you may not use this file except in compliance with the
Apache License. You may obtain a copy of the Apache License at
http://www.apache.org/licenses/LICENSE-2.0.

Unless required by applicable law or agreed to in writing, software distributed
under the Apache License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES
OR CONDITIONS OF ANY KIND, either express or implied. See the Apache License for
the specific language governing permissions and limitations under the License.

  Copyright (c) 2020 Audiokinetic Inc.
*******************************************************************************/

#include "WoaGainFX.h"
#include "../WoaGainConfig.h"

#include <AK/AkWwiseSDKVersion.h>

AK::IAkPlugin* CreateWoaGainFX(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, WoaGainFX());
}

AK::IAkPluginParam* CreateWoaGainFXParams(AK::IAkPluginMemAlloc* in_pAllocator)
{
    return AK_PLUGIN_NEW(in_pAllocator, WoaGainFXParams());
}

AK_IMPLEMENT_PLUGIN_FACTORY(WoaGainFX, AkPluginTypeEffect, WoaGainConfig::CompanyID, WoaGainConfig::PluginID)

WoaGainFX::WoaGainFX()
    : m_pParams(nullptr)
    , m_pAllocator(nullptr)
    , m_pContext(nullptr)
{
}

WoaGainFX::~WoaGainFX()
{
}

AKRESULT WoaGainFX::Init(AK::IAkPluginMemAlloc* in_pAllocator, AK::IAkEffectPluginContext* in_pContext, AK::IAkPluginParam* in_pParams, AkAudioFormat& in_rFormat)
{
    m_pParams = (WoaGainFXParams*)in_pParams;
    m_pAllocator = in_pAllocator;
    m_pContext = in_pContext;

    return AK_Success;
}

AKRESULT WoaGainFX::Term(AK::IAkPluginMemAlloc* in_pAllocator)
{
    AK_PLUGIN_DELETE(in_pAllocator, this);
    return AK_Success;
}

AKRESULT WoaGainFX::Reset()
{
    return AK_Success;
}

AKRESULT WoaGainFX::GetPluginInfo(AkPluginInfo& out_rPluginInfo)
{
    out_rPluginInfo.eType = AkPluginTypeEffect;
    out_rPluginInfo.bIsInPlace = true;
    out_rPluginInfo.uBuildVersion = AK_WWISESDK_VERSION_COMBINED;
    return AK_Success;
}

#define AK_LINTODB( __lin__ ) (log10f(__lin__) * 20.f)

void WoaGainFX::Execute(AkAudioBuffer* io_pBuffer)
{
    const AkUInt32 uNumChannels = io_pBuffer->NumChannels();

    // Monitor Data
    AkReal32 rmsBefore = 0.f;
    AkReal32 rmsAfter = 0.f;

    AkUInt16 uFramesProcessed;
    for (AkUInt32 i = 0; i < uNumChannels; ++i)
    {
        AkReal32* AK_RESTRICT pBuf = (AkReal32* AK_RESTRICT)io_pBuffer->GetChannel(i);

        uFramesProcessed = 0;
        while (uFramesProcessed < io_pBuffer->uValidFrames)
        {
            if (m_pContext->CanPostMonitorData())
                rmsBefore += powf(pBuf[uFramesProcessed], 2);

            pBuf[uFramesProcessed] = pBuf[uFramesProcessed] * AK_DBTOLIN(m_pParams->RTPC.fDummy);

            if (m_pContext->CanPostMonitorData())
                rmsAfter += powf(pBuf[uFramesProcessed], 2);

            ++uFramesProcessed;
        }
    }

    if (m_pContext->CanPostMonitorData())
    {
        // RMS = Root of the Mean of the Squares
        //       sqrt( (1/n) * sum_0-n( (x_1)^2, ..., (x_n)^2 ) )

        rmsBefore /= (uNumChannels * io_pBuffer->uValidFrames);
        rmsBefore = sqrtf(rmsBefore);

        rmsAfter /= (uNumChannels * io_pBuffer->uValidFrames);
        rmsAfter = sqrtf(rmsAfter);

        AkReal32 monitorData[2] = { AK_LINTODB(rmsBefore), AK_LINTODB(rmsAfter) };
        m_pContext->PostMonitorData((void*)monitorData, sizeof(monitorData));
    }
}

AKRESULT WoaGainFX::TimeSkip(AkUInt32 in_uFrames)
{
    return AK_DataReady;
}
