
#pragma once
#include <list>
#include <map>
#include <TOM.h>
#include "RichEditObj.h"
#include "ImgProvider.h"
#include "RichEditOleBase.h"
#include "IRichEditObjHost.h"
#include "control/SRichEdit.h"

class SImRichEdit : public SOUI::SRichEdit, IRichEditObjHost
{
    struct UpdateContext
    {
        BOOL bReadOnly;
        BOOL bWordWrap;
        BOOL bHasScrollBar;
    };

    SOUI_CLASS_NAME(SImRichEdit, L"imrichedit");
#define RECONTENT_LAST   4294967294 /*内容插入在结尾(2^32-2)*/
#define RECONTENT_CARET  4294967295 /*内容插入在光标(2^32-1)*/

public:
    SImRichEdit();
    ~SImRichEdit();

    //
    // IRichEditObjHost methods
    //

	void            DirectDraw(const SOUI::CRect& rc);
	void            DelayDraw(const SOUI::CRect& rc);
    int             GetContentLength();
    ITextRange*     GetTextRange(const CHARRANGE& chr);
    HRESULT         SendMessage(UINT uMsg, WPARAM wParam, LPARAM lParam, LRESULT *pRet);
	SOUI::ISwndContainer * GetHostContainer() { return GetContainer(); }
	SOUI::CRect           GetHostRect()       { return GetClientRect(); }
	SOUI::CRect           GetAdjustedRect();
    ITextDocument * GetTextDoc()        { return m_pTxtDoc; }

    //
    // richedit message wrapper
    //

    void            SetFontSize(int size);
    void            SetSelectionColor(COLORREF cr);
    void            GetSel(long* pStartCp, long* pEndCp);
    void            SetSel(int nStart, int nEnd);
    void            ScrollEnd();
    BOOL            SetParaFormat(PARAFORMAT2& pf);
    int             LineFromChar(int ncp);
    int             LineIndex(int nLineNum);
    int             LineLength(int nLineCP /* = -1 */);
    void            PosFromChar(UINT ncp, POINT* pPos);
    void            GetVisibleCharRange(CHARRANGE& chr);
    int             CharFromPos(POINT pt);
    int             GetFirstVisibleLine();

    //
    // extent methods
    //

    BOOL                InsertImage(LPCWSTR lpImagePath);    
    UINT                InserSpaceLine(UINT uInsertAt=RECONTENT_LAST);
    int                InsertContent(LPCWSTR lpszContent, UINT uInsertAt=RECONTENT_LAST);
    RichEditContent *   GetContent(UINT uIndex);
    void                DeleteContent(UINT uIndex);
    void                Clear();
    void                SetCapbility(int nCap);
    RichEditObj *       GetElementById(LPCWSTR lpszId);
    RichEditOleBase *   GetOleById(LPCWSTR lpszId);

protected:
    LRESULT             OnCreate( LPVOID );
	void                OnSize(UINT nType, SOUI::CSize size);
    void                OnDestroy();
	void                OnPaint(SOUI::IRenderTarget * pRT);
	void                OnLButtonDown(UINT nFlags, SOUI::CPoint point);
	void                OnLButtonUp(UINT nFlags, SOUI::CPoint point);
	void                OnRButtonDown(UINT nFlags, SOUI::CPoint point);
	void                OnMouseMove(UINT nFlags, SOUI::CPoint point);
	BOOL                OnSetCursor(const SOUI::CPoint &pt);
	BOOL                OnMouseWheel(UINT nFlags, short zDelta, SOUI::CPoint pt);
	BOOL                OnUpdateToolTip(SOUI::CPoint pt, SOUI::SwndToolTipInfo &tipInfo);
    BOOL                OnScroll( BOOL bVertical,UINT uCode,int nPos );

    BOOL                GetDefCharFormat(CHARFORMAT& cf);
    BOOL                SetDefCharFormat(CHARFORMAT& cf);

    RichEditObj*        HitTest(RichEditObj* p, POINT ptInControl);
    RichEditObj*        HitTest(POINT ptInControl);
    void                SetOleCallback();
    void                PreUpdate(UpdateContext&);
    void                DoneUpdate(const UpdateContext&);
    void                SortContents(UINT uSortAfter);
    RichEditContent*    CreateRichEditConent(LPCWSTR lpszContent);
    void                UpdateBkgndRenderTarget();

    SOUI_MSG_MAP_BEGIN()
        MSG_WM_CREATE(OnCreate)
        MSG_WM_DESTROY(OnDestroy)
        MSG_WM_PAINT_EX(OnPaint)
        MSG_WM_SIZE(OnSize)
        MSG_WM_LBUTTONDOWN(OnLButtonDown)
        MSG_WM_LBUTTONUP(OnLButtonUp)
        MSG_WM_RBUTTONDOWN(OnRButtonDown)
        MSG_WM_MOUSEWHEEL(OnMouseWheel)
        MSG_WM_MOUSEMOVE(OnMouseMove)
    SOUI_MSG_MAP_END()
    
private:
	typedef SOUI::SArray<RichEditContent*> RichContentArray;

	SOUI::CAutoRefPtr<SOUI::IRenderTarget> m_prtBackground;  /**< 缓存窗口绘制的RT */
    BOOL                m_bBkgndDirty;

    ITextDocument*     m_pTxtDoc;
    RichContentArray    m_arrContent;
    RichEditObj*        m_pLastOverOle;
	SOUI::CRect               m_rcDelayDraw;      // 延迟刷新的叠加区域
    time_t              m_nTicksLastDraw;   // 最后一次绘制的时间
};
