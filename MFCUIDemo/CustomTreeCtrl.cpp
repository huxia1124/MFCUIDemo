#include "pch.h"
#include "CustomTreeCtrl.h"
#include "HighlightTextPainter.h"

#define TREE_EMPTY_BUTTON_ID	700

BEGIN_MESSAGE_MAP(CustomTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CustomTreeCtrl::OnNMCustomdraw)
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_BN_CLICKED(TREE_EMPTY_BUTTON_ID, OnButtonClicked)
	ON_WM_CREATE()
	ON_WM_SETFONT()
END_MESSAGE_MAP()


void CustomTreeCtrl::SetHighlightTokens(const CString& tokens)
{
	m_highlightVer++;
	m_childrenHighligh.clear();

	m_highlightTokens = tokens;
	if (GetSafeHwnd())
		Invalidate();
}

void CustomTreeCtrl::SetAdditionalTextCallback(std::function<void(CustomTreeCtrl*, HTREEITEM, CString&)> callback)
{
	m_additionalTextCallback = callback;
}

void CustomTreeCtrl::SetEmptyMessage(LPCTSTR lpszEmptyMessage)
{
	m_emptyMessage = lpszEmptyMessage;

	if (GetSafeHwnd())
	{
		UpdateButtonPosition();
		Invalidate();
	}
}

void CustomTreeCtrl::SetEmptyButton(LPCTSTR lpszButtonCaption)
{
	m_emptyButtonCaption = lpszButtonCaption;
	UpdateButtonVisibility();
}

void CustomTreeCtrl::UpdateHoverItem(HTREEITEM item)
{
	if (m_hoverItem != item)
	{
		m_hoverItem = item;

		if (GetSafeHwnd())
			Invalidate();
	}
}

void CustomTreeCtrl::FullRowSelect(const CPoint& pt)
{
	TVHITTESTINFO ti = {};
	ti.pt = pt;
	HTREEITEM item = HitTest(&ti);

	if(item)
		SelectItem(item);

}

BOOL CustomTreeCtrl::FullRowDoubleClick(const CPoint& pt, HTREEITEM& item, bool& onItemRight)
{
	TVHITTESTINFO ti = {};
	ti.pt = pt;
	item = HitTest(&ti);

	CString itemText = GetItemText(item);

	if (ItemHasChildren(item))
	{
		if (item && (ti.flags & (TVHT_ONITEMINDENT | TVHT_ONITEMRIGHT | TVHT_ONITEMLABEL)))
		{
			Expand(item, TVE_TOGGLE);
			return TRUE;
		}
	}

	onItemRight = (ti.flags & TVHT_ONITEMRIGHT);
	
	return FALSE;
}

void CustomTreeCtrl::UpdateButtonVisibility()
{
	if (GetRootItem() || m_emptyButtonCaption.IsEmpty())
	{
		m_emptyButton.DestroyWindow();
	}
	else
	{
		if (!m_emptyButton.GetSafeHwnd())
		{
			m_emptyButton.Create(m_emptyButtonCaption, WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, CRect(), this, TREE_EMPTY_BUTTON_ID);
			m_emptyButton.SetFont(GetFont());
		}

		UpdateButtonPosition();
	}
}

void CustomTreeCtrl::UpdateButtonPosition()
{
	if (!m_emptyButton.GetSafeHwnd() || !GetSafeHwnd())
		return;

	const int buttonMinWidth = static_cast<int>(afxGlobalData.GetRibbonImageScale() * 60.0);
	const int buttonHeight = static_cast<int>(afxGlobalData.GetRibbonImageScale() * 20.0);
	const int buttonPaddings = static_cast<int>(afxGlobalData.GetRibbonImageScale() * 4.0);

	CClientDC dc(this);
	CFont* pOldFont = dc.SelectObject(GetFont());
	int buttonWidth = max(dc.GetTextExtent(m_emptyButtonCaption).cx + buttonPaddings * 2, buttonMinWidth);
	dc.SelectObject(pOldFont);

	CRect rcClient;
	GetClientRect(rcClient);

	buttonWidth = min(rcClient.Width(), buttonWidth);
	const int buttonOffset = max((rcClient.Width() - buttonWidth) / 2, 0);
	m_emptyButton.MoveWindow(CRect(buttonOffset, buttonHeight, buttonOffset + buttonWidth, buttonHeight * 2));
}

