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
#include <io.h>

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
#include <Uxtheme.h>
#include <WindowsX.h>
#include <Vssym32.h>
#include <commctrl.h>
#include <uxtheme.h>
#include <iostream>
#include <cstdio>
#include <fstream>
#include <dwmapi.h>
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Uxtheme.lib")
#pragma comment(lib, "Dwmapi.lib")


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
BSTR SELECTED_VIDEO_MACRO; //name and path
BSTR SELECTED_VIDEO_MACRO2;
BSTR SELECTED_VIDEOFILENAME_MACRO;//just the name
BSTR SELECTED_VIDEOFILENAME_MACRO2;
BSTR SELECTED_SLOWDOWN_MACRO;
std::vector<const wchar_t*> console_output;
int LINES = 0;

static HFONT s_hFont = NULL;

/////////////////////////////// Forward Declarations //////////////////////////////////////////////
LRESULT CALLBACK PopupProc(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
INT_PTR CALLBACK UpdownDlgProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK ConsoleProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
LRESULT CALLBACK titleTwo(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam);
BSTR Concat(BSTR a, BSTR b);
 //////////////////////////////////////////////////////////////////////////////////////////////////////
void CWMPHost::OnFinalMessage(HWND /*hWnd*/)
{
    ::PostQuitMessage(0);
}

LRESULT CWMPHost::OnCreate(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */)
{

    AtlAxWinInit();

    CComPtr<IAxWinHostWindow>           spHost;
    CComPtr<IAxWinHostWindow>           spHost2;
    CComPtr<IConnectionPointContainer>  spConnectionContainer;
    CComPtr<IConnectionPointContainer>  spConnectionContainer2;
    CComWMPEventDispatch* pEventListener = NULL;
    CComWMPEventDispatch* pEventListener2 = NULL;

    CComPtr<IWMPEvents>                 spEventListener;
    CComPtr<IWMPEvents>                 spEventListener2;

    HRESULT                             hr;
    RECT                                rcClient;
    // DWORD dwExtStyle = 0;
     //DWORD dwStyle = WS_POPUPWINDOW;
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
    m_dwAdviseCookie = 0;
    GetClientRect(&rcClient);
    WNDCLASS wc{};
    wc.hInstance = (HINSTANCE)(::GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE));
    wc.lpszClassName = L"scroll_window";
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = CreateSolidBrush(RGB(46, 42, 45));
    wc.lpfnWndProc = ConsoleProc;
    RegisterClass(&wc);
    console_display = CreateWindow(L"scroll_window", L"", WS_OVERLAPPED | WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
        200, 200, (LONG)(.4 * (rcClient.right - rcClient.left)), (LONG)(.4 * (rcClient.bottom - rcClient.top)), // x, y, w, h
        m_hWnd,
        NULL,
        NULL,
        NULL);
    for (int i = 0; i < 30; i++) {
        console_output.push_back(L"Hello");
    }

   console_output.push_back(L"Select a video to slowdown");
   LINES = (int)(console_output.size());
   SendMessage(console_display, WM_PAINT, NULL, NULL);
   SendMessage(console_display, WM_SIZE, NULL, NULL);



   WNDCLASS wc_title1{};
   wc_title1.hInstance = (HINSTANCE)(::GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE));
   wc_title1.lpszClassName = L"title_1";
   wc_title1.hCursor = LoadCursor(nullptr, IDC_ARROW);
   wc_title1.hbrBackground = (HBRUSH)COLOR_WINDOW;
   wc_title1.lpfnWndProc = titleTwo;
   RegisterClass(&wc_title1);
   video_player1_title = CreateWindow(L"title_1", L"", WS_CHILD | WS_VISIBLE | WS_BORDER | ES_AUTOHSCROLL,
       (LONG)((.7) * rcClient.right - rcClient.left), (LONG)((.6) * rcClient.bottom - rcClient.top), (LONG)(.05 * (rcClient.right - rcClient.left)), (LONG)(.05 * (rcClient.bottom - rcClient.top)), // x, y, w, h
       m_hWnd,
       NULL,
       NULL,
       NULL);
   
   video_player2_title = CreateWindow(L"title_1", L"", WS_CHILD | WS_VISIBLE| WS_BORDER | ES_AUTOHSCROLL,
       (LONG)((.1)*rcClient.right - rcClient.left), (LONG)((.6) * rcClient.bottom - rcClient.top), (LONG)(.05 * (rcClient.right - rcClient.left)), (LONG)(.05 * (rcClient.bottom - rcClient.top)), // x, y, w, h
       m_hWnd,
       NULL,
       NULL,
       NULL);

    LONG rcWidth = rcClient.right - rcClient.left;
    LONG rcHeight = rcClient.bottom - rcClient.top;
    // create window
    LONG leftOffset = (LONG)(0.05 * rcWidth);
    LONG rightOffset = -(LONG)(0.55 * rcWidth);
    LONG topOffset = (LONG)(0.05 * rcHeight);
    LONG bottomOffset = -(LONG)(0.55 * (rcHeight));

    LONG modalLeft = rcClient.left + leftOffset;
    LONG modalRight = rcClient.right + rightOffset;
    LONG modalTop = rcClient.top + topOffset;
    LONG modalBottom = rcClient.bottom + bottomOffset;

    LONG leftOffset2 = (LONG)(0.60 * rcWidth);
    LONG rightOffset2 = -(LONG)(0.05 * rcWidth);
    LONG topOffset2 = (LONG)(0.05 * rcHeight);
    LONG bottomOffset2 = -(LONG)(0.55 * (rcHeight));

    LONG modalLeft2 = rcClient.left + leftOffset2;
    LONG modalRight2 = rcClient.right + rightOffset2;
    LONG modalTop2 = rcClient.top + topOffset2;
    LONG modalBottom2 = rcClient.bottom + bottomOffset2;

    RECT rect = { modalLeft, modalTop, modalRight, modalBottom };
    RECT rect2 = { modalLeft2, modalTop2, modalRight2, modalBottom2 };
    HMENU hMenu = ::GetMenu(m_hWnd);
    HBRUSH menu_brush = CreateSolidBrush(RGB(167, 173, 186)); //create brush
    MENUINFO mi = { 0 };
    mi.cbSize = sizeof(mi);
    mi.fMask = MIM_BACKGROUND ;
    mi.hbrBack = menu_brush;
    if (IsMenu(hMenu))
    {
        SetMenuInfo(hMenu, &mi);
    }
   
    
    trim1 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Trim",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        modalLeft + 10,         // x position 
        modalTop + 400,         // y position 
        120,        // Button width
        120,        // Button height
        m_hWnd,     // Parent window
        NULL,       // No menu.
        (HINSTANCE)(::GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE)),
        NULL);
    trim2 = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Trim",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        modalLeft + 10,         // x position 
        modalTop + 400,         // y position 
        120,        // Button width
        120,        // Button height
        m_hWnd,     // Parent window
        NULL,       // No menu.
        (HINSTANCE)(::GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE)),
        NULL);
   
    open_modal = CreateWindow(
        L"BUTTON",  // Predefined class; Unicode assumed 
        L"Slow Down Video",      // Button text 
        WS_TABSTOP | WS_VISIBLE | WS_CHILD | BS_DEFPUSHBUTTON,  // Styles 
        modalLeft + 10,         // x position 
        modalTop + 400,         // y position 
        120,        // Button width
        120,        // Button height
        m_hWnd,     // Parent window
        NULL,       // No menu.
        (HINSTANCE)(::GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE)),
        NULL);
    const TCHAR* fontName = _T("Croobie");
    const long nFontSize = 10;

    HDC hdc = ::GetDC(open_modal);

    LOGFONT logFont = { 0 };
    logFont.lfHeight = -MulDiv(nFontSize, GetDeviceCaps(hdc, LOGPIXELSY), 72);
    logFont.lfWeight = FW_BOLD;
    _tcscpy_s(logFont.lfFaceName, fontName);

    s_hFont = CreateFontIndirect(&logFont);

    ::ReleaseDC(open_modal, hdc);
    SendMessage(open_modal, WM_SETFONT, (WPARAM)s_hFont, (LPARAM)MAKELONG(TRUE, 0));


    popup.hbrBackground = (HBRUSH)COLOR_WINDOW;
    popup.hCursor = LoadCursor(NULL, IDC_ARROW);
    popup.hInstance = (HINSTANCE)(::GetWindowLongPtr(m_hWnd, GWLP_HINSTANCE));
    popup.lpszClassName = L"popupClass";
    popup.lpfnWndProc = PopupProc;

    RegisterClassW(&popup);



    //AdjustWindowRect(&rect, WS_OVERLAPPEDWINDOW, true);
    //RECT myrcClient = RECT{0,0,100,100};
    m_wndView.Create(m_hWnd, rect, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
    m_wndView2.Create(m_hWnd, rect2, NULL, WS_CHILD | WS_VISIBLE | WS_CLIPCHILDREN, WS_EX_CLIENTEDGE);
    if (NULL == m_wndView.m_hWnd)
        goto FAILURE;
    if (NULL == m_wndView2.m_hWnd)
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
    hr = m_wndView2.QueryHost(&spHost2);
    if (FAILMSG(hr))
        goto FAILURE;

    hr = spHost->CreateControl(CComBSTR(L"{6BF52A52-394A-11d3-B153-00C04F79FAA6}"), m_wndView, 0);
    if (FAILMSG(hr))
        goto FAILURE;
    hr = spHost2->CreateControl(CComBSTR(L"{6BF52A52-394A-11d3-B153-00C04F79FAA6}"), m_wndView2, 0);
    if (FAILMSG(hr))
        goto FAILURE;

    hr = m_wndView.QueryControl(&m_spWMPPlayer);
    if (FAILMSG(hr))
        goto FAILURE;
    hr = m_wndView2.QueryControl(&m_2spWMPPlayer);
    if (FAILMSG(hr))
        goto FAILURE;

    // start listening to events

    hr = CComWMPEventDispatch::CreateInstance(&pEventListener);
    spEventListener = pEventListener;
    if (FAILMSG(hr))
        goto FAILURE;
    hr = CComWMPEventDispatch::CreateInstance(&pEventListener2);
    spEventListener2 = pEventListener2;
    if (FAILMSG(hr))
        goto FAILURE;

    hr = m_spWMPPlayer->QueryInterface(&spConnectionContainer);
    if (FAILMSG(hr))
        goto FAILURE;

    hr = m_2spWMPPlayer->QueryInterface(&spConnectionContainer2);
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

    hr = spConnectionContainer2->FindConnectionPoint(__uuidof(IWMPEvents), &m_spConnectionPoint2);
    if (FAILED(hr))
    {
        // If not, try the _WMPOCXEvents interface, which will use IDispatch
        hr = spConnectionContainer2->FindConnectionPoint(__uuidof(_WMPOCXEvents), &m_spConnectionPoint2);
        if (FAILMSG(hr))
            goto FAILURE;
    }

    hr = m_spConnectionPoint->Advise(spEventListener, &m_dwAdviseCookie);
    if (FAILMSG(hr))
        goto FAILURE;
    hr = m_spConnectionPoint->Advise(spEventListener2, &m_dwAdviseCookie2);
    if (FAILMSG(hr))
        goto FAILURE;

    return 0;

FAILURE:
    ::PostQuitMessage(0);
    return 0;
}
 
