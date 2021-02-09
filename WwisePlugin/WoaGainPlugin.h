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

#pragma once

#include <windows.h>
#define _ATL_CSTRING_EXPLICIT_CONSTRUCTORS
#include <atlstr.h>

#include <AK/Wwise/AudioPlugin.h>

/// See https://www.audiokinetic.com/library/edge/?source=SDK&id=plugin__dll.html
/// for the documentation about Authoring plug-ins
class WoaGainPlugin
    : public AK::Wwise::DefaultAudioPluginImplementation
{
public:
    WoaGainPlugin();
    ~WoaGainPlugin();

    //=== Core Methods ===

    /// This will be called to delete the plug-in.
    /// The object is responsible for deleting itself when this method is called.
    void Destroy() override;

    /// The property set interface is given to the plug-in through this method.
    /// It is called by Wwise during initialization of the plug-in, before most other calls.
    void SetPluginPropertySet(AK::Wwise::IPluginPropertySet* in_pPSet) override;

    /// This function is called by Wwise to obtain parameters that will be written to a bank.
    /// Because these can be changed at run-time, the parameter block should stay relatively small.
    // Larger data should be put in the Data Block.
    bool GetBankParameters(const GUID& in_guidPlatform, AK::Wwise::IWriteData* in_pDataWriter) const override;

    //=== Frontend Methods ===

    /// Retrieve the handle holding resources for the module containing the plug-in instance
    HINSTANCE GetResourceHandle() const override;

    /// Retrieve the dialog ID and its table mapping control ID to properties
    bool GetDialog(
        eDialog in_eDialog,				///< The dialog type
        UINT & out_uiDialogID,			///< The returned resource ID of the dialog
        AK::Wwise::PopulateTableItem *& out_pTable	///< The returned table of property-control bindings (can be NULL)
    ) const override;

    /// Window message handler
    bool WindowProc(
        eDialog in_eDialog,		///< The dialog type
        HWND in_hWnd,			///< The window handle of the dialog
        UINT in_message,		///< The incoming message. This is a standard Windows message ID (ex. WM_PAINT).
        WPARAM in_wParam,		///< The WPARAM of the message (see MSDN)
        LPARAM in_lParam,		///< The LPARAM of the message (see MSDN)
        LRESULT & out_lResult 	///< The returned value if the message has been processed (it is only considered if the method also returns True)
    ) override;

    /// Function called when user clicked the help [?] button
    bool Help(
        HWND in_hWnd,					///< The handle of the dialog
        eDialog in_eDialog,				///< The dialog type
        LPCWSTR in_szLanguageCode		///< The language code in ISO639-1
    ) const override;

    /// Function called when monitor data is posted for our plug-in type
    void NotifyMonitorData(
        const AK::Wwise::IAudioPlugin::MonitorData * in_pData,
        unsigned int in_uDataSize,
        bool in_bNeedsByteSwap,
        bool in_bRealtime
    ) override;

private:
    AK::Wwise::IPluginPropertySet* m_pPSet;
    HWND m_hwnd;
};
