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

#include "WoaGainPlugin.h"
#include "resource.h"

#include "../SoundEnginePlugin/WoaGainFXFactory.h"

#include <AK/Tools/Common/AkAssert.h>

#include <shellapi.h>
#include <string>


WoaGainPlugin::WoaGainPlugin()
    : m_pPSet(nullptr)
    , m_hwnd(NULL)
{
}

WoaGainPlugin::~WoaGainPlugin()
{
}

void WoaGainPlugin::Destroy()
{
    delete this;
}

void WoaGainPlugin::SetPluginPropertySet(AK::Wwise::IPluginPropertySet* in_pPSet)
{
    m_pPSet = in_pPSet;
}

bool WoaGainPlugin::GetBankParameters(const GUID& in_guidPlatform, AK::Wwise::IWriteData* in_pDataWriter) const
{
    // Write bank data here
    CComVariant varProp;
    m_pPSet->GetValue(in_guidPlatform, L"Dummy", varProp);
    in_pDataWriter->WriteReal32(varProp.fltVal);

    return true;
}

// Acquire the module instance from the Microsoft linker
extern "C" IMAGE_DOS_HEADER __ImageBase;

HINSTANCE WoaGainPlugin::GetResourceHandle() const
{
    return ((HINSTANCE)&__ImageBase);

    // OR using MFC:
    //AFX_MANAGE_STATE( AfxGetStaticModuleState() );
    //return AfxGetStaticModuleState()->m_hCurrentResourceHandle;
}

// These macros generate a table named "WoaGainProperties" to pass to GetDialog
// See https://www.audiokinetic.com/library/edge/?source=SDK&id=wwiseplugin_dialog_guide.html#wwiseplugin_dialog_guide_poptable
//
// The preprocessor turns the code below into:
// AK::Wwise::PopulateTableItem WoaGainProperties = {
//    {IDC_GAIN_SLIDER, L"Dummy"},
//    {0, NULL}
// };
AK_BEGIN_POPULATE_TABLE(WoaGainProperties)
    AK_POP_ITEM(
        IDC_GAIN_SLIDER, /* < ID of the Win32 control in resource.h and WoaGain.rc */
        L"Dummy"         /* < Property Name in WoaGain.xml */
    )
AK_END_POPULATE_TABLE()

// Return true = Custom GUI
// Return false = Generated GUI
bool WoaGainPlugin::GetDialog(
    eDialog in_eDialog,				///< The dialog type
    UINT & out_uiDialogID,			///< The returned resource ID of the dialog
    AK::Wwise::PopulateTableItem *& out_pTable	///< The returned table of property-control bindings (can be NULL)
) const
{
    // Which dialog type is being requested?
    switch (in_eDialog)
    {
        // Plug-in Settings: Available to all plug-ins
        case SettingsDialog:
        {
            out_uiDialogID = IDD_WOA_DIALOG;
            out_pTable = WoaGainProperties;
            return true;
        }
        // Contents Editor: Only available to source plug-ins
        case ContentsEditorDialog:
        default:
        {
            return false;
        }
    }
}

// Window message handler
// See https://docs.microsoft.com/en-us/previous-versions/windows/desktop/legacy/ms633573(v=vs.85)
bool WoaGainPlugin::WindowProc(
    eDialog in_eDialog,		///< The dialog type
    HWND in_hWnd,			///< The window handle of the dialog
    UINT in_message,		///< The incoming message. This is a standard Windows message ID (ex. WM_PAINT).
    WPARAM in_wParam,		///< The WPARAM of the message (see MSDN)
    LPARAM in_lParam,		///< The LPARAM of the message (see MSDN)
    LRESULT & out_lResult 	///< The returned value if the message has been processed (it is only considered if the method also returns True)
)
{
    // Effects only support the dialog type "SettingsDialog"
    if (in_eDialog != SettingsDialog)
        return false;

    bool messageWasHandled = true;

    switch (in_message)
    {
        // Sent to the dialog box procedure immediately before a dialog box is displayed.
        // Dialog box procedures typically use this message to initialize controls and carry out any other
        // initialization tasks that affect the appearance of the dialog box.
        case WM_INITDIALOG:
        {
            m_hwnd = in_hWnd;

            // return TRUE to direct the system to set the keyboard focus to the control specified by wParam
            // return FALSE to prevent the system from setting the default keyboard focus
            out_lResult = FALSE;
            break;
        }
        // Sent when a window is being destroyed.
        // It is sent to the window procedure of the window being destroyed after the window is removed from the screen.
        case WM_DESTROY:
        {
            m_hwnd = NULL;

            // If an application processes this message, it should return zero.
            out_lResult = 0;
            break;
        }
        default:
        {
            messageWasHandled = false;
            break;
        }
    }

    return messageWasHandled;
}

// Function called when user clicked the help [?] button
// See https://www.audiokinetic.com/library/edge/?source=SDK&id=plugin_dll.html#wwiseplugin_help
bool WoaGainPlugin::Help(
    HWND in_hWnd,					///< The handle of the dialog
    eDialog in_eDialog,				///< The dialog type
    LPCWSTR in_szLanguageCode		///< The language code in ISO639-1
) const
{
    if (in_eDialog == SettingsDialog && wcslen(in_szLanguageCode) == 2)
    {
        wchar_t url[128] = { 0 };
        wsprintf(url, L"https://www.audiokinetic.com/%s/library/edge/?source=SDK&id=plugin_dll.html", in_szLanguageCode);
        // Let Windows open the URL through the default application
        ::ShellExecute(0, 0, url, 0, 0 , SW_SHOW);
        return true;
    }
    return false;
}

// See https://www.audiokinetic.com/library/edge/?source=SDK&id=plugin_dll.html#wwiseplugin_dll_notifymonitordata
void WoaGainPlugin::NotifyMonitorData(
    const AK::Wwise::IAudioPlugin::MonitorData * in_pData,
    unsigned int in_uDataSize,
    bool in_bNeedsByteSwap,
    bool in_bRealtime
)
{
    // Below are pedantic validation for demonstration purposes

    if (m_hwnd != NULL &&          // The dialog exists
        in_pData != nullptr &&     // The data payload is not null
        in_uDataSize >= 1 &&       // Received data from at least 1 instance
        in_bNeedsByteSwap == false // Only handle little-endian to little-endian
    ) {
        // TODO: This handles only a single instance!
        //       We _must_ handle all instances (in_uDataSize = number of instances)

        if (in_pData->pData != nullptr &&               // The monitor data is not null
            in_pData->uDataSize == sizeof(AkReal32) * 2 // 
        ) {
            AkReal32* serializedData = (AkReal32*)in_pData->pData;

            HWND inputLvlLabel = ::GetDlgItem(m_hwnd, IDC_INPUT_LVL);
            ::SetWindowTextW(inputLvlLabel, std::to_wstring(serializedData[0]).c_str());

            HWND outputLvlLabel = ::GetDlgItem(m_hwnd, IDC_OUTPUT_LVL);
            ::SetWindowTextW(outputLvlLabel, std::to_wstring(serializedData[1]).c_str());
        }
    }
}