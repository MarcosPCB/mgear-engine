
// mAudioDlg.h : header file
//

#pragma once
#include "afxcmn.h"
#include "afxwin.h"
#include "afxeditbrowsectrl.h"
#include <fmod.h>


// CmAudioDlg dialog
class CmAudioDlg : public CDialogEx
{
// Construction
public:
	CmAudioDlg(CWnd* pParent = NULL);	// standard constructor

// Dialog Data
	enum { IDD = IDD_MAUDIO_DIALOG };

	protected:
	virtual void DoDataExchange(CDataExchange* pDX);	// DDX/DDV support


// Implementation
protected:
	HICON m_hIcon;

	// Generated message map functions
	virtual BOOL OnInitDialog();
	afx_msg void OnPaint();
	afx_msg HCURSOR OnQueryDragIcon();
	DECLARE_MESSAGE_MAP()
public:
	FMOD_SYSTEM *snd_system;
	FMOD_CHANNEL *snd_channel;
	FMOD_SOUND *audio;

	CListCtrl listct;
	afx_msg void OnBnClickedButton10();
	CEdit IDView;
	CButton LoopControl;
	CComboBox SoundType;
	CEdit SoundPr;
	afx_msg void OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult);
	CListCtrl listm;
	CEdit IDmusic;
	CButton LoopMusic;
	afx_msg void OnEnChangeEdit1();
	afx_msg void OnBnClickedCheck2();
	afx_msg void OnCbnSelchangeCombo3();
	afx_msg void OnEnChangeEdit2();
	afx_msg void OnBnClickedButton11();
	afx_msg void OnBnClickedButton3();
	afx_msg void OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult);
	afx_msg void OnEnChangeEdit3();
	afx_msg void OnBnClickedCheck3();
	afx_msg void OnBnClickedButton7();
	CButton RemoveSND;
	CButton RemoveMSC;
	afx_msg void OnBnClickedButton6();
	afx_msg void OnBnClickedButton5();
	afx_msg void OnBnClickedButton4();
	CButton savelist;
	CButton PathView;
	CButton PathViewM;
	CButton OpenList;
	afx_msg void OnBnClickedButton1();
	afx_msg void OnBnClickedButton2();
	afx_msg void OnBnClickedButton8();
	afx_msg void OnBnClickedButton9();
	afx_msg void OnBnClickedButton12();
	afx_msg void OnBnClickedButton13();
	afx_msg void OnBnClickedButton14();
	CButton PlayAudio;
	CButton StopAudio;
	afx_msg void OnBnClickedButton15();
	CButton PlaySND;
	CButton StopSND;
	afx_msg void OnBnClickedButton16();
};
