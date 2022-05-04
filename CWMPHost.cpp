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

#pragma region Includes and Manifest Dependencies
#include <stdio.h>
#include <windows.h>
#include <windowsx.h>
#include "Resource.h"
#include <assert.h>

#include <commctrl.h>
#pragma comment(lib, "comctl32.lib")

// Enable Visual Style
#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='x86' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='ia64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='amd64' publicKeyToken='6595b64144ccf1df' language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32' name='Microsoft.Windows.Common-Controls' version='6.0.0.0' processorArchitecture='*' publicKeyToken='6595b64144ccf1df' language='*'\"")
#endif
#pragma endregion

BSTR selected_folder_macro;
BSTR SELECTED_VIDEO_MACRO;
BSTR SELECTED_VIDEOFILENAME_MACRO;
BSTR SELECTED_SLOWDOWN_MACRO;
/////////////////////////////////////////////////////////////////////////////
LRESULT CALLBACK PopupProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK UpdownDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
BSTR Concat(BSTR a, BSTR b);

void CWMPHost::OnFinalMessage(HWND /*hWnd*/)
{
    ::PostQuitMessage(0);
}

LRESULT CWMPHost::OnCreate(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */)
{

    AtlAxWinInit();
    CComPtr<IAxWinHostWindow>           spHost;
    CComPtr<IConnectionPointContainer>  spConnectionContainer;
    CComWMPEventDispatch* pEventListener = NULL;
    CComPtr<IWMPEvents>                 spEventListener;
    HRESULT                             hr;
    RECT                                rcClient;
    // DWORD dwExtStyle = 0;
     //DWORD dwStyle = WS_POPUPWINDOW;
    m_dwAdviseCookie = 0;
    GetClientRect(&rcClient);

    LONG rcWidth = rcClient.right - rcClient.left;
    LONG rcHeight = rcClient.bottom - rcClient.top;
    // create window
    LONG leftOffset = (LONG)(0.04 * rcWidth);
    LONG rightOffset = -(LONG)(0.4444 * rcWidth);
    LONG topOffset = (LONG)(0.04 * rcHeight);
    LONG bottomOffset = -(LONG)(0.4444 * (rcHeight));

    LONG modalLeft = rcClient.left + leftOffset;
    LONG modalRight = rcClient.right + rightOffset;
    LONG modalTop = rcClient.top + topOffset;
    LONG modalBottom = rcClient.bottom + bottomOffset;
    RECT rect = { modalLeft, modalTop, modalRight, modalBottom };
    open_modal = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Process Video",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        modalLeft + 10,         // x position 
        modalTop + 400,         // y position 
        100,        // Button width
        100,        // Button height
        m_hWnd,     // Parent window
        NULL,       // No menu.
        (HINSTANCE)(::GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE)),
        NULL);
    popup.hbrBackground = (HBRUSH)COLOR_WINDOW;
    popup.hCursor = LoadCursor(NULL, IDC_ARROW);
    popup.hInstance = (HINSTANCE)(::GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE));
    popup.lpszClassName = L"popupClass";
    popup.lpfnWndProc = PopupProc;

    RegisterClassW(&popup);



    //AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, true);
    //RECT myrcClient = RECT{0,0,100,100};
    m_wndView.Create(m_hWnd, rect, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
    if (NULL == m_wndView.m_hWnd)
        goto FAILURE;


    /*int width = 380;
    int height = 380;
    ::SetWindowPos(m_wndView, 0, 100, 100, width, height, SWP_NOMOVE | SWP_NOOWNERZORDER | SWP_NOZORDER);*/

    // load OCX in window
    //hr = m_wndView.ResizeClient(50,50,FALSE);
   // if (FAILMSG(hr))
    //    goto FAILURE;

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
//LRESULT CWMPHost::OnPaint(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */)
 /* {
    PAINTSTRUCT ps;
    HDC hDC = m_wndView.BeginPaint(&ps);
    //Use the hDC as much as you want
    ::Rectangle(hDC, 0, 0, 150, 150);

    m_wndView.EndPaint(&ps);
    return 0;
}*/
LRESULT CWMPHost::OnDestroy(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& bHandled)
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
    RECT size_rcClient;
    GetClientRect(&size_rcClient);

    LONG size_rcWidth = size_rcClient.right - size_rcClient.left;
    LONG size_rcHeight = size_rcClient.bottom - size_rcClient.top;

    LONG size_modalLeft = size_rcClient.left + (LONG)(0.04 * (size_rcWidth));
    LONG size_modalRight = size_rcClient.right - (LONG)(0.4444 * (size_rcWidth));
    LONG size_modalTop = size_rcClient.top + (LONG)(0.04 * (size_rcHeight));
    LONG size_modalBottom = size_rcClient.bottom - (LONG)(0.4444 * (size_rcHeight));

    m_wndView.MoveWindow(size_modalLeft, size_modalTop, size_modalRight, size_modalBottom, SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);

    HDC screenDC = GetDC(); //NULL gets whole screen

    HBRUSH brush = CreateSolidBrush(RGB(255, 255, 255)); //create brush
    SelectObject(screenDC, brush); //select brush into DC
    Rectangle(screenDC, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); //draw rectangle over whole screen



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

        //OutputDebugString(target);
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
        hr = m_spWMPPlayer->put_URL(dlgOpen.m_bstrName);

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
    void* pUnknown;

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

    ((IUnknown*)pUnknown)->Release();

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

LRESULT CWMPHost::OnWMPSelectFolder(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */)
{
    CFolderPickerDialog dlgOpen;
    //HRESULT      hr;

    if (dlgOpen.DoModal(m_hWnd) == IDOK)
    {
        /*hr = m_spWMPPlayer->put_URL(dlgOpen.m_bstrName);
        if (FAILMSG(hr))
            return 0;*/
        selected_folder_macro = SysAllocString(dlgOpen.m_bstrName);
        OutputDebugString(L"\n\n");

        OutputDebugString(selected_folder_macro);
        OutputDebugString(L"\n\n");
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
    CString solutionCString = MY_SOLUTIONDIR;
    auto restOfFile = SysAllocString(L"Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\test.bat");

    auto solutionBSTR = solutionCString.AllocSysString();
    auto cmd = Concat(solutionBSTR, restOfFile);
    //auto a = SysAllocString(L"/k %CD%\\Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\test.bat ");
    auto videoArg = SysAllocString(SELECTED_VIDEOFILENAME_MACRO);
    auto space = SysAllocString(L" ");
    auto slowdownArg = SysAllocString(SELECTED_SLOWDOWN_MACRO);

    auto first_result = Concat(cmd, videoArg);
    auto second_result = Concat(first_result, space);
    auto final_result = Concat(second_result, slowdownArg);
    // OutputDebugString(final_result);

     //OutputDebugString(final_result);
    ShellExecute(NULL, _T("open"), _T("cmd.exe"), final_result, NULL, SW_SHOW);

    SysFreeString(cmd);
    SysFreeString(solutionBSTR);
    SysFreeString(videoArg);
    SysFreeString(space);
    SysFreeString(slowdownArg);

    return 0;
}



LRESULT CWMPHost::twoxOption(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */) {
    SELECTED_SLOWDOWN_MACRO = L"2";
    return 0;
}
LRESULT CWMPHost::threexOption(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */) {
    SELECTED_SLOWDOWN_MACRO = L"3";
    return 0;
}
LRESULT CWMPHost::fourxOption(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */) {
    SELECTED_SLOWDOWN_MACRO = L"4";

    return 0;

}

LRESULT CWMPHost::PlaySlomo(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */) {
    CString solutionCString = MY_SOLUTIONDIR;
    HRESULT      hr;

    auto solution_dir_to_video_folder = SysAllocString(L"Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\Outputs\\");
    auto solutionBSTR = solutionCString.AllocSysString();
    auto full_video_folder_path = Concat(solutionBSTR, solution_dir_to_video_folder);
    auto slowdown = SysAllocString(SELECTED_SLOWDOWN_MACRO);
    auto videoarg = SysAllocString(L"Output_");
    auto x_letter = SysAllocString(L"X");
    auto extension = SysAllocString(L".mkv");
    auto penultimate_concat = Concat(videoarg, slowdown);
    auto final_concat_insertion = Concat(penultimate_concat, x_letter);
    auto final_video = Concat(final_concat_insertion, extension);

    /*
    std::wstring wstr(video_arg);
    size_t video_type_idx = wstr.rfind(SysAllocString(L"."));

    wstr.insert(video_type_idx, final_concat_insertion);
    BSTR video_name = SysAllocString(wstr.c_str());
    */

    auto final_concatination = Concat(full_video_folder_path, final_video);
    //OutputDebugString(L"\n\n\n");
    //OutputDebugString(final_concatination);
    //OutputDebugString(L"\n\n\n");

    hr = m_spWMPPlayer->put_URL(final_concatination);

    if (FAILMSG(hr))
        return 0;
    return 0;

}
LRESULT CALLBACK PopupProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_CLOSE:
        DestroyWindow(hwnd);
        break;
    default:
        return DefWindowProcW(hwnd, msg, wParam, lParam);
    }
    return 0;
}

