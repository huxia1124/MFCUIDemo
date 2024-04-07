#include "pch.h"
#include "VectorEditBox.h"


IMPLEMENT_DYNAMIC(VectorEditBox, CEdit)

BEGIN_MESSAGE_MAP(VectorEditBox, CEdit)
	// ON_WM_INPUT()
	ON_WM_CREATE()
	ON_WM_NCPAINT()
	ON_WM_NCCALCSIZE()
END_MESSAGE_MAP()

VectorEditBox::VectorEditBox()
: m_nTextPadding(2)
, m_hasBorder(false)
{
}

VectorEditBox::~VectorEditBox()
{
}

void VectorEditBox::Redraw()
{
	Invalidate();
	SetWindowPos(nullptr, 0, 0, 0, 0,
		SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
}

bool VectorEditBox::EnablePreview(float minX, float maxX, float minY, float maxY)
{
	m_minX = minX;
	m_minY = minY;
	m_maxX = maxX;
	m_maxY = maxY;

	return IsPreviewEnabled();
}

bool VectorEditBox::IsPreviewEnabled() const
{
	return m_minX < m_maxX && m_minY < m_maxY;
}

BOOL VectorEditBox::PreCreateWindow(CREATESTRUCT& cs)
{
	m_hasBorder = (cs.style & WS_BORDER) != 0;
	cs.style &= ~WS_BORDER;

	return TRUE;
}

int VectorEditBox::OnCreate(LPCREATESTRUCT lpCreateStruct)
{
	if (__super::OnCreate(lpCreateStruct) == -1)
		return -1;

	return 0;
}

void VectorEditBox::OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp)
{
	CEdit::OnNcCalcSize(bCalcValidRects, lpncsp);


	if (m_hasBorder)
	{
		lpncsp->rgrc[0].top += 1;
		lpncsp->rgrc[0].bottom -= 1;
		lpncsp->rgrc[0].left += 1;
		lpncsp->rgrc[0].right -= 1;
	}

	lpncsp->rgrc[0].right -= m_nTextPadding;
	lpncsp->rgrc[0].left += m_nTextPadding;

	const int minHeight = static_cast<int>(afxGlobalData.GetRibbonImageScale() * 16.0);
	const int bottomSpace = (lpncsp->rgrc[0].bottom - lpncsp->rgrc[0].top) > minHeight ? (lpncsp->rgrc[0].bottom - lpncsp->rgrc[0].top - minHeight) : 0;
	lpncsp->rgrc[0].bottom -= bottomSpace;

	if (IsPreviewEnabled())
	{
		CRect rcClient;
		GetClientRect(rcClient);
		lpncsp->rgrc[0].right -= rcClient.Height() + bottomSpace;
	}
}

void VectorEditBox::OnNcPaint()
{
	CEdit::OnNcPaint();

	CWindowDC dc(this);
	CFont* pOldFont = dc.SelectObject(GetFont());
	CRect rcWindow;
	GetWindowRect(&rcWindow);
	rcWindow.OffsetRect(-rcWindow.left, -rcWindow.top);

	CRect rcBorder = rcWindow;
	if (m_hasBorder)
	{
		rcWindow.DeflateRect(1, 1);
	}

	bool preview = IsPreviewEnabled();
	if (preview)
	{
		CRect rcPreview = rcWindow;
		rcPreview.left = rcPreview.right - rcPreview.Height();
		dc.FillSolidRect(rcPreview, CDrawingManager::PixelAlpha(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_WINDOWTEXT), 95));

		rcWindow.right -= rcWindow.Height();
	}

	CString text;
	GetWindowText(text);
	int start = 0;
	int end = 0;
	CRect rcColorBar = rcWindow;
	const int minHeight = static_cast<int>(afxGlobalData.GetRibbonImageScale() * 16.0);
	rcColorBar.top = minHeight;

	if (m_nTextPadding)
	{
		dc.FillSolidRect(CRect(rcWindow.left, rcWindow.top, rcWindow.left + m_nTextPadding, rcWindow.bottom), GetSysColor(COLOR_WINDOW));
		dc.FillSolidRect(CRect(rcWindow.right - m_nTextPadding, rcWindow.top, rcWindow.right, rcWindow.bottom), GetSysColor(COLOR_WINDOW));
	}

	if (rcColorBar.IsRectEmpty())
		return;

	dc.FillSolidRect(rcColorBar, CDrawingManager::PixelAlpha(GetSysColor(COLOR_WINDOW), GetSysColor(COLOR_WINDOWTEXT), 95));

	rcColorBar.left += m_nTextPadding;
	rcColorBar.right -= m_nTextPadding;

	COLORREF colors[] = { RGB(255, 168, 168) ,RGB(168, 224, 168) ,RGB(168, 168, 255), RGB(168, 168, 168) };
	LPCTSTR axisNames[] = { _T("X"), _T("Y"), _T("Z"), _T("W") };

	int colorIdx = 0;
	while (true)
	{
		if ((end = text.Find(_T(','), start)) == -1)
		{
			end = text.GetLength();
		}

		CSize sz = dc.GetTextExtent(static_cast<LPCTSTR>(text) + start, end - start);
		start = end;

		while (end < text.GetLength() && !isdigit(text[end]))
			++end;

		CSize szSpace = dc.GetTextExtent(static_cast<LPCTSTR>(text) + start, end - start);
		
		rcColorBar.right = rcColorBar.left + sz.cx;
		dc.FillSolidRect(rcColorBar, colors[colorIdx]);

		if (rcColorBar.Height() >= static_cast<int>(afxGlobalData.GetRibbonImageScale() * 12.0))
		{
			dc.DrawText(axisNames[colorIdx], rcColorBar, DT_CENTER | DT_TOP | DT_NOPREFIX | DT_SINGLELINE);
		}

		if (end >= text.GetLength())
			break;

		rcColorBar.OffsetRect(rcColorBar.Width() + szSpace.cx, 0);

		colorIdx++;
		colorIdx = colorIdx % 4;
		start = end;
	}

	if (m_hasBorder)
	{
		CBrush brush(GetGlobalData()->clrWindowText);
		dc.FrameRect(&rcBorder, &brush);
	}

	dc.SelectObject(pOldFont);
}

LRESULT VectorEditBox::WindowProc(UINT message, WPARAM wParam, LPARAM lParam)
{
	if (message == WM_LBUTTONDBLCLK)
	{
		// Override the double-click behavior
		// to make sure only the digits and dot are selected.
		// If we don't do this, the separator (comma) would be selected which is annoying

		CPoint pt;
		pt.x = GET_X_LPARAM(lParam);
		pt.y = GET_Y_LPARAM(lParam);

		// See where in the string we double-clicked
		int charIdx = CharFromPos(pt);

		CString text;
		GetWindowText(text);

		// find the start and the end of this floating number
		int start = charIdx;
		while (start > 0 && !IsWhitespace(text[start]))
			start--;
		if (IsWhitespace(text[start]))
			start++;

		int end = charIdx;
		while (end < text.GetLength() && !IsWhitespace(text[end]))
			end++;

		// Select the floating number
		SetSel(start, end);
		return 0;
	}

	LRESULT result = CEdit::WindowProc(message, wParam, lParam);

	if (message == WM_CHAR)
	{
		// Redraw non-client area, to make sure the axis indicators are updated
		SetWindowPos(nullptr, 0, 0, 0, 0,
			SWP_DRAWFRAME | SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_NOZORDER);
	}

	return result;
}

bool VectorEditBox::IsWhitespace(TCHAR ch)
{
	return ch == _T(' ') || ch == _T(',');;
}