bool CustomTreeCtrl::HasHighlightedChildren(HTREEITEM item)
{
	auto it = m_childrenHighligh.find(item);
	if (it != m_childrenHighligh.end())
		return it->second != 0;

	bool childHighlight = false;
	HTREEITEM childItem = GetNextItem(item, TVGN_CHILD);
	while (childItem)
	{
		if (HasHighlightedText(childItem) || HasHighlightedChildren(childItem))
		{
			childHighlight = true;
			break;
		}
		childItem = GetNextItem(childItem, TVGN_NEXT);
	}

	m_childrenHighligh[item] = childHighlight ? 1 : 0;
	return childHighlight;
}

bool CustomTreeCtrl::HasHighlightedText(HTREEITEM item)
{
	CString itemText = GetItemText(item);
	HighlightTextPainter::DefaultSplitter splitter(false);

	std::list<std::pair<size_t, bool>> parts;
	splitter.Split(itemText, m_highlightTokens, parts);

	return parts.size() > 1;
}

void CustomTreeCtrl::OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult)
{
	LPNMTVCUSTOMDRAW  pNMCD = reinterpret_cast<LPNMTVCUSTOMDRAW>(pNMHDR);
	switch (pNMCD->nmcd.dwDrawStage)
	{
	case CDDS_PREPAINT:
		*pResult = CDRF_NOTIFYITEMDRAW;
		return;
	case CDDS_ITEMPREPAINT:
	{
		CDC dc;
		dc.Attach(pNMCD->nmcd.hdc);
		const HTREEITEM currentItem = reinterpret_cast<HTREEITEM>(pNMCD->nmcd.dwItemSpec);

		COLORREF clrTextBk = pNMCD->clrTextBk;
		if (m_hoverItem == currentItem)
		{
			CRect rcItem;
			GetItemRect(currentItem, &rcItem, FALSE);

			clrTextBk = CDrawingManager::PixelAlpha(GetSysColor(COLOR_HIGHLIGHT), afxGlobalData.clrWindow, 15);
			dc.FillSolidRect(rcItem, clrTextBk);
		}

		dc.Detach();

		*pResult = CDRF_NOTIFYPOSTPAINT;
		break;
	}
	case CDDS_ITEMPOSTPAINT:
	{
		CDC dc;
		dc.Attach(pNMCD->nmcd.hdc);
		const HTREEITEM currentItem = reinterpret_cast<HTREEITEM>(pNMCD->nmcd.dwItemSpec);

		COLORREF clrTextBk = pNMCD->clrTextBk;
		if (m_hoverItem == currentItem)
		{
			clrTextBk = CDrawingManager::PixelAlpha(GetSysColor(COLOR_HIGHLIGHT), afxGlobalData.clrWindow, 15);
		}
		if (pNMCD->nmcd.uItemState & CDIS_SELECTED)
		{
			clrTextBk = GetSysColor(COLOR_HIGHLIGHT);
		}

		if (m_additionalTextCallback)
		{
			CString additionalText;
			m_additionalTextCallback(this, currentItem, additionalText);
			if (!additionalText.IsEmpty())
			{
				CSize textSize = dc.GetTextExtent(additionalText);
				HighlightTextPainter::DefaultSplitter splitter(false);
				HighlightTextPainter text(&splitter, additionalText, m_highlightTokens);
				CRect rcItem;
				if (GetItemRect(currentItem, &rcItem, FALSE))
				{
					CRect rcAdditionalText(rcItem);
					rcAdditionalText.left = rcAdditionalText.right - textSize.cx - static_cast<int>(afxGlobalData.GetRibbonImageScale() * 2.0);
					HighlightTextPainter::GDIPainter painter(pNMCD->nmcd.hdc, true);

					COLORREF oldTextColor = dc.SetTextColor(CDrawingManager::PixelAlpha(GetSysColor(COLOR_WINDOWTEXT), GetSysColor(COLOR_WINDOW), 50));
					text.Draw(&painter, static_cast<float>(rcAdditionalText.left), static_cast<float>(rcAdditionalText.top), static_cast<float>(rcAdditionalText.Width()), static_cast<float>(rcAdditionalText.Height()), pNMCD->nmcd.uItemState & CDIS_SELECTED);
					dc.SetTextColor(oldTextColor);
				}
			}
		}

		CString itemText = GetItemText(currentItem);

		HighlightTextPainter::DefaultSplitter splitter(false);
		HighlightTextPainter text(&splitter, itemText, m_highlightTokens);
		CRect rcLabel;
		if (GetItemRect(currentItem, &rcLabel, TRUE))
		{
			HighlightTextPainter::GDIPainter painter(pNMCD->nmcd.hdc);
			dc.FillSolidRect(rcLabel, clrTextBk);
			UINT itemState = GetItemState(currentItem, TVIS_EXPANDED);
			if ((itemState & TVIS_EXPANDED) == 0 && HasHighlightedChildren(currentItem))
			{
				CRect rcChildhighlight = rcLabel;
				rcChildhighlight.top = rcChildhighlight.bottom - 4;
				dc.FillSolidRect(rcChildhighlight, RGB(255, 255, 32));
			}

			dc.SetBkColor(clrTextBk);

			text.Draw(&painter, static_cast<float>(rcLabel.left), static_cast<float>(rcLabel.top), static_cast<float>(rcLabel.Width()), static_cast<float>(rcLabel.Height()), pNMCD->nmcd.uItemState & CDIS_SELECTED);

			if (pNMCD->nmcd.uItemState & CDIS_FOCUS)
			{
				dc.DrawFocusRect(rcLabel);
			}
		}

		dc.Detach();
		*pResult = CDRF_SKIPDEFAULT;
		break;
	}

	default:
		break;
	}
}