LRESULT CWMPHost::displayPopup(HWND hWnd) {
    try {
        popUp_hWnd = CreateWindow(L"popupClass", L"Proceess initalizer", WS_VISIBLE | WS_OVERLAPPEDWINDOW, 400, 400, 400, 400, hWnd, NULL, NULL, NULL);
        open_modal = CreateWindow(
            L"BUTTON",  // Predefined class; Unicode assumed 
            L"SLow Down Video",      // Button text 
            WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
            10,         // x position 
            10,         // y position 
            100,        // Button width
            100,        // Button height
            popUp_hWnd,     // Parent window
            NULL,       // No menu.
            (HINSTANCE)(::GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE)),
            NULL);
    }
    catch (int bad) {
        return bad;
    }
    return 0;
}

LRESULT CWMPHost::OpenModal(UINT  uMsg, WPARAM  wParam, LPARAM  lParam, BOOL& bHandled)
{
    if ((HWND)lParam == open_modal)
    {
        HINSTANCE g_inst = (HINSTANCE)(::GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE));
        hDlg = CreateDialog(g_inst,
            MAKEINTRESOURCE(IDD_UPDOWNDIALOG),
            m_hWnd, UpdownDlgProc);
        if (hDlg)
        {
            ::ShowWindow(hDlg, SW_SHOW);
        }




    }

    bHandled = True;

    wParam = NULL;
    uMsg = NULL;


    bHandled = TRUE;

    return 0;
}

