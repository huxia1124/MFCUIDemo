#pragma once
#include <afxcmn.h>
#include <functional>
#include <map>

class CustomTreeCtrl : public CTreeCtrl
{
public:
	void SetHighlightTokens(const CString& tokens);
	void SetAdditionalTextCallback(std::function<void(CustomTreeCtrl*, HTREEITEM, CString&)> callback);

protected:

	void UpdateHoverItem(HTREEITEM item);
	void FullRowSelect(const CPoint& pt);
	void FullRowDoubleClick(const CPoint& pt);
	void UpdateButtonVisibility();
	void UpdateButtonPosition();

	virtual bool HasHighlightedChildren(HTREEITEM item);
	virtual bool HasHighlightedText(HTREEITEM item);
	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);

	DECLARE_MESSAGE_MAP()
	afx_msg void OnNMCustomdraw(NMHDR* pNMHDR, LRESULT* pResult);
	afx_msg void OnMouseLeave();
	afx_msg void OnButtonClicked();
	afx_msg void OnMouseMove(UINT nFlags, CPoint point);
	afx_msg void OnSize(UINT nType, int cx, int cy);

private:
	CButton m_emptyButton;
	CString m_highlightTokens;
	HTREEITEM m_hoverItem = nullptr;
	uint64_t m_highlightVer = 0;
	std::map<HTREEITEM, uint64_t> m_childrenHighligh;
	std::function<void(CustomTreeCtrl*, HTREEITEM, CString&)> m_additionalTextCallback;
};

