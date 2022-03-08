// CWMPHost.cpp : Implementation of the CWMPHost
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//
#include "stdafx.h"
#include "CWMPHost.h"
#include "dialogs.h"
#include <iostream>
#include <stdlib.h>
#include <string>
#include <vector>
#include <stdlib.h>

#include "atlbase.h"
#include "atlstr.h"
#include "comutil.h"
#include <filesystem>


BSTR selected_folder_macro;
BSTR SELECTED_VIDEO_MACRO;
BSTR SELECTED_VIDEOFILENAME_MACRO;
BSTR SELECTED_SLOWDOWN_MACRO;
/////////////////////////////////////////////////////////////////////////////
// CWMPHost

void CWMPHost::OnFinalMessage(HWND /*hWnd*/)
{
    ::PostQuitMessage(0);
}

LRESULT CWMPHost::OnCreate(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */)
{
    AtlAxWinInit();
    CComPtr<IAxWinHostWindow>           spHost;
    CComPtr<IConnectionPointContainer>  spConnectionContainer;
    CComWMPEventDispatch                *pEventListener = NULL;
    CComPtr<IWMPEvents>                 spEventListener;
    HRESULT                             hr;
    RECT                                rcClient;

    m_dwAdviseCookie = 0;

    // create window

    GetClientRect(&rcClient);
    m_wndView.Create(m_hWnd, rcClient, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
    if (NULL == m_wndView.m_hWnd)
        goto FAILURE;
    
    // load OCX in window

    hr = m_wndView.QueryHost(&spHost);
    if (FAILMSG(hr))
        goto FAILURE;

    hr = spHost->CreateControl(CComBSTR(L"{6BF52A52-394A-11d3-B153-00C04F79FAA6}"), m_wndView, 0);
    if (FAILMSG(hr))
        goto FAILURE;

    hr = m_wndView.QueryControl(&m_spWMPPlayer);
    if (FAILMSG(hr))
        goto FAILURE;

    // start listening to events

    hr = CComWMPEventDispatch::CreateInstance(&pEventListener);
    spEventListener = pEventListener;
    if (FAILMSG(hr))
        goto FAILURE;

    hr = m_spWMPPlayer->QueryInterface(&spConnectionContainer);
    if (FAILMSG(hr))
        goto FAILURE;

    // See if OCX supports the IWMPEvents interface
    hr = spConnectionContainer->FindConnectionPoint(__uuidof(IWMPEvents), &m_spConnectionPoint);
    if (FAILED(hr))
    {
        // If not, try the _WMPOCXEvents interface, which will use IDispatch
        hr = spConnectionContainer->FindConnectionPoint(__uuidof(_WMPOCXEvents), &m_spConnectionPoint);
        if (FAILMSG(hr))
            goto FAILURE;
    }

    hr = m_spConnectionPoint->Advise(spEventListener, &m_dwAdviseCookie);
    if (FAILMSG(hr))
        goto FAILURE;

    return 0;

FAILURE:
    ::PostQuitMessage(0);
    return 0;
}

LRESULT CWMPHost::OnDestroy(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& bHandled )
{
    // stop listening to events

    if (m_spConnectionPoint)
    {
        if (0 != m_dwAdviseCookie)
            m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
        m_spConnectionPoint.Release();
    }

    // close the OCX

    if (m_spWMPPlayer)
    {
        m_spWMPPlayer->close();
        m_spWMPPlayer.Release();
    }

    bHandled = FALSE;
    return 0;
}

LRESULT CWMPHost::OnErase(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */)
{
    return 1;
}

LRESULT CWMPHost::OnSize(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */)
{
    RECT rcClient;
    GetClientRect(&rcClient);
    m_wndView.MoveWindow(rcClient.left, rcClient.top, rcClient.right, rcClient.bottom);
    return 0;
}

LRESULT CWMPHost::OnFileOpen(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    CFileOpenDlg dlgOpen;
    HRESULT      hr;

    if (dlgOpen.DoModal(m_hWnd) == IDOK)
    {
        hr = m_spWMPPlayer->put_URL(dlgOpen.m_bstrName);
        auto path = SysAllocString(dlgOpen.m_bstrName);
        SELECTED_VIDEO_MACRO = path;
        SELECTED_VIDEOFILENAME_MACRO = extract_filename(path);

        CString source = SELECTED_VIDEO_MACRO;
        CString restOfFolder = L"Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1";
        CString solution = MY_SOLUTIONDIR;
        CString target = solution + restOfFolder;

        OutputDebugString(target);
        SHFILEOPSTRUCT SH = { 0 };

        SH.hwnd = NULL;
        SH.wFunc = FO_COPY;

        SH.fFlags = NULL;
        SH.fFlags |= FOF_SILENT;
        SH.fFlags |= FOF_NOCONFIRMMKDIR;
        SH.fFlags |= FOF_NOCONFIRMATION;
        SH.fFlags |= FOF_WANTMAPPINGHANDLE;
        SH.fFlags |= FOF_NOERRORUI;
        
        std::vector<TCHAR> sourceBuffer;
        std::vector<TCHAR> targetBuffer;

        sourceBuffer.resize(source.GetLength() + 1);
        memcpy(&(sourceBuffer[0]), source.operator LPCTSTR(),
            sizeof(TCHAR) * (source.GetLength() + 1));  // (1)
        sourceBuffer.push_back('\0');

        targetBuffer.resize(target.GetLength() + 1);
        memcpy(&(targetBuffer[0]), target.operator LPCTSTR(),
            sizeof(TCHAR) * (target.GetLength() + 1));  // (1)
        targetBuffer.push_back('\0');

        SH.pFrom = &(sourceBuffer[0]);
        SH.pTo = &(targetBuffer[0]);
        ::SHFileOperation(&SH);
      
        if (FAILMSG(hr))
            return 0;
    }
    return 0;
}

LRESULT CWMPHost::OnFileExit(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    DestroyWindow();
    return 0;
}

LRESULT CWMPHost::OnWMPCoreClose(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    HRESULT     hr;

    hr = m_spWMPPlayer->close();
    if (FAILMSG(hr))
        return 0;

    return 0;
}

LRESULT CWMPHost::OnWMPCoreURL(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    CComBSTR    m_bstrValue;
    HRESULT     hr;

    hr = m_spWMPPlayer->get_URL(&m_bstrValue);
    if (FAILMSG(hr))
        return 0;

    CStringDlg dlgString(L"IWMPCore->URL", m_bstrValue);

    if (dlgString.DoModal(m_hWnd) == IDOK)
    {
        hr = m_spWMPPlayer->put_URL(dlgString.m_bstrValue);
        if (FAILMSG(hr))
            return 0;
    }
    return 0;
}

LRESULT CWMPHost::OnWMPCoreOpenState(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    HRESULT      hr;
    WMPOpenState osValue;

    hr = m_spWMPPlayer->get_openState(&osValue);
    if (FAILMSG(hr))
        return 0;

    WCHAR   wszValue[MAX_STRING];

    //::swprintf_s(wszValue, MAX_STRING, L"Value = %d", osValue);
    ::MessageBox(NULL, wszValue, L"IWMPCore->openState", MB_OK);

    return 0;
}

LRESULT CWMPHost::OnWMPCorePlayState(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    HRESULT      hr;
    WMPPlayState psValue;

    hr = m_spWMPPlayer->get_playState(&psValue);
    if (FAILMSG(hr))
        return 0;

    WCHAR   wszValue[MAX_STRING];

    //::swprintf_s(wszValue, MAX_STRING, L"Value = %d", psValue);
    ::MessageBox(NULL, wszValue, L"IWMPCore->playState", MB_OK);

    return 0;
}

LRESULT CWMPHost::OnWMPCoreVersionInfo(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    CComBSTR    m_bstrValue;
    HRESULT     hr;

    hr = m_spWMPPlayer->get_versionInfo(&m_bstrValue);
    if (FAILMSG(hr))
        return 0;

    WCHAR   wszValue[MAX_STRING];

    //::swprintf_s(wszValue, MAX_STRING, L"Version = %s", m_bstrValue);
    ::MessageBox(NULL, wszValue, L"IWMPCore->versionInfo", MB_OK);

    return 0;
}

LRESULT CWMPHost::OnWMPCoreLaunchURL(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    CComBSTR    m_bstrValue;
    HRESULT     hr;

    CStringDlg dlgString(L"IWMPCore->LaunchURL");

    if (dlgString.DoModal(m_hWnd) == IDOK)
    {
        hr = m_spWMPPlayer->launchURL(dlgString.m_bstrValue);
        if (FAILMSG(hr))
            return 0;
    }
    return 0;
}

LRESULT CWMPHost::OnWMPCoreIsOnline(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    HRESULT      hr;
    VARIANT_BOOL fValue;

    hr = m_spWMPPlayer->get_isOnline(&fValue);
    if (FAILMSG(hr))
        return 0;

    WCHAR   wszValue[MAX_STRING];

    //::swprintf_s(wszValue, MAX_STRING, L"Value = %s", fValue ? L"TRUE" : L"FALSE");
    ::MessageBox(NULL, wszValue, L"IWMPCore->isOnline", MB_OK);

    return 0;
}

LRESULT CWMPHost::OnWMPCoreStatus(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    CComBSTR    m_bstrValue;
    HRESULT     hr;

    hr = m_spWMPPlayer->get_status(&m_bstrValue);
    if (FAILMSG(hr))
        return 0;

    WCHAR   wszValue[MAX_STRING];

    //::swprintf_s(wszValue, MAX_STRING, L"Status = %s", m_bstrValue);
    ::MessageBox(NULL, wszValue, L"IWMPCore->status", MB_OK);

    return 0;
}

LRESULT CWMPHost::OnWMPCoreInterface(WORD /* wNotifyCode */, WORD wID, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    HRESULT     hr;
    WCHAR       wszName[MAX_STRING];
    void        *pUnknown;

    hr = E_FAIL;
    wszName[0] = L'\0';
    pUnknown = NULL;

    switch (wID)
    {
    case ID_WMPCORE_CONTROLS:
        {
            CComPtr<IWMPControls> spWMPControls;

            hr = m_spWMPPlayer->get_controls(&spWMPControls);
            if (spWMPControls)
            {
                spWMPControls->QueryInterface(__uuidof(IWMPControls), &pUnknown);
                wcscpy_s(wszName, MAX_STRING, L"IWMPCore->controls");
            }
        }
        break;
    case ID_WMPCORE_SETTINGS:
        {
            CComPtr<IWMPSettings> spWMPSettings;

            hr = m_spWMPPlayer->get_settings(&spWMPSettings);
            if (spWMPSettings)
            {
                spWMPSettings->QueryInterface(__uuidof(IWMPSettings), &pUnknown);
                wcscpy_s(wszName, MAX_STRING, L"IWMPCore->settings");
            }
        }
        break;
    case ID_WMPCORE_CURRENTMEDIA:
        {
            CComPtr<IWMPMedia> spWMPMedia;

            hr = m_spWMPPlayer->get_currentMedia(&spWMPMedia);
            if (spWMPMedia)
            {
                spWMPMedia->QueryInterface(__uuidof(IWMPMedia), &pUnknown);
                wcscpy_s(wszName, MAX_STRING, L"IWMPCore->currentMedia");
            }
        }
        break;
    case ID_WMPCORE_MEDIACOLLECTION:
        {
            CComPtr<IWMPMediaCollection> spWMPMediaCollection;

            hr = m_spWMPPlayer->get_mediaCollection(&spWMPMediaCollection);
            if (spWMPMediaCollection)
            {
                spWMPMediaCollection->QueryInterface(__uuidof(IWMPMediaCollection), &pUnknown);
                wcscpy_s(wszName, MAX_STRING, L"IWMPCore->currentMediaCollection");
            }
        }
        break;
    case ID_WMPCORE_PLAYLISTCOLLECTION:
        {
            CComPtr<IWMPPlaylistCollection> spWMPPlaylistCollection;

            hr = m_spWMPPlayer->get_playlistCollection(&spWMPPlaylistCollection);
            if (spWMPPlaylistCollection)
            {
                spWMPPlaylistCollection->QueryInterface(__uuidof(IWMPPlaylistCollection), &pUnknown);
                wcscpy_s(wszName, MAX_STRING, L"IWMPCore->playlistCollection");
            }
        }
        break;
    case ID_WMPCORE_NETWORK:
        {
            CComPtr<IWMPNetwork> spWMPNetwork;

            hr = m_spWMPPlayer->get_network(&spWMPNetwork);
            if (spWMPNetwork)
            {
                spWMPNetwork->QueryInterface(__uuidof(IWMPNetwork), &pUnknown);
                wcscpy_s(wszName, MAX_STRING, L"IWMPCore->network");
            }
        }
        break;
    case ID_WMPCORE_CURRENTPLAYLIST:
        {
            CComPtr<IWMPPlaylist> spWMPPlaylist;

            hr = m_spWMPPlayer->get_currentPlaylist(&spWMPPlaylist);
            if (spWMPPlaylist)
            {
                spWMPPlaylist->QueryInterface(__uuidof(IWMPPlaylist), &pUnknown);
                wcscpy_s(wszName, MAX_STRING, L"IWMPCore->currentPlaylist");
            }
        }
        break;
    case ID_WMPCORE_CDROMCOLLECTION:
        {
            CComPtr<IWMPCdromCollection> spWMPCDRomCollection;

            hr = m_spWMPPlayer->get_cdromCollection(&spWMPCDRomCollection);
            if (spWMPCDRomCollection)
            {
                spWMPCDRomCollection->QueryInterface(__uuidof(IWMPCdromCollection), &pUnknown);
                wcscpy_s(wszName, MAX_STRING, L"IWMPCore->cdromCollection");
            }
        }
        break;
    case ID_WMPCORE_CLOSEDCAPTION:
        {
            CComPtr<IWMPClosedCaption> spWMPClosedCaption;

            hr = m_spWMPPlayer->get_closedCaption(&spWMPClosedCaption);
            if (spWMPClosedCaption)
            {
                spWMPClosedCaption->QueryInterface(__uuidof(IWMPClosedCaption), &pUnknown);
                wcscpy_s(wszName, MAX_STRING, L"IWMPCore->closedCaption");
            }
        }
        break;
    case ID_WMPCORE_ERROR:
        {
            CComPtr<IWMPError> spWMPError;

            hr = m_spWMPPlayer->get_error(&spWMPError);
            if (spWMPError)
            {
                spWMPError->QueryInterface(__uuidof(IWMPError), &pUnknown);
                wcscpy_s(wszName, MAX_STRING, L"IWMPCore->error");
            }
        }
        break;
    case ID_WMPCORE2_DVD:
        {
            CComPtr<IWMPPlayer3> spWMPPlayer3;

            hr = m_spWMPPlayer.QueryInterface(&spWMPPlayer3);
            if (FAILMSG(hr))
                return 0;

            CComPtr<IWMPDVD> spWMPDVD;

            hr = spWMPPlayer3->get_dvd(&spWMPDVD);
            if (spWMPDVD)
            {
                spWMPDVD->QueryInterface(__uuidof(IWMPDVD), &pUnknown);
                wcscpy_s(wszName, MAX_STRING, L"IWMPCore2->dvd");
            }
        }
        break;
    }

    if (FAILMSG(hr))
        return 0;

    if (!pUnknown)
    {
        FAILMSG(E_NOINTERFACE);
        return 0;
    }

    ((IUnknown *)pUnknown)->Release();

    ::MessageBox(NULL, L"Got the expected interface", wszName, MB_OK);

    return 0;
}

LRESULT CWMPHost::OnWMPPlayerEnabled(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    HRESULT      hr;
    VARIANT_BOOL fValue;

    hr = m_spWMPPlayer->get_enabled(&fValue);
    if (FAILMSG(hr))
        return 0;

    CBooleanDlg dlgBoolean(L"IWMPPlayer->enabled", fValue);

    if (dlgBoolean.DoModal(m_hWnd) == IDOK)
    {
        hr = m_spWMPPlayer->put_enabled(dlgBoolean.m_fValue);
        if (FAILMSG(hr))
            return 0;
    }

    return 0;
}

LRESULT CWMPHost::OnWMPPlayerFullScreen(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    HRESULT      hr;
    VARIANT_BOOL fValue;

    hr = m_spWMPPlayer->get_fullScreen(&fValue);
    if (FAILMSG(hr))
        return 0;

    CBooleanDlg dlgBoolean(L"IWMPPlayer->fullScreen", fValue);

    if (dlgBoolean.DoModal(m_hWnd) == IDOK)
    {
        hr = m_spWMPPlayer->put_fullScreen(dlgBoolean.m_fValue);
        if (FAILMSG(hr))
            return 0;
    }

    return 0;
}

LRESULT CWMPHost::OnWMPPlayerEnableContextMenu(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    HRESULT      hr;
    VARIANT_BOOL fValue;

    hr = m_spWMPPlayer->get_enableContextMenu(&fValue);
    if (FAILMSG(hr))
        return 0;

    CBooleanDlg dlgBoolean(L"IWMPPlayer->enableContextMenu", fValue);

    if (dlgBoolean.DoModal(m_hWnd) == IDOK)
    {
        hr = m_spWMPPlayer->put_enableContextMenu(dlgBoolean.m_fValue);
        if (FAILMSG(hr))
            return 0;
    }

    return 0;
}

LRESULT CWMPHost::OnWMPPlayerUIMode(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    CComBSTR    m_bstrValue;
    HRESULT     hr;

    hr = m_spWMPPlayer->get_uiMode(&m_bstrValue);
    if (FAILMSG(hr))
        return 0;

    CStringDlg dlgString(L"IWMPCore->uiMode", m_bstrValue);

    if (dlgString.DoModal(m_hWnd) == IDOK)
    {
        hr = m_spWMPPlayer->put_uiMode(dlgString.m_bstrValue);
        if (FAILMSG(hr))
            return 0;
    }
    return 0;
}

LRESULT CWMPHost::OnWMPPlayer2StretchToFit(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    HRESULT      hr;
    VARIANT_BOOL fValue;
    CComPtr<IWMPPlayer2> spWMPPlayer2;

    hr = m_spWMPPlayer.QueryInterface(&spWMPPlayer2);
    if (FAILMSG(hr))
        return 0;

    hr = spWMPPlayer2->get_stretchToFit(&fValue);
    if (FAILMSG(hr))
        return 0;

    CBooleanDlg dlgBoolean(L"IWMPPlayer2->stretchToFit", fValue);

    if (dlgBoolean.DoModal(m_hWnd) == IDOK)
    {
        hr = spWMPPlayer2->put_stretchToFit(dlgBoolean.m_fValue);
        if (FAILMSG(hr))
            return 0;
    }

    return 0;
}

LRESULT CWMPHost::FowardMsgToWMP(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled)
{
    HRESULT hr;
    LRESULT llResult = 0;
    CComPtr<IOleInPlaceObjectWindowless> spSite = NULL;

    hr = m_spWMPPlayer->QueryInterface(&spSite);
    if( SUCCEEDED(hr) )
    {
        spSite->OnWindowMessage(uMsg, wParam, lParam, &llResult);
    }
    bHandled = TRUE;

    return llResult;
}

LRESULT CWMPHost::OnWMPSelectFolder(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    CFolderPickerDialog dlgOpen;
    //HRESULT      hr;

    if (dlgOpen.DoModal(m_hWnd) == IDOK)
    {
        /*hr = m_spWMPPlayer->put_URL(dlgOpen.m_bstrName);
        if (FAILMSG(hr))
            return 0;*/
        selected_folder_macro = dlgOpen.m_bstrName;
        //OutputDebugString(dlgOpen.m_bstrName);
    }
    return 0;
}
LRESULT CWMPHost::OnTestShell(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */) {
    //ShellExecute(NULL, NULL, L"cmd", _T("/c mkdir  \"C:\\TestFolder\""), NULL, SW_SHOWNORMAL);
    //ShellExecute(NULL, _T("open"), _T("cmd.exe"), _T("\"%CD%\\Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\testNewFldr.bat\""), NULL, SW_SHOWNORMAL);
    //ShellExecute(NULL, NULL, L"cmd", _T("/c mkdir \"%CD%\\Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\testNewFldr.bat\""), NULL, SW_SHOWNORMAL);
    // this one works ShellExecute(NULL, _T("open"), _T("cmd.exe"), _T("/k \"%CD%\\Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\test.bat\""), NULL, SW_SHOW);
    //BSTR test = _T("/k \"%CD%\\Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\testNew.bat");
    //ShellExecute(NULL, _T("open"), _T("cmd.exe"), _T("/k \"%CD%\\Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\testNew.bat\""), NULL, SW_SHOW);
    //BSTR cmd = L"/k \"%CD%\\Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\testNew.bat testDir\"";

    auto a = SysAllocString(L"/k %CD%\\Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\test.bat ");
    auto b = SysAllocString(SELECTED_VIDEOFILENAME_MACRO);
    auto c = SysAllocString(L" ");
    auto d = SysAllocString(SELECTED_SLOWDOWN_MACRO);

    auto first_result = Concat(a, b);
    auto second_result = Concat(first_result, c);
    auto final_result = Concat(second_result, d);

    //OutputDebugString(final_result);
    ShellExecute(NULL, _T("open"), _T("cmd.exe"),final_result, NULL, SW_SHOW);

    SysFreeString(a);
    SysFreeString(b);
    SysFreeString(c);
    return 0;
}
LRESULT CWMPHost::fourxOption(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */) {
    SELECTED_SLOWDOWN_MACRO = L"4";
    return 0;
}
LRESULT CWMPHost::eightxOption(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */) {
    SELECTED_SLOWDOWN_MACRO = L"8";
    return 0;
}
LRESULT CWMPHost::sixteenxOption(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */) {
    SELECTED_SLOWDOWN_MACRO = L"16";
    return 0;
}
BSTR CWMPHost::Concat(BSTR a, BSTR b)
{
    auto lengthA = SysStringLen(a);
    auto lengthB = SysStringLen(b);

    auto result = SysAllocStringLen(NULL, lengthA + lengthB);

    memcpy(result, a, lengthA * sizeof(OLECHAR));
    memcpy(result + lengthA, b, lengthB * sizeof(OLECHAR));

    result[lengthA + lengthB] = 0;
    return result;
}
BSTR CWMPHost::extract_filename(BSTR path)
{
    std::wstring wstr(path);
    size_t finalBackSlash = 0;
    size_t len = wstr.length();
    for (size_t i = 0; i < len; i++) {
        if (wstr[i] == L'\\') {
            finalBackSlash = i;
        }
    }
    std::wstring retString = wstr.substr(finalBackSlash + 1);
    BSTR bstr = SysAllocString(retString.c_str());

    return bstr;
}