LRESULT CWMPHost::OnDestroy(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& bHandled)
{
    // stop listening to events

    if (m_spConnectionPoint)
    {
        if (0 != m_dwAdviseCookie)
            m_spConnectionPoint->Unadvise(m_dwAdviseCookie);
        m_spConnectionPoint.Release();
    }
    if (m_spConnectionPoint2)
    {
        if (0 != m_dwAdviseCookie2)
            m_spConnectionPoint2->Unadvise(m_dwAdviseCookie2);
        m_spConnectionPoint2.Release();
    }
    // close the OCX

    if (m_spWMPPlayer)
    {
        m_spWMPPlayer->close();
        m_spWMPPlayer.Release();
    }
    if (m_2spWMPPlayer)
    {
        m_2spWMPPlayer->close();
        m_2spWMPPlayer.Release();
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

    LONG xPlayer1 = (LONG)(0.0 * size_rcWidth);
    LONG yPlayer1 = (LONG)(0.00 * size_rcHeight);
    LONG widthPlayer1 = (LONG)(size_rcWidth * .48);
    LONG heightPlayer1 = (LONG)(size_rcHeight * .48);

    LONG xPlayer2 = (LONG)(0.52 * size_rcWidth);
    LONG yPlayer2 = (LONG)(0.00 * size_rcHeight);
    LONG widthPlayer2 = (LONG)(size_rcWidth * .48);
    LONG heightPlayer2 = (LONG)(size_rcHeight * .48);

    LONG xModal3 = (LONG)(.44 * size_rcWidth);
    LONG yModal3 = (LONG)(0.65 * size_rcHeight);
    LONG widthModal3 = (LONG)(size_rcWidth * .115);
    LONG heightModal3 = (LONG)(widthModal3/1.5);
    
    LONG xModal4 = (LONG)(0.025 * size_rcWidth);
    LONG yModal4 = (LONG)(.50 * size_rcHeight);
    LONG widthModal4 = (LONG)(size_rcWidth * .05);
    LONG heightModal4 = (LONG)(widthModal4/1.5);

    LONG xModal5 = (LONG)(0.525 * size_rcWidth);
    LONG yModal5 = (LONG)(0.50 * size_rcHeight);
 

    LONG xModal8 = (LONG)(0.00 * size_rcWidth);
    LONG yModal8 = (LONG)(0.60 * size_rcHeight);
    LONG widthModal8 = (LONG)(size_rcWidth * .40);
    LONG heightModal8 = (LONG)(widthModal8/1.8);
 

    m_wndView.MoveWindow(xPlayer1, yPlayer1, widthPlayer1, heightPlayer1, SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    m_wndView2.MoveWindow(xPlayer2, yPlayer2, widthPlayer2, heightPlayer2, SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    ::MoveWindow(open_modal,xModal3,yModal3,widthModal3, heightModal3, SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    ::MoveWindow(trim1, xModal4, yModal4, widthModal4, heightModal4, SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    ::MoveWindow(trim2, xModal5, yModal5, widthModal4, heightModal4, SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    ::MoveWindow(video_player1_title, (LONG)((.1) * size_rcClient.right - size_rcClient.left),
        yModal4, yModal4,
        (LONG)(.05 * (size_rcClient.bottom - size_rcClient.top)), SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    ::MoveWindow(video_player2_title, (LONG)((.7) * size_rcClient.right - size_rcClient.left), 
        yModal4, yModal4,
        (LONG)(.05 * (size_rcClient.bottom - size_rcClient.top)), SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    ::MoveWindow(console_display, xModal8, yModal8, widthModal8, heightModal8, SWP_DRAWFRAME | SWP_NOSIZE | SWP_NOMOVE | SWP_NOACTIVATE);
    RECT movedRect = { xModal8, yModal8, widthModal8, heightModal8 };
    ::RedrawWindow(console_display, &movedRect, 0, RDW_INVALIDATE | RDW_FRAME);



    HBRUSH brush = CreateSolidBrush(RGB(79, 91, 102)); //create brush
    RECT rect;

    int w_length = (int)(wcslen(L"Input Video"));
    // Write a line of text to the client area.
   
    HDC InputVideoDC = ::GetDC(video_player1_title);
    SelectObject(InputVideoDC, brush); //select brush into DC
    Rectangle(InputVideoDC, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); //draw rectangle over whole screen

    SetTextColor(InputVideoDC, 0x00FFFFFF);

    SetTextAlign(InputVideoDC, TA_CENTER);
    ::GetClientRect(video_player1_title, &rect);
    SetBkMode(InputVideoDC, TRANSPARENT);
    TextOut(InputVideoDC, (rect.right + rect.left) / 2, (rect.bottom + rect.top) / 6,
        L"Input Video", static_cast<int> (w_length));
    
    HDC screenDC = GetDC(); //NULL gets whole screen
   
    SelectObject(screenDC, brush); //select brush into DC
    Rectangle(screenDC, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); //draw rectangle over whole screen
    
    ::ReleaseDC(NULL,screenDC);
 
    HDC OutputVideoDC = ::GetDC(video_player2_title); //NULL gets whole screen

    w_length = (int)wcslen(L"Output Video");
    // Write a line of text to the client area.
    SelectObject(OutputVideoDC, brush); //select brush into DC
    Rectangle(OutputVideoDC, 0, 0, GetSystemMetrics(SM_CXSCREEN), GetSystemMetrics(SM_CYSCREEN)); //draw rectangle over whole screen
    SetTextAlign(OutputVideoDC, TA_CENTER);
    SetTextColor(OutputVideoDC, 0x00FFFFFF);

    ::GetClientRect(video_player2_title, &rect);
    SetBkMode(OutputVideoDC, TRANSPARENT);
    TextOut(OutputVideoDC, (rect.right + rect.left) / 2, (rect.bottom + rect.top) / 6,
        L"Output Video", static_cast<int> (w_length));

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
        CComPtr<IWMPPlayer2> spWMPPlayer2;

        hr = m_spWMPPlayer.QueryInterface(&spWMPPlayer2);
        if (FAILMSG(hr))
            return 0;

        if (FAILMSG(hr))
            return 0;
        hr = spWMPPlayer2->put_stretchToFit(true);

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
    auto extension = SysAllocString(L".mp4");
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
    if ((HWND)lParam == trim1)
    {
        BOOL boolean = TRUE;
        WORD type = (WORD)0;
        TrimVideo(NULL, type, NULL, boolean);

    }
    if ((HWND)lParam == trim2)
    {
        BOOL boolean = TRUE;
        WORD type = (WORD)1;
        TrimVideo(NULL, type, NULL, boolean);

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
LRESULT CWMPHost::TrimVideo(WORD /* wNotifyCode */, WORD  wID , HWND /* hWndCtl */, BOOL& /* bHandled */) {
    BSTR video_name, video_file_and_path;
    if (wID == WORD(1)) {
        video_name = SELECTED_VIDEOFILENAME_MACRO2;
        video_file_and_path = SELECTED_VIDEO_MACRO2;
    }
    else {
        video_name = SELECTED_VIDEOFILENAME_MACRO;
        video_file_and_path = SELECTED_VIDEO_MACRO;
    }
    CComBSTR    m_bstrValue;
    CStringDlg dlgString(L"Trim Video", m_bstrValue);
    CComBSTR   start, end;
    if (dlgString.DoModal(m_hWnd) == IDCANCEL) {
        return 0;
    }
    if (dlgString.DoModal(m_hWnd) == IDOK)
    {
        start = dlgString.m_bstrValue1;
        end = dlgString.m_bstrValue2;
        if (start == ATL::CComBSTR(L"") || end == ATL::CComBSTR(L"")) {
            return 0;
        }
    }

    CString solutionCString = MY_SOLUTIONDIR;

    //ffmpeg -ss 00:01:00 -to 00:02:00 -i input.mp4 -c copy output.mp4
    auto withK = Concat(SysAllocString(L"/k "), solutionCString.AllocSysString());
    auto begin = Concat(withK, SysAllocString(L"\\ffmpeggtest.bat "));
    auto time_start = Concat(begin, SysAllocString(start));
    auto time_start_space = Concat(SysAllocString(time_start), SysAllocString(L" "));
    auto time_end = Concat(time_start_space, SysAllocString(end));
    auto time_end_space = Concat(SysAllocString(time_end), SysAllocString(L" "));

    auto input_file_name = Concat(time_end_space, SysAllocString(video_file_and_path));
    auto input_file_name_space = Concat(SysAllocString(input_file_name), SysAllocString(L" "));

    auto output_file_folder = Concat(input_file_name_space, SysAllocString(selected_folder_macro));

    auto output_file_folder_with_slash = Concat(output_file_folder, SysAllocString(L"\\"));
    std::wstring wstr(video_name);
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



    auto output_file_name = Concat(output_file_folder_with_slash, file_wo_mp4);

    auto penultimate = Concat(output_file_name, SysAllocString(L"_trimmed"));
    auto final_result = Concat(penultimate, SysAllocString(extension));
 
    ShellExecute(NULL, _T("open"),_T("cmd.exe"), final_result, NULL, SW_SHOW);
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
LRESULT OnUpDownOk(HWND hWnd, int id, HWND /*hWndCtl*/ , UINT /*codeNotify*/) {



    if (id == 1) {
        TCHAR* TBuf = 0;
        int BufSize = 0;
        BufSize = Edit_GetTextLength(hEdit) + 1;
        TBuf = new TCHAR[BufSize];

        Edit_GetText(hEdit, TBuf, BufSize);
        OutputDebugString(L"Value:");
        //_bstr_t bstrResult = _bstr_t(TBuf);
        OutputDebugString(TBuf);

        CString solutionCString = MY_SOLUTIONDIR;
        auto restOfFile = SysAllocString(L"/C Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\test.bat ");
        BSTR withK = Concat(SysAllocString(L"/k "), solutionCString.AllocSysString());

        BSTR cmd = Concat(withK, restOfFile);
        //auto a = SysAllocString(L"/k %CD%\\Super-Slow-Motion-Video-LN-ProjectStructure\\UserDir\\ExampleProj1\\test.bat ");
        BSTR videoArg = SysAllocString(SELECTED_VIDEOFILENAME_MACRO);
        BSTR space = SysAllocString(L" ");
        BSTR slowdownArg = SysAllocString(TBuf);
        SELECTED_SLOWDOWN_MACRO = slowdownArg;
        auto first_result = Concat(cmd, videoArg);
        auto second_result = Concat(space,first_result);
        

        auto final_result = Concat(second_result, slowdownArg);
        // OutputDebugString(final_result);

         //OutputDebugString(final_result);
        ShellExecute(NULL, _T("open"), _T("cmd.exe"), final_result, NULL, SW_SHOW);

        SysFreeString(cmd);
        SysFreeString(withK);
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

        HANDLE_MSG(hWnd, WM_COMMAND, OnUpDownOk);

    default:
        return FALSE;	// Let system deal with msg
    }

}

#pragma endregion



LRESULT CALLBACK ConsoleProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    TEXTMETRIC tm;
    SCROLLINFO si;

    // These variables are required to display text. 
    static int xClient;     // width of client area 
    static int yClient;     // height of client area 
    static int xClientMax;  // maximum width of client area 

    static int xChar;       // horizontal scrolling unit 
    static int yChar;       // vertical scrolling unit 
    static int xUpper;      // average width of uppercase letters 

    static int xPos;        // current horizontal scrolling position 
    static int yPos;        // current vertical scrolling position 

    int i;                  // loop counter 
    int x, y;               // horizontal and vertical coordinates

    int FirstLine;          // first line in the invalidated area 
    int LastLine;           // last line in the invalidated area 
    size_t w_length;        // length of an abc[] item 

    // Create an array of lines to display. 

  

    switch (uMsg)
    {

    case WM_CREATE:
        // Get the handle to the client area's device context. 
        hdc = GetDC(hwnd);

        // Extract font dimensions from the text metrics. 
        GetTextMetrics(hdc, &tm);
        xChar = tm.tmAveCharWidth;
        xUpper = (tm.tmPitchAndFamily & 1 ? 3 : 2) * xChar / 2;
        yChar = tm.tmHeight + tm.tmExternalLeading;

        // Free the device context. 
        ReleaseDC(hwnd, hdc);

        // Set an arbitrary maximum width for client area. 
        // (xClientMax is the sum of the widths of 48 average 
        // lowercase letters and 12 uppercase letters.) 
        xClientMax = 48 * xChar + 12 * xUpper;

        return 0;

    case WM_SIZE:

        // Retrieve the dimensions of the client area. 
        yClient = HIWORD(lParam);
        xClient = LOWORD(lParam);

        // Set the vertical scrolling range and page size
        si.cbSize = sizeof(si);
        si.fMask = SIF_RANGE | SIF_PAGE;
        si.nMin = 0;
        si.nMax = LINES - 1;
        si.nPage = yClient / yChar;
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);

        return 0;
    case WM_HSCROLL:
        // Get all the vertial scroll bar information.
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;

        // Save the position for comparison later on.
        GetScrollInfo(hwnd, SB_HORZ, &si);
        xPos = si.nPos;
        switch (LOWORD(wParam))
        {
            // User clicked the left arrow.
        case SB_LINELEFT:
            si.nPos -= 1;
            break;

            // User clicked the right arrow.
        case SB_LINERIGHT:
            si.nPos += 1;
            break;

            // User clicked the scroll bar shaft left of the scroll box.
        case SB_PAGELEFT:
            si.nPos -= si.nPage;
            break;

            // User clicked the scroll bar shaft right of the scroll box.
        case SB_PAGERIGHT:
            si.nPos += si.nPage;
            break;

            // User dragged the scroll box.
        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
            break;

        default:
            break;
        }

        // Set the position and then retrieve it.  Due to adjustments
        // by Windows it may not be the same as the value set.
        si.fMask = SIF_POS;
        SetScrollInfo(hwnd, SB_HORZ, &si, TRUE);
        GetScrollInfo(hwnd, SB_HORZ, &si);

        // If the position has changed, scroll the window.
        if (si.nPos != xPos)
        {
            ScrollWindow(hwnd, xChar * (xPos - si.nPos), 0, NULL, NULL);
        }

        return 0;

    case WM_VSCROLL:
        // Get all the vertial scroll bar information.
        si.cbSize = sizeof(si);
        si.fMask = SIF_ALL;
        GetScrollInfo(hwnd, SB_VERT, &si);

        // Save the position for comparison later on.
        yPos = si.nPos;
        switch (LOWORD(wParam))
        {

            // User clicked the HOME keyboard key.
        case SB_TOP:
            si.nPos = si.nMin;
            break;

            // User clicked the END keyboard key.
        case SB_BOTTOM:
            si.nPos = si.nMax;
            break;

            // User clicked the top arrow.
        case SB_LINEUP:
            si.nPos -= 1;
            break;

            // User clicked the bottom arrow.
        case SB_LINEDOWN:
            si.nPos += 1;
            break;

            // User clicked the scroll bar shaft above the scroll box.
        case SB_PAGEUP:
            si.nPos -= si.nPage;
            break;

            // User clicked the scroll bar shaft below the scroll box.
        case SB_PAGEDOWN:
            si.nPos += si.nPage;
            break;

            // User dragged the scroll box.
        case SB_THUMBTRACK:
            si.nPos = si.nTrackPos;
            break;

        default:
            break;
        }

        // Set the position and then retrieve it.  Due to adjustments
        // by Windows it may not be the same as the value set.
        si.fMask = SIF_POS;
        SetScrollInfo(hwnd, SB_VERT, &si, TRUE);
        GetScrollInfo(hwnd, SB_VERT, &si);

        // If the position has changed, scroll window and update it.
        if (si.nPos != yPos)
        {
            ScrollWindow(hwnd, 0, yChar * (yPos - si.nPos), NULL, NULL);
            UpdateWindow(hwnd);
        }

        return 0;

    case WM_PAINT:
       
        // Prepare the window for painting.
        hdc = BeginPaint(hwnd, &ps);

        // Get vertical scroll bar position.
        si.cbSize = sizeof(si);
        si.fMask = SIF_POS;
        GetScrollInfo(hwnd, SB_VERT, &si);
        yPos = si.nPos;

        // Get horizontal scroll bar position.
        GetScrollInfo(hwnd, SB_HORZ, &si);
        xPos = si.nPos;
       
        // Find painting limits.
        FirstLine = max(0, yPos + ps.rcPaint.top / yChar);
        LastLine = min(LINES - 1, yPos + ps.rcPaint.bottom / yChar);

        for (i = FirstLine; i <= LastLine; i++)
        {
            x = xChar * (1 - xPos);
            y = yChar * (i - yPos);

            // Note that "55" in the following depends on the 
            // maximum size of an abc[] item. Also, you must include
            // strsafe.h to use the StringCchLength function.
            SetTextColor(hdc, 0x00FFFFFF);
            w_length = wcslen(console_output.at(i));
            SetBkMode(hdc, TRANSPARENT);

            // Write a line of text to the client area.
            TextOut(hdc, x, y, console_output.at(i), static_cast<int>(w_length));
        }

        // Indicate that painting is finished.
        EndPaint(hwnd, &ps);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }

    return DefWindowProc(hwnd, uMsg, wParam, lParam);
}
LRESULT CALLBACK titleTwo(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
    HDC hdc;
    PAINTSTRUCT ps;
    TEXTMETRIC tm;

    // These variables are required to display text. 
    static int xClient;     // width of client area 
    static int yClient;     // height of client area 
    static int xClientMax;  // maximum width of client area 

    static int xChar;       // horizontal scrolling unit 
    static int yChar;       // vertical scrolling unit 
    static int xUpper;      // average width of uppercase letters 

    switch (uMsg)
    {

    case WM_CREATE:
        // Get the handle to the client area's device context. 
        hdc = GetDC(hwnd);

        // Extract font dimensions from the text metrics. 
        GetTextMetrics(hdc, &tm);
        xChar = tm.tmAveCharWidth;
        xUpper = (tm.tmPitchAndFamily & 1 ? 3 : 2) * xChar / 2;
        yChar = tm.tmHeight + tm.tmExternalLeading;

        // Free the device context. 
        ReleaseDC(hwnd, hdc);

        // Set an arbitrary maximum width for client area. 
        // (xClientMax is the sum of the widths of 48 average 
        // lowercase letters and 12 uppercase letters.) 
        xClientMax = 48 * xChar + 12 * xUpper;

        return 0;

    case WM_SIZE:

        // Retrieve the dimensions of the client area. 
        yClient = HIWORD(lParam);
        xClient = LOWORD(lParam);
        return 0;

    case WM_PAINT:

        // Prepare the window for painting.
        hdc = BeginPaint(hwnd, &ps);
        // Indicate that painting is finished.
        EndPaint(hwnd, &ps);
        return 0;

    case WM_DESTROY:
        PostQuitMessage(0);
        return 0;
    }
    return DefWindowProc(hwnd, uMsg, wParam, lParam);

}