BSTR Concat(BSTR a, BSTR b)
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
LRESULT CWMPHost::TrimVideo(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */) {
    CComBSTR    m_bstrValue;
    CStringDlg dlgString(L"Trim Video", m_bstrValue);
    CComBSTR   start, end;
    if (dlgString.DoModal(m_hWnd) == IDOK)
    {
        start = dlgString.m_bstrValue1;
        end = dlgString.m_bstrValue2;
    }

    CString solutionCString = MY_SOLUTIONDIR;
    OutputDebugString(selected_folder_macro);

    //ffmpeg -ss 00:01:00 -to 00:02:00 -i input.mp4 -c copy output.mp4
    auto time_start = Concat(SysAllocString(L"ffmpeg -ss "), SysAllocString(start));
    auto second_command = Concat(time_start, SysAllocString(L" -to "));
    auto time_end = Concat(second_command, SysAllocString(end));
    auto third_command = Concat(time_end, SysAllocString(L" -i "));
    auto input_file_name = Concat(third_command, SysAllocString(SELECTED_VIDEO_MACRO));
    auto fourth_command = Concat(input_file_name, SysAllocString(L" -c copy "));
    auto output_file_folder = Concat(fourth_command, SysAllocString(selected_folder_macro));
    auto output_file_folder_with_slash = Concat(output_file_folder, SysAllocString(L"\\"));
    std::wstring wstr(SELECTED_VIDEOFILENAME_MACRO);
    size_t period = 0;
    size_t len = wstr.length();
    for (size_t i = 0; i < len; i++) {
        if (wstr[i] == L'.') {
            period = i;
        }
    }
    std::wstring retString = wstr.substr(0, period);
    std::wstring extensionwstr = wstr.substr(period);


    BSTR file_wo_mp4 = SysAllocString(retString.c_str());
    BSTR extension = SysAllocString(extensionwstr.c_str());
    OutputDebugString(L"\n\n");
    OutputDebugString(file_wo_mp4);
    OutputDebugString(L"\n\n");
    OutputDebugString(L"\n\n");
    OutputDebugString(extension);
    OutputDebugString(L"\n\n");


    auto output_file_name = Concat(output_file_folder_with_slash, SysAllocString(file_wo_mp4));

    auto penultimate = Concat(output_file_name, SysAllocString(L"_trimmed"));
    auto final_result = Concat(penultimate, SysAllocString(extension));
    OutputDebugString(L"\n\n");
    OutputDebugString(L"\n\n");
    OutputDebugString(L"\n\n");

    OutputDebugString(output_file_name);
    OutputDebugString(L"\n\n");
    OutputDebugString(L"\n\n");
    OutputDebugString(L"\n\n");
    OutputDebugString(L"\n\n");

    OutputDebugString(final_result);
    OutputDebugString(L"\n\n");
    ShellExecute(NULL, _T("open"), _T("cmd.exe"), final_result, NULL, SW_SHOW);
    return 0;
}
BOOL CWMPHost::OnInitDialog(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */)
{
    return TRUE;
}
LRESULT OnClose(HWND hWnd)
{
    EndDialog(hWnd, 0);
    return 0;
}

#pragma region Updown
// MSDN: Up-Down Control
// http://msdn.microsoft.com/en-us/library/bb759880.aspx

