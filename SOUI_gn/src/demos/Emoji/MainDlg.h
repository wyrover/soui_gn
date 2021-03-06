// MainDlg.h : interface of the CMainDlg class
//
/////////////////////////////////////////////////////////////////////////////
#pragma once

#include "FaceList_soui.h"
#define	FACE_CTRL_SEL  WM_USER + 1001

namespace SOUI
{
	enum
	{
		WM_SELECT_EMOJI = WM_USER + 100,
		WM_HOVER_EMOJI = WM_USER + 101,
		WM_LEAVE_EMOJI = WM_USER + 102,
	};
	class CEmojiDlg : public SHostWnd
	{
	public:
		CEmojiDlg();
		~CEmojiDlg();

		void OnClose();
		void OnMaximize();
		void OnRestore();
		void OnMinimize();
		void OnBtnSet();
		void OnDelete();
		void OnBtnSelectAll();
		void UnSelectAll();

		void OnBtnMsgBox();
		int OnCreate(LPCREATESTRUCT lpCreateStruct);
		BOOL OnInitDialog(HWND wndFocus, LPARAM lInitParam);
		void SetFaceList(CFaceList_SOUI* lpSysFaceList, CFaceList_SOUI* lpMyFaceList, HWND hParentWnd, SStringT szMyPath = _T(""));


	protected:
		//soui消息
		EVENT_MAP_BEGIN()
			EVENT_NAME_COMMAND(L"btn_close", OnClose)
			EVENT_NAME_COMMAND(L"btn_min", OnMinimize)
			EVENT_NAME_COMMAND(L"btn_max", OnMaximize)
			EVENT_NAME_COMMAND(L"btn_restore", OnRestore)
			EVENT_NAME_COMMAND(L"btnset", OnBtnSet)
			EVENT_NAME_COMMAND(L"btnDel", OnDelete)
			EVENT_NAME_COMMAND(L"quitManage", OnBtnQuitManage)
			EVENT_NAME_COMMAND(L"btnSelectAll", OnBtnSelectAll)
			EVENT_MAP_END()

			//HostWnd真实窗口消息处理
			BEGIN_MSG_MAP_EX(CEmojiDlg)
			MSG_WM_CREATE(OnCreate)
			MSG_WM_INITDIALOG(OnInitDialog)
			MSG_WM_CLOSE(OnClose)
			MSG_WM_COMMAND(OnCommand)
			MSG_WM_ACTIVATE(OnActivate)
			MSG_WM_LBUTTONDOWN(OnLButtonDown)
			MESSAGE_HANDLER_EX(WM_SELECT_EMOJI, OnSelectEmoji)
			MESSAGE_HANDLER_EX(WM_HOVER_EMOJI, OnHoverEmoji)
			MESSAGE_HANDLER_EX(WM_LEAVE_EMOJI, OnLeaveEmoji)

			CHAIN_MSG_MAP(SHostWnd)
			REFLECT_NOTIFICATIONS_EX()
			END_MSG_MAP()

		void OnCommand(UINT uNotifyCode, int nID, HWND wndCtl);
		// 添加表情
		void OnAddEmoji();
		// 管理表情
		void OnManageEmoji();
		// 退出管理
		void OnBtnQuitManage();
		LRESULT OnSelectEmoji(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnHoverEmoji(UINT uMsg, WPARAM wParam, LPARAM lParam);
		LRESULT OnLeaveEmoji(UINT uMsg, WPARAM wParam, LPARAM lParam);
		void OnActivate(UINT nState, BOOL bMinimized, HWND wndOther);
		bool GetManageStatus();
		bool OnEmojiPageChanging(EventArgs* pEvt);
		void OnLButtonDown(UINT nFlags, CPoint point);
	private:
		BOOL			m_bLayoutInited;
		CFaceList_SOUI*		m_pSystemFaceList;
		CFaceList_SOUI*		m_pMyFaceList;
		bool     m_bManaging;
		bool     m_bPopSetMenu;
		HWND     m_hParenWND;
		SStringT m_MyFacePath;
	public:
		SStringT m_strSelectEmoji;
	};

}
