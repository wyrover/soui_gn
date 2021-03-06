// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "SwndFrame.h"
#include "SCaptureButton.h"

class CMainDlg : public SOUI::SHostWnd
{
public:
	CMainDlg();
	~CMainDlg();

	void OnClose()
	{
		AnimateHostWindow(200,AW_CENTER|AW_HIDE);
        DestroyWindow();
	}
	void OnMaximize()
	{
		SendMessage(WM_SYSCOMMAND,SC_MAXIMIZE);
	}
	void OnRestore()
	{
		SendMessage(WM_SYSCOMMAND,SC_RESTORE);
	}
	void OnMinimize()
	{
		SendMessage(WM_SYSCOMMAND,SC_MINIMIZE);
	}
    
    void OnBtnSearch();
protected:
	void OnSize(UINT nType, SOUI::CSize size)
	{
		SetMsgHandled(FALSE);
		if(!m_bLayoutInited) return;
		if(nType==SIZE_MAXIMIZED)
		{
			FindChildByName(L"btn_restore")->SetVisible(TRUE);
			FindChildByName(L"btn_max")->SetVisible(FALSE);
		}else if(nType==SIZE_RESTORED)
		{
            FindChildByName(L"btn_restore")->SetVisible(FALSE);
            FindChildByName(L"btn_max")->SetVisible(TRUE);
		}
	}
        
	void OnShowWindow(BOOL bShow, UINT nStatus);
    BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);

    BOOL OnEventCaptureHost(SOUI::EventArgs *pEvt);
    BOOL OnEventCaptureHostFinish(SOUI::EventArgs *pEvt);

    BOOL OnEventCaptureSwnd(SOUI::EventArgs *pEvt);
    BOOL OnEventCaptureSwndFinish(SOUI::EventArgs *pEvt);
    
    BOOL OnEventTreeSelChanged(SOUI::EventArgs *pEvt);
    
    BOOL OnCopyData(HWND hSend, PCOPYDATASTRUCT pCopyDataStruct);

    void EnumSwnd(HWND hHost ,DWORD dwSwnd, HSTREEITEM hItem);
protected:

	EVENT_MAP_BEGIN()
		EVENT_NAME_COMMAND(L"btn_close",OnClose)
        EVENT_NAME_COMMAND(L"btn_min",OnMinimize)
        EVENT_NAME_COMMAND(L"btn_max",OnMaximize)
        EVENT_NAME_COMMAND(L"btn_restore",OnRestore)
		EVENT_NAME_HANDLER(L"img_capture_host", SOUI::EventCapture::EventID, OnEventCaptureHost)
		EVENT_NAME_HANDLER(L"img_capture_host", SOUI::EventCaptureFinish::EventID, OnEventCaptureHostFinish)
		EVENT_NAME_HANDLER(L"img_capture_swnd", SOUI::EventCapture::EventID, OnEventCaptureSwnd)
        EVENT_NAME_HANDLER(L"img_capture_swnd",SOUI::EventCaptureFinish::EventID,OnEventCaptureSwndFinish)
		EVENT_NAME_HANDLER(L"tree_swnd", SOUI::EventTCSelChanged::EventID, OnEventTreeSelChanged)
        EVENT_NAME_COMMAND(L"btn_search",OnBtnSearch)
	EVENT_MAP_END()	

	BEGIN_MSG_MAP_EX(CMainDlg)
        MSG_WM_INITDIALOG(OnInitDialog)
		MSG_WM_CLOSE(OnClose)
		MSG_WM_SIZE(OnSize)
		MSG_WM_SHOWWINDOW(OnShowWindow)
        MSG_WM_COPYDATA(OnCopyData)
		CHAIN_MSG_MAP(SHostWnd)
		REFLECT_NOTIFICATIONS_EX()
	END_MSG_MAP()
private:
	BOOL			m_bLayoutInited;

	SOUI::STreeCtrl     * m_pTreeCtrl;

	SOUI::SwndFrame       m_wndFrame;
};