#define IDC_EDIT		2990
#define IDC_UPDOWN		2991
#define IDC_OK          2992

//
//   FUNCTION: OnInitUpdownDialog(HWND, HWND, LPARAM)
//
//   PURPOSE: Process the WM_INITDIALOG message
//
HWND hUpdown, hEdit;
BOOL OnInitUpdownDialog(HWND hWnd, HWND hWndFocus, LPARAM lParam)
{
    HINSTANCE g_hInst = (HINSTANCE)(::GetWindowLongPtr(hWnd, GWLP_HINSTANCE));
    lParam = NULL;
    hWndFocus = NULL;
    // Load and register Updown control class
    INITCOMMONCONTROLSEX iccx;
    iccx.dwSize = sizeof(INITCOMMONCONTROLSEX);
    iccx.dwICC = ICC_UPDOWN_CLASS;
    if (!InitCommonControlsEx(&iccx))
        return FALSE;

    // Create an Edit control
    RECT rc = { 40, 120, 200, 48 };
    hEdit = CreateWindowEx(WS_EX_CLIENTEDGE, L"EDIT", 0,
        WS_CHILD | WS_VISIBLE, rc.left, rc.top, rc.right, rc.bottom,
        hWnd, (HMENU)IDC_EDIT, g_hInst, 0);

    // Create the ComboBoxEx control
    SetRect(&rc, 40, 160, 360, 40);
    hUpdown = CreateWindowEx(0, UPDOWN_CLASS, 0,
        UDS_ALIGNRIGHT | UDS_SETBUDDYINT | UDS_WRAP | WS_CHILD | WS_VISIBLE,
        rc.left, rc.top, rc.right, rc.bottom,
        hWnd, (HMENU)IDC_UPDOWN, g_hInst, 0);
    HFONT hFont = CreateFont(40, 0, 0, 0, FW_DONTCARE, FALSE, FALSE, FALSE, ANSI_CHARSET, OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, DEFAULT_QUALITY, DEFAULT_PITCH | FF_SWISS, L"Arial");
    SendMessage(hEdit, WM_SETFONT, WPARAM(hFont), TRUE);


    // Explicitly attach the Updown control to its 'buddy' edit control
    SendMessage(hUpdown, UDM_SETBUDDY, (WPARAM)hEdit, 0);


    return TRUE;
}

//
//  FUNCTION: UpdownDlgProc(HWND, UINT, WPARAM, LPARAM)
//
//  PURPOSE:  Processes messages for the Updown control dialog.
//
//
LRESULT OnOk(HWND hWnd, int /*id*/, HWND /*hWndCtl*/, UINT codeNotify) {
    if (codeNotify == 0) {
        TCHAR* TBuf = 0;
        int BufSize = 0;
        BufSize = Edit_GetTextLength(hEdit) + 1;
        TBuf = new TCHAR[BufSize];

        Edit_GetText(hEdit, TBuf, BufSize);
        OutputDebugString(L"Value:");
        //_bstr_t bstrResult = _bstr_t(TBuf);
        OutputDebugString(TBuf);

        CString solutionCString = MY_SOLUTIONDIR;
        auto restOfFile = SysAllocString(L"Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\test.bat");

        BSTR solutionBSTR = solutionCString.AllocSysString();
        BSTR cmd = Concat(solutionBSTR, restOfFile);
        //auto a = SysAllocString(L"/k %CD%\\Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\test.bat ");
        BSTR videoArg = SysAllocString(SELECTED_VIDEOFILENAME_MACRO);
        BSTR space = SysAllocString(L" ");
        BSTR slowdownArg = SysAllocString(TBuf);

        auto first_result = Concat(cmd, videoArg);
        auto second_result = Concat(first_result, space);
        auto final_result = Concat(second_result, slowdownArg);
        // OutputDebugString(final_result);

         //OutputDebugString(final_result);
        ShellExecute(NULL, _T("open"), _T("cmd.exe"), final_result, NULL, SW_SHOW);

        SysFreeString(cmd);
        SysFreeString(solutionBSTR);
        SysFreeString(videoArg);
        SysFreeString(space);
        SysFreeString(slowdownArg);
        EndDialog(hWnd, 0);

        return 0;

    }
    return 0;
}

INT_PTR CALLBACK UpdownDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
        // Handle the WM_INITDIALOG message in OnInitUpdownDialog
        HANDLE_MSG(hWnd, WM_INITDIALOG, OnInitUpdownDialog);

        // Handle the WM_CLOSE message in OnClose
        HANDLE_MSG(hWnd, WM_CLOSE, OnClose);

        HANDLE_MSG(hWnd, WM_COMMAND, OnOk);

    default:
        return FALSE;	// Let system deal with msg
    }

}

#pragma endregion
