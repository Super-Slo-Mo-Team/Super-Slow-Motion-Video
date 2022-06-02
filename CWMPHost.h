// CWMPHost.h : Declaration of the CWMPHost
//
// Copyright (c) Microsoft Corporation. All rights reserved.
//

#include "resource.h"       // main symbols
#include <oledlg.h>
#include "wmp.h"
#include "CWMPEventDispatch.h"


/////////////////////////////////////////////////////////////////////////////
// CWMPHost

class CWMPHost : public CWindowImpl<CWMPHost, CWindow, CWinTraits<WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_CLIPCHILDREN | WS_CLIPSIBLINGS, WS_EX_APPWINDOW | WS_EX_WINDOWEDGE> >
{
public:
    DECLARE_WND_CLASS_EX(NULL, 0, 0)

    BEGIN_MSG_MAP(CWMPHost)
        MESSAGE_HANDLER(WM_CREATE, OnCreate)
        MESSAGE_HANDLER(WM_DESTROY, OnDestroy)
        MESSAGE_HANDLER(WM_SIZE, OnSize)
        MESSAGE_HANDLER(WM_ERASEBKGND, OnErase)


        //MESSAGE_HANDLER(WM_CONTEXTMENU, FowardMsgToWMP)
        COMMAND_ID_HANDLER(ID_FILE_OPEN, OnFileOpen)
        COMMAND_ID_HANDLER(ID_APP_EXIT, OnFileExit)
        //MESSAGE_HANDLER(WM_INITDIALOG, OnInitDialog)

        COMMAND_ID_HANDLER(ID_WMPCORE_CLOSE, OnWMPCoreClose)
        COMMAND_ID_HANDLER(ID_WMPCORE_URL, OnWMPCoreURL)
        COMMAND_ID_HANDLER(ID_WMPCORE_OPENSTATE, OnWMPCoreOpenState)
        COMMAND_ID_HANDLER(ID_WMPCORE_PLAYSTATE, OnWMPCorePlayState)
        COMMAND_ID_HANDLER(ID_WMPCORE_VERSIONINFO, OnWMPCoreVersionInfo)
        COMMAND_ID_HANDLER(ID_WMPCORE_LAUNCHURL, OnWMPCoreLaunchURL)
        COMMAND_ID_HANDLER(ID_WMPCORE_ISONLINE, OnWMPCoreIsOnline)
        COMMAND_ID_HANDLER(ID_WMPCORE_STATUS, OnWMPCoreStatus)
        COMMAND_ID_HANDLER(ID_WMPCORE_CONTROLS, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_SETTINGS, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_CURRENTMEDIA, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_MEDIACOLLECTION, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_PLAYLISTCOLLECTION, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_NETWORK, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_CURRENTPLAYLIST, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_CDROMCOLLECTION, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_CLOSEDCAPTION, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_ERROR, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE2_DVD, OnWMPCoreInterface)
        COMMAND_ID_HANDLER(ID_WMPCORE_SELECTFOLDER, OnWMPSelectFolder)
       // COMMAND_ID_HANDLER(ID_TESTSHELL, OnTestShell)

        COMMAND_ID_HANDLER(ID_WMPPLAYER_ENABLED, OnWMPPlayerEnabled)
        COMMAND_ID_HANDLER(ID_WMPPLAYER_FULLSCREEN, OnWMPPlayerFullScreen)
        COMMAND_ID_HANDLER(ID_WMPPLAYER_ENABLECONTEXTMENU, OnWMPPlayerEnableContextMenu)
        COMMAND_ID_HANDLER(ID_WMPPLAYER_UIMODE, OnWMPPlayerUIMode)
        COMMAND_ID_HANDLER(ID_WMPPLAYER2_STRETCHTOFIT, OnWMPPlayer2StretchToFit)
        COMMAND_ID_HANDLER(ID_2xOption, twoxOption)
        COMMAND_ID_HANDLER(ID_3xOption, threexOption)
        COMMAND_ID_HANDLER(ID_4xOption, fourxOption)
        COMMAND_ID_HANDLER(ID_PLAYSLOMO, PlaySlomo)
        COMMAND_ID_HANDLER(ID_TRIMVIDEO, TrimVideo)
        MESSAGE_HANDLER(WM_COMMAND, handle_object_messaages)

    END_MSG_MAP()

    void OnFinalMessage(HWND /*hWnd*/);

    LRESULT OnDestroy(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& bHandled);
    LRESULT OnCreate(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */);
    LRESULT OnErase(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */);
    LRESULT OnSize(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */);
    LRESULT OnFileOpen(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnFileExit(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreClose(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreURL(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreOpenState(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCorePlayState(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreVersionInfo(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreLaunchURL(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreIsOnline(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreStatus(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPCoreInterface(WORD /* wNotifyCode */, WORD wID, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPSelectFolder(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);

    LRESULT OnWMPPlayerEnabled(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPPlayerFullScreen(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPPlayerEnableContextMenu(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnWMPPlayerUIMode(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);

    LRESULT OnWMPPlayer2StretchToFit(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    //LRESULT FowardMsgToWMP(UINT uMsg, WPARAM wParam, LPARAM lParam, BOOL& bHandled);
    //LRESULT OnTestShell(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT TrimVideo(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);

    LRESULT twoxOption(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT threexOption(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT fourxOption(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT PlaySlomo(WORD /* wNotifyCode */, WORD /* wID */, HWND /* hWndCtl */, BOOL& /* bHandled */);
    LRESULT OnFullScreen(UINT /* uMsg */, WPARAM /* wParam */, LPARAM /* lParam */, BOOL& /* bHandled */);
    LRESULT handle_object_messaages(UINT  uMsg, WPARAM  wParam, LPARAM  lParam, BOOL& bHandled);
    LRESULT registerPopupClass(HINSTANCE hInst);
    LRESULT displayPopup(HWND hwnd);
    LRESULT OnUpDownOk(HWND hWnd, int /*id*/, HWND /*hWndCtl*/, UINT /*codeNotify*/);

    BSTR extract_filename(BSTR path);

    CAxWindow                   m_wndView;
    CAxWindow                   m_wndView2;

    CComPtr<IWMPPlayer>         m_spWMPPlayer;
    CComPtr<IConnectionPoint>   m_spConnectionPoint;
    CComPtr<IWMPPlayer>         m_2spWMPPlayer;
    CComPtr<IConnectionPoint>   m_spConnectionPoint2;
    DWORD                       m_dwAdviseCookie;
    DWORD                       m_dwAdviseCookie2;
    HWND                        video_player1_title;
    HWND                        video_player2_title;
    HWND                        console_display;
    HWND                        open_modal;
    HWND                        updown_box;
    HWND                        updown_control;
    HWND                        updown_background;
    HWND                        trim1;
    HWND                        trim2;
    HWND                        popUp_hWnd;
    WNDCLASSW                   popup = { 0 };
    HWND                         hDlg;

};