#include "pch.h"
#include "CustomTreeCtrl.h"
#include "HighlightTextPainter.h"

BEGIN_MESSAGE_MAP(CustomTreeCtrl, CTreeCtrl)
	ON_NOTIFY_REFLECT(NM_CUSTOMDRAW, &CustomTreeCtrl::OnNMCustomdraw)
	ON_WM_MOUSELEAVE()
	ON_WM_MOUSEMOVE()
	ON_WM_SIZE()
	ON_BN_CLICKED(700, OnButtonClicked)
END_MESSAGE_MAP()


void CustomTreeCtrl::SetHighlightTokens(const CString& tokens)
{
	m_highlightVer++;
	m_childrenHighligh.clear();

	m_highlightTokens = tokens;
	if (GetSafeHwnd())
		Invalidate();
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

void CustomTreeCtrl::FullRowDoubleClick(const CPoint& pt)
{
	TVHITTESTINFO ti = {};
	ti.pt = pt;
	HTREEITEM item = HitTest(&ti);

	CString itemText = GetItemText(item);

	if (item && (ti.flags & (TVHT_ONITEMINDENT | TVHT_ONITEMRIGHT | TVHT_ONITEMLABEL)))
		Expand(item, TVE_TOGGLE);
}

void CustomTreeCtrl::UpdateButtonVisibility()
{
	if (GetRootItem())
	{
		m_emptyButton.DestroyWindow();
	}
	else
	{
		if (!m_emptyButton.GetSafeHwnd())
		{
			m_emptyButton.Create(_T("Add Nodes"), WS_VISIBLE | WS_CHILD | BS_PUSHBUTTON, CRect(), this, 700);
		}

		UpdateButtonPosition();
	}
}

void CustomTreeCtrl::UpdateButtonPosition()
{
	if (!m_emptyButton.GetSafeHwnd())
		return;

	const int buttonMaxWidth = static_cast<int>(afxGlobalData.GetRibbonImageScale() * 120.0);
	const int buttonHeight = static_cast<int>(afxGlobalData.GetRibbonImageScale() * 18.0);
	CRect rcClient;
	GetClientRect(rcClient);

	const int buttonWidth = min(rcClient.Width(), buttonMaxWidth);
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

			text.Draw(&painter, rcLabel.left, rcLabel.top, rcLabel.right - rcLabel.left, rcLabel.bottom - rcLabel.top, pNMCD->nmcd.uItemState & CDIS_SELECTED);

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

		FullRowDoubleClick(pt);
		return 0;
	}
	else if (message == WM_PAINT)
	{
		if (!GetRootItem())
		{
			CPaintDC dc(this); // device context for painting
			CRect rcClient;
			GetClientRect(rcClient);
			dc.FillSolidRect(rcClient, GetSysColor(COLOR_WINDOW));
			dc.DrawText(_T("Nothing here"), rcClient, DT_CENTER | DT_TOP | DT_SINGLELINE | DT_NOPREFIX);
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