void CustomTreeCtrl::OnMouseLeave()
{
	UpdateHoverItem(nullptr);
}

void CustomTreeCtrl::OnButtonClicked()
{
	GetParent()->SendMessage(WM_COMMAND, MAKEWPARAM(GetDlgCtrlID(), BN_CLICKED), reinterpret_cast<LPARAM>(m_emptyButton.GetSafeHwnd()));
}

void CustomTreeCtrl::OnMouseMove(UINT nFlags, CPoint point)
{
	TRACKMOUSEEVENT trackmouseevent;
	trackmouseevent.cbSize = sizeof(trackmouseevent);
	trackmouseevent.dwFlags = TME_LEAVE;
	trackmouseevent.hwndTrack = GetSafeHwnd();
	TrackMouseEvent(&trackmouseevent);

	TVHITTESTINFO ti = {};
	ti.pt = point;
	HTREEITEM item = HitTest(&ti);

	UpdateHoverItem(item);

	CTreeCtrl::OnMouseMove(nFlags, point);
}


LRESULT CustomTreeCtrl::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_LBUTTONDOWN || message == WM_RBUTTONDOWN || message == WM_MBUTTONDOWN)
	{
		CPoint pt;
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);

		FullRowSelect(pt);
	}
	else if (message == WM_LBUTTONDBLCLK)
	{
		CPoint pt;
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);

		HTREEITEM item;
		bool onItemRight = false;
		if(FullRowDoubleClick(pt, item, onItemRight))
			return 0;
		else
		{
			if (item && onItemRight)
			{
				NMHDR nm;
				nm.code = NM_DBLCLK;
				nm.hwndFrom = GetSafeHwnd();
				nm.idFrom = GetDlgCtrlID();
				GetParent()->SendMessage(WM_NOTIFY, nm.idFrom, reinterpret_cast<LPARAM>(&nm));
				return 0;
			}
		}
	}
	else if (message == WM_PAINT)
	{
		if (!GetRootItem())
		{
			// The tree is empty
			CPaintDC dc(this);
			CRect rcClient;
			GetClientRect(rcClient);
			CFont* pOldFont = dc.SelectObject(GetFont());
			dc.FillSolidRect(rcClient, GetSysColor(COLOR_WINDOW));
			dc.DrawText(m_emptyMessage, rcClient, DT_CENTER | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
			dc.SelectObject(pOldFont);
			return 0;
		}
	}

	LRESULT result = CTreeCtrl::WindowProc(message, wParam, lParam);

	if (message == TVM_INSERTITEM || message == TVM_DELETEITEM)
	{
		UpdateButtonVisibility();
	}

	return result;
}

void CustomTreeCtrl::OnSize(UINT nType, int cx, int cy)
{
	CTreeCtrl::OnSize(nType, cx, cy);

	Invalidate();
	UpdateButtonPosition();
}


int CustomTreeCtrl::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (CTreeCtrl::OnCreate(lpCreateStruct) == -1)
		return -1;

	UpdateButtonVisibility();

	return 0;
}

void CustomTreeCtrl::OnSetFont(CFont* pFont, BOOL bRedraw)
{
	Default();

	if (m_emptyButton.GetSafeHwnd())
	{
		m_emptyButton.SetFont(pFont);
		if (bRedraw)
			Invalidate();
	}
}