#pragma once
#include <afxwin.h>
#include <vector>

class VectorEditBox : public CEdit
{
	DECLARE_DYNAMIC(VectorEditBox)

public:
	VectorEditBox();
	virtual ~VectorEditBox();

	void Redraw();
	bool EnablePreview(float minX, float maxX, float minY, float maxY);

private:
	int			m_nTextPadding;
	bool		m_hasBorder;
	float	m_minX = 0.0f;
	float	m_minY = 0.0f;
	float	m_maxX = 0.0f;
	float	m_maxY = 0.0f;
	std::vector<float> m_values;

	bool IsPreviewEnabled() const;


	virtual BOOL PreCreateWindow(CREATESTRUCT& cs);

	DECLARE_MESSAGE_MAP()


	afx_msg void OnNcCalcSize(BOOL bCalcValidRects, NCCALCSIZE_PARAMS FAR* lpncsp);
	afx_msg void OnNcPaint();
	afx_msg int OnCreate(LPCREATESTRUCT lpCreateStruct);

	virtual LRESULT WindowProc(UINT message, WPARAM wParam, LPARAM lParam);
	static bool IsWhitespace(TCHAR ch);
};

