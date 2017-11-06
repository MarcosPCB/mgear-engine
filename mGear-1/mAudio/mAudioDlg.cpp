
// mAudioDlg.cpp : implementation file
//

#include "stdafx.h"
#include "mAudio.h"
#include "mAudioDlg.h"
#include "afxdialogex.h"

#include <fmod.h>
#include <fmod_errors.h>

#ifdef _DEBUG
#define new DEBUG_NEW
#endif


// CmAudioDlg dialog



CmAudioDlg::CmAudioDlg(CWnd* pParent /*=NULL*/)
	: CDialogEx(CmAudioDlg::IDD, pParent)
{
	m_hIcon = AfxGetApp()->LoadIcon(IDR_MAINFRAME);
}

void CmAudioDlg::DoDataExchange(CDataExchange* pDX)
{
	CDialogEx::DoDataExchange(pDX);

	DDX_Control(pDX, IDC_LIST2, listct);
	DDX_Control(pDX, IDC_EDIT1, IDView);
	DDX_Control(pDX, IDC_CHECK2, LoopControl);
	DDX_Control(pDX, IDC_COMBO3, SoundType);
	DDX_Control(pDX, IDC_EDIT2, SoundPr);
	DDX_Control(pDX, IDC_LIST1, listm);
	DDX_Control(pDX, IDC_EDIT3, IDmusic);
	DDX_Control(pDX, IDC_CHECK3, LoopMusic);
	DDX_Control(pDX, IDC_BUTTON11, RemoveSND);
	DDX_Control(pDX, IDC_BUTTON7, RemoveMSC);
	DDX_Control(pDX, IDC_BUTTON4, savelist);
	DDX_Control(pDX, IDC_BUTTON2, PathView);
	DDX_Control(pDX, IDC_BUTTON8, PathViewM);
	DDX_Control(pDX, IDC_BUTTON1, OpenList);
	DDX_Control(pDX, IDC_BUTTON13, PlayAudio);
	DDX_Control(pDX, IDC_BUTTON14, StopAudio);
	DDX_Control(pDX, IDC_BUTTON15, PlaySND);
	DDX_Control(pDX, IDC_BUTTON16, StopSND);
}

BEGIN_MESSAGE_MAP(CmAudioDlg, CDialogEx)
	ON_WM_PAINT()
	ON_WM_QUERYDRAGICON()
	ON_BN_CLICKED(IDC_BUTTON10, &CmAudioDlg::OnBnClickedButton10)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST2, &CmAudioDlg::OnLvnItemchangedList2)
	ON_EN_CHANGE(IDC_EDIT1, &CmAudioDlg::OnEnChangeEdit1)
	ON_BN_CLICKED(IDC_CHECK2, &CmAudioDlg::OnBnClickedCheck2)
	ON_CBN_SELCHANGE(IDC_COMBO3, &CmAudioDlg::OnCbnSelchangeCombo3)
	ON_EN_CHANGE(IDC_EDIT2, &CmAudioDlg::OnEnChangeEdit2)
	ON_BN_CLICKED(IDC_BUTTON11, &CmAudioDlg::OnBnClickedButton11)
	ON_BN_CLICKED(IDC_BUTTON3, &CmAudioDlg::OnBnClickedButton3)
	ON_NOTIFY(LVN_ITEMCHANGED, IDC_LIST1, &CmAudioDlg::OnLvnItemchangedList1)
	ON_EN_CHANGE(IDC_EDIT3, &CmAudioDlg::OnEnChangeEdit3)
	ON_BN_CLICKED(IDC_CHECK3, &CmAudioDlg::OnBnClickedCheck3)
	ON_BN_CLICKED(IDC_BUTTON7, &CmAudioDlg::OnBnClickedButton7)
	ON_BN_CLICKED(IDC_BUTTON6, &CmAudioDlg::OnBnClickedButton6)
	ON_BN_CLICKED(IDC_BUTTON5, &CmAudioDlg::OnBnClickedButton5)
	ON_BN_CLICKED(IDC_BUTTON4, &CmAudioDlg::OnBnClickedButton4)
	ON_BN_CLICKED(IDC_BUTTON1, &CmAudioDlg::OnBnClickedButton1)
	ON_BN_CLICKED(IDC_BUTTON2, &CmAudioDlg::OnBnClickedButton2)
	ON_BN_CLICKED(IDC_BUTTON8, &CmAudioDlg::OnBnClickedButton8)
	ON_BN_CLICKED(IDC_BUTTON9, &CmAudioDlg::OnBnClickedButton9)
	ON_BN_CLICKED(IDC_BUTTON12, &CmAudioDlg::OnBnClickedButton12)
	ON_BN_CLICKED(IDC_BUTTON13, &CmAudioDlg::OnBnClickedButton13)
	ON_BN_CLICKED(IDC_BUTTON14, &CmAudioDlg::OnBnClickedButton14)
	ON_BN_CLICKED(IDC_BUTTON15, &CmAudioDlg::OnBnClickedButton15)
	ON_BN_CLICKED(IDC_BUTTON16, &CmAudioDlg::OnBnClickedButton16)
END_MESSAGE_MAP()


// CmAudioDlg message handlers

BOOL CmAudioDlg::OnInitDialog()
{
	CDialogEx::OnInitDialog();

	FMOD_RESULT re;

	CString report;

	if ((re = FMOD_System_Create(&snd_system)) != FMOD_OK)
	{
		report.Format(_T("%s"), FMOD_ErrorString(re));
		MessageBox(report, L"Error", MB_OK);
	}

	if ((re = FMOD_System_Init(snd_system, 1, FMOD_INIT_NORMAL, NULL)) != FMOD_OK)
	{
		report.Format(_T("%s"), FMOD_ErrorString(re));
		MessageBox(report, L"Error", MB_OK);
	}

	memset(&AList, 0, sizeof(struct AudioList));

	listct.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	listct.InsertColumn(0, L"Sound");
	listct.SetColumnWidth(0, 80);

	listct.InsertColumn(1, L"ID");
	listct.SetColumnWidth(1, 40);

	listct.InsertColumn(2, L"Path");
	listct.SetColumnWidth(2, 280);

	listct.InsertColumn(3, L"Loop?");
	listct.SetColumnWidth(3, 60);

	listct.InsertColumn(4, L"Type");
	listct.SetColumnWidth(4, 100);

	listct.InsertColumn(5, L"Priority");
	listct.SetColumnWidth(5, 80);

	listm.SetExtendedStyle(LVS_EX_FULLROWSELECT | LVS_EX_CHECKBOXES);

	listm.InsertColumn(0, L"Music");
	listm.SetColumnWidth(0, 80);

	listm.InsertColumn(1, L"ID");
	listm.SetColumnWidth(1, 40);

	listm.InsertColumn(2, L"Path");
	listm.SetColumnWidth(2, 280);

	listm.InsertColumn(3, L"Loop?");
	listm.SetColumnWidth(3, 60);

	IDView.EnableWindow(0);
	PathView.EnableWindow(0);
	LoopControl.EnableWindow(0);
	SoundType.EnableWindow(0);
	SoundPr.EnableWindow(0);
	RemoveSND.EnableWindow(0);

	IDmusic.EnableWindow(0);
	PathViewM.EnableWindow(0);
	LoopMusic.EnableWindow(0);
	RemoveMSC.EnableWindow(0);

	savelist.EnableWindow(0);

	// Set the icon for this dialog.  The framework does this automatically
	//  when the application's main window is not a dialog
	SetIcon(m_hIcon, TRUE);			// Set big icon
	SetIcon(m_hIcon, FALSE);		// Set small icon

	// TODO: Add extra initialization here

	return TRUE;  // return TRUE  unless you set the focus to a control
}

// If you add a minimize button to your dialog, you will need the code below
//  to draw the icon.  For MFC applications using the document/view model,
//  this is automatically done for you by the framework.

void CmAudioDlg::OnPaint()
{
	if (IsIconic())
	{
		CPaintDC dc(this); // device context for painting

		SendMessage(WM_ICONERASEBKGND, reinterpret_cast<WPARAM>(dc.GetSafeHdc()), 0);

		// Center icon in client rectangle
		int cxIcon = GetSystemMetrics(SM_CXICON);
		int cyIcon = GetSystemMetrics(SM_CYICON);
		CRect rect;
		GetClientRect(&rect);
		int x = (rect.Width() - cxIcon + 1) / 2;
		int y = (rect.Height() - cyIcon + 1) / 2;

		// Draw the icon
		dc.DrawIcon(x, y, m_hIcon);
	}
	else
	{
		CDialogEx::OnPaint();
	}
}

// The system calls this function to obtain the cursor to display while the user drags
//  the minimized window.
HCURSOR CmAudioDlg::OnQueryDragIcon()
{
	return static_cast<HCURSOR>(m_hIcon);
}



void CmAudioDlg::OnBnClickedButton10()
{
	// TODO: Add your control notification handler code here

	int id;
	CString str;

	id=listct.InsertItem(AList.num_sounds, L"Sound");
	
	str.Format(_T("%d"), AList.num_sounds);
	listct.SetItemText(id, 1, str.GetString());
	//AList.sound[AList.num_sounds].ID = AList.num_sounds;

	str.SetString(L".");
	listct.SetItemText(id, 2, str.GetString());

	listct.SetItemText(id, 3, L"No");

	listct.SetItemText(id, 4, L"FX");

	listct.SetItemText(id, 5, L"0");

	AList.num_sounds++;

	AList.changes = 1;

	savelist.EnableWindow(1);
}


void CmAudioDlg::OnLvnItemchangedList2(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;

	LPNMITEMACTIVATE temp = (LPNMITEMACTIVATE)pNMHDR;

	CString str;

	AList.multiplesndselection = 0;
	for (int i = 0, j = 0; i < listct.GetItemCount(); i++)
	{
		if (listct.GetCheck(i))
		{
			AList.selectedsnd[j] = i;
			AList.multiplesndselection++;
			j++;
		}
	}

	if (AList.multiplesndselection > 0)
	{
		SoundSelected = -2;
		AudioSelected = -1;

		IDView.EnableWindow(0);
		PathView.EnableWindow(0);
		LoopControl.EnableWindow(1);
		SoundType.EnableWindow(1);
		SoundPr.EnableWindow(1);
		RemoveSND.EnableWindow(1);
		PlaySND.EnableWindow(0);
		PlayAudio.EnableWindow(0);
		StopAudio.EnableWindow(0);
		StopSND.EnableWindow(0);

		LoopControl.SetCheck(0);
		SoundType.SetCurSel(5);
		SoundPr.SetWindowTextW(L"?");
	}
	else
	{
		SoundSelected = temp->iItem;

		AudioSelected = 0;

		IDView.EnableWindow(1);
		PathView.EnableWindow(1);
		LoopControl.EnableWindow(1);
		SoundType.EnableWindow(1);
		SoundPr.EnableWindow(1);
		RemoveSND.EnableWindow(1);
		PlayAudio.EnableWindow(1);
		PlaySND.EnableWindow(1);
		StopAudio.EnableWindow(1);
		StopSND.EnableWindow(1);

		//str.Format(_T("%d"), temp->iItem);

		//str.Format(_T("%d"), AList.sound[temp->iItem].ID);
		IDView.SetWindowTextW(listct.GetItemText(SoundSelected, 1));

		//str.Format(_T("%s"), AList.sound[temp->iItem].file);
		//PathView.SetWindowTextW(str.GetString());

		str = listct.GetItemText(SoundSelected, 3);

		if (str.Compare(L"Yes"))
			LoopControl.SetCheck(0);
		else
			LoopControl.SetCheck(1);

		str = listct.GetItemText(SoundSelected, 4);

		if (!str.Compare(L"AMBIENT"))
			SoundType.SetCurSel(0);

		if (!str.Compare(L"FX"))
			SoundType.SetCurSel(1);

		if (!str.Compare(L"PLAYER"))
			SoundType.SetCurSel(2);

		if (!str.Compare(L"NPC"))
			SoundType.SetCurSel(3);

		if (!str.Compare(L"TALK"))
			SoundType.SetCurSel(4);

		SoundPr.SetWindowTextW(listct.GetItemText(SoundSelected, 5));
	}
}


void CmAudioDlg::OnEnChangeEdit1()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here

	CString str;

	USES_CONVERSION;

	if (SoundSelected > -1)
	{
		IDView.GetWindowTextW(str);

		//AList.sound[SoundSelected].ID = atoi(W2A(str.GetBuffer(str.GetLength())));

		listct.SetItemText(SoundSelected, 1, str.GetString());

		AList.changes = 1;

		savelist.EnableWindow(1);
	}
}

void CmAudioDlg::OnBnClickedCheck2()
{
	// TODO: Add your control notification handler code here

	if (SoundSelected > -1)
	{
		//AList.sound[SoundSelected].loop = LoopControl.GetCheck();

		if (LoopControl.GetCheck())
			listct.SetItemText(SoundSelected, 3, L"Yes");
		else
			listct.SetItemText(SoundSelected, 3, L"No");

		AList.changes = 1;

		savelist.EnableWindow(1);
	}
	
	if (SoundSelected == -2)
	{
		for (int i = 0; i < AList.multiplesndselection; i++)
		{
			if (LoopControl.GetCheck())
				listct.SetItemText(AList.selectedsnd[i], 3, L"Yes");
			else
				listct.SetItemText(AList.selectedsnd[i], 3, L"No");
		}

		AList.changes = 1;

		savelist.EnableWindow(1);
	}
}


void CmAudioDlg::OnCbnSelchangeCombo3()
{
	// TODO: Add your control notification handler code here

	CString str;

	if (SoundSelected > -1)
	{
		//AList.sound[SoundSelected].type = SoundType.GetCurSel();

		switch (SoundType.GetCurSel())
		{
		case 0:
			str = L"AMBIENT";
			break;

		case 1:
			str = L"FX";
			break;

		case 2:
			str = L"PLAYER";
			break;

		case 3:
			str = L"NPC";
			break;

		case 4:
			str = L"TALK";
			break;
		}

		listct.SetItemText(SoundSelected, 4, str.GetString());

		AList.changes = 1;

		savelist.EnableWindow(1);
	}

	if (SoundSelected == -2)
	{
		for (int i = 0; i < AList.multiplesndselection; i++)
		{
			switch (SoundType.GetCurSel())
			{
			case 0:
				str = L"AMBIENT";
				break;

			case 1:
				str = L"FX";
				break;

			case 2:
				str = L"PLAYER";
				break;

			case 3:
				str = L"NPC";
				break;

			case 4:
				str = L"TALK";
				break;
			}

			listct.SetItemText(AList.selectedsnd[i], 4, str.GetString());

			AList.changes = 1;

			savelist.EnableWindow(1);
		}
	}
}


void CmAudioDlg::OnEnChangeEdit2()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here

	CString str;

	USES_CONVERSION;

	if (SoundSelected > -1)
	{
		SoundPr.GetWindowTextW(str);

		//AList.sound[SoundSelected].priority = atoi(W2A(str.GetBuffer(str.GetLength())));

		listct.SetItemText(SoundSelected, 5, str.GetString());

		AList.changes = 1;

		savelist.EnableWindow(1);
	}

	if (SoundSelected == -2)
	{
		for (int i = 0; i < AList.multiplesndselection; i++)
		{
			SoundPr.GetWindowTextW(str);

			listct.SetItemText(AList.selectedsnd[i], 5, str.GetString());

			AList.changes = 1;

			savelist.EnableWindow(1);
		}
	}
}


void CmAudioDlg::OnBnClickedButton11()
{
	// TODO: Add your control notification handler code here

	if (SoundSelected > -1)
	{
		listct.DeleteItem(SoundSelected);
		SoundSelected = -1;
		AudioSelected = -1;
		AList.num_sounds--;

		IDView.EnableWindow(0);
		PathView.EnableWindow(0);
		LoopControl.EnableWindow(0);
		SoundType.EnableWindow(0);
		SoundPr.EnableWindow(0);
		RemoveSND.EnableWindow(0);
		PlayAudio.EnableWindow(0);
		PlaySND.EnableWindow(0);
		StopAudio.EnableWindow(0);
		StopSND.EnableWindow(0);

		AList.changes = 1;

		savelist.EnableWindow(1);
	}
}


void CmAudioDlg::OnBnClickedButton3()
{
	// TODO: Add your control notification handler code here

	CString str;

	int id;

	id = listm.InsertItem(AList.num_musics, L"Music");

	str.Format(_T("%d"), AList.num_musics);
	listm.SetItemText(id, 1, str.GetString());
	//AList.sound[AList.num_sounds].ID = AList.num_sounds;

	str.SetString(L".");
	listm.SetItemText(id, 2, str.GetString());

	listm.SetItemText(id, 3, L"No");

	AList.num_musics++;

	AList.changes = 1;

	savelist.EnableWindow(1);
}


void CmAudioDlg::OnLvnItemchangedList1(NMHDR *pNMHDR, LRESULT *pResult)
{
	LPNMLISTVIEW pNMLV = reinterpret_cast<LPNMLISTVIEW>(pNMHDR);
	// TODO: Add your control notification handler code here
	*pResult = 0;


	LPNMITEMACTIVATE temp = (LPNMITEMACTIVATE)pNMHDR;

	CString str;

	AList.multiplemusselection = 0;
	for (int i = 0, j = 0; i < listm.GetItemCount(); i++)
	{
		if (listm.GetCheck(i))
		{
			AList.selectedmus[j] = i;
			AList.multiplemusselection++;
			j++;
		}
	}

	if (AList.multiplemusselection > 0)
	{
		MusicSelected = -2;
		AudioSelected = -1;

		IDmusic.EnableWindow(0);
		PathViewM.EnableWindow(0);
		LoopMusic.EnableWindow(1);
		RemoveMSC.EnableWindow(1);
		PlayAudio.EnableWindow(0);
		PlaySND.EnableWindow(0);
		StopAudio.EnableWindow(0);
		StopSND.EnableWindow(0);

		LoopMusic.SetCheck(0);
	}
	else
	{
		MusicSelected = temp->iItem;

		AudioSelected = 1;

		IDmusic.EnableWindow(1);
		PathViewM.EnableWindow(1);
		LoopMusic.EnableWindow(1);
		RemoveMSC.EnableWindow(1);
		PlayAudio.EnableWindow(1);
		PlaySND.EnableWindow(1);
		StopAudio.EnableWindow(1);
		StopSND.EnableWindow(1);

		IDmusic.SetWindowTextW(listm.GetItemText(MusicSelected, 1));

		str = listm.GetItemText(MusicSelected, 3);

		if (str.Compare(L"Yes"))
			LoopMusic.SetCheck(0);
		else
			LoopMusic.SetCheck(1);
	}
}


void CmAudioDlg::OnEnChangeEdit3()
{
	// TODO:  If this is a RICHEDIT control, the control will not
	// send this notification unless you override the CDialogEx::OnInitDialog()
	// function and call CRichEditCtrl().SetEventMask()
	// with the ENM_CHANGE flag ORed into the mask.

	// TODO:  Add your control notification handler code here

	CString str;

	USES_CONVERSION;

	if (MusicSelected > -1)
	{
		IDmusic.GetWindowTextW(str);

		//AList.sound[SoundSelected].ID = atoi(W2A(str.GetBuffer(str.GetLength())));

		listm.SetItemText(MusicSelected, 1, str.GetString());

		AList.changes = 1;

		savelist.EnableWindow(1);
	}
}


void CmAudioDlg::OnBnClickedCheck3()
{
	// TODO: Add your control notification handler code here

	if (MusicSelected > -1)
	{
		if (LoopMusic.GetCheck())
			listm.SetItemText(MusicSelected, 3, L"Yes");
		else
			listm.SetItemText(MusicSelected, 3, L"No");

		AList.changes = 1;

		savelist.EnableWindow(1);
	}

	if (MusicSelected == -2)
	{
		for (int i = 0; i < AList.multiplemusselection; i++)
		{
			if (LoopMusic.GetCheck())
				listm.SetItemText(AList.selectedmus[i], 3, L"Yes");
			else
				listm.SetItemText(AList.selectedmus[i], 3, L"No");

			AList.changes = 1;

			savelist.EnableWindow(1);
		}
	}
}


void CmAudioDlg::OnBnClickedButton7()
{
	// TODO: Add your control notification handler code here

	if (MusicSelected > -1)
	{
		listm.DeleteItem(MusicSelected);
		MusicSelected = -1;
		AudioSelected = -1;
		AList.num_musics--;

		IDmusic.EnableWindow(0);
		PathViewM.EnableWindow(0);
		LoopMusic.EnableWindow(0);
		RemoveMSC.EnableWindow(0);
		PlayAudio.EnableWindow(0);
		PlaySND.EnableWindow(0);
		StopAudio.EnableWindow(0);
		StopSND.EnableWindow(0);

		AList.changes = 1;

		savelist.EnableWindow(1);
	}
}


void CmAudioDlg::OnBnClickedButton6()
{
	// TODO: Add your control notification handler code here

	if (MessageBox(L"Are you sure you want to create a new sound list?\nAll unsaved content will be deleted.", L"New Sound List", MB_YESNO) == IDYES)
	{
		listct.DeleteAllItems();
		listm.DeleteAllItems();

		IDView.EnableWindow(0);
		PathView.EnableWindow(0);
		LoopControl.EnableWindow(0);
		SoundType.EnableWindow(0);
		SoundPr.EnableWindow(0);
		RemoveSND.EnableWindow(0);
		PlayAudio.EnableWindow(0);
		PlaySND.EnableWindow(0);
		StopAudio.EnableWindow(0);
		StopSND.EnableWindow(0);

		IDmusic.EnableWindow(0);
		PathViewM.EnableWindow(0);
		LoopMusic.EnableWindow(0);
		RemoveMSC.EnableWindow(0);

		memset(&AList, 0, sizeof(struct AudioList));

		AList.changes = 0;

		savelist.EnableWindow(0);
	}
}


void CmAudioDlg::OnBnClickedButton5()
{
	// TODO: Add your control notification handler code here

	USES_CONVERSION;

	FILE *f;
	char *id, *path, loop[8], *type, *pr, *pathn;

	const char headercomments[] = { "//List of sounds in game\n//SOUND id path LOOP/NOLOOP AMBIENT/FX/PLAYER/NPC/TALK PRIORITY (0 - 50) (most important to least important)" };

	const char musiccomments[] = { "\n//List of musics\n//MUSIC id path LOOP/NOLOOP" };

	int proceed = 0;

	CString str;

	CFileDialog saveas(FALSE, _T(".list"), L"sound", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("List files|*.list||"));

	if (saveas.DoModal() == IDOK)
	{
		if ((f = fopen(W2A(saveas.GetPathName()), "w")) == NULL)
		{
			MessageBox(L"Could not save list file", L"Error", MB_OK);
			proceed = 1;
		}

		if (!proceed)
		{
			fprintf(f, "%s\n\n", headercomments);

			for (int i = 0; i < AList.num_sounds; i++)
			{
				id = W2A(CmAudioDlg::listct.GetItemText(i, 1));
				path = W2A(listct.GetItemText(i, 2));
				str = listct.GetItemText(i, 3);

				if (!str.Compare(L"Yes"))
					strcpy(loop, "LOOP");
				else
					strcpy(loop, "NOLOOP");

				type = W2A(listct.GetItemText(i, 4));
				pr = W2A(listct.GetItemText(i, 5));
				fprintf(f, "SOUND %s \"%s\" %s %s %s \n", id, path, loop, type, pr);
			}

			fprintf(f, "%s\n\n", musiccomments);

			for (int i = 0; i < AList.num_musics; i++)
			{
				id = W2A(listm.GetItemText(i, 1));
				path = W2A(listm.GetItemText(i, 2));
				str = listm.GetItemText(i, 3);

				if (!str.Compare(L"Yes"))
					strcpy(loop, "LOOP");
				else
					strcpy(loop, "NOLOOP");

				fprintf(f, "MUSIC %s \"%s\" %s \n", id, path, loop);
			}

			fclose(f);

			strcpy(AList.file, W2A(saveas.GetPathName()));
			AList.issaved = 1;
			AList.changes = 0;

			savelist.EnableWindow(0);

			CString Wtext;

			Wtext.Format(_T("mAudio - %s"), saveas.GetPathName());

			SetWindowText(Wtext.GetString());
		}
	}
}


void CmAudioDlg::OnBnClickedButton4()
{
	// TODO: Add your control notification handler code here

	USES_CONVERSION;

	FILE *f = NULL;
	char *id, *path, loop[8], *type, *pr, *pathn;

	const char headercomments[] = { "//List of sounds in game\n//SOUND id path LOOP/NOLOOP AMBIENT/FX/PLAYER/NPC/TALK PRIORITY (0 - 50) (most important to least important)" };

	const char musiccomments[] = { "\n//List of musics\n//MUSIC id path LOOP/NOLOOP" };

	int proceed = 0;

	CString str;

	CFileDialog saveas(FALSE, _T(".list"), L"sound", OFN_HIDEREADONLY | OFN_OVERWRITEPROMPT, _T("List files|*.list||"));

	if (!AList.issaved)
	{
		if (saveas.DoModal() == IDOK)
		{
			if ((f = fopen(W2A(saveas.GetPathName()), "w")) == NULL)
			{
				MessageBox(L"Could not save list file", L"Error", MB_OK);
				proceed = 1;
			}
		}
		else
			proceed = 1;
	}
	else
	{
		if ((f = fopen(AList.file, "w")) == NULL)
		{
			MessageBox(L"Could not save list file", L"Error", MB_OK);
			proceed = 1;
		}
	}

	if (!proceed)
	{
		fprintf(f, "%s\n\n", headercomments);

		for (int i = 0; i < AList.num_sounds; i++)
		{
			id = W2A(CmAudioDlg::listct.GetItemText(i, 1));
			path = W2A(listct.GetItemText(i, 2));
			str = listct.GetItemText(i, 3);

			if (!str.Compare(L"Yes"))
				strcpy(loop, "LOOP");
			else
				strcpy(loop, "NOLOOP");

			type = W2A(listct.GetItemText(i, 4));
			pr = W2A(listct.GetItemText(i, 5));
			fprintf(f, "SOUND %s \"%s\" %s %s %s \n", id, path, loop, type, pr);
		}

		fprintf(f, "%s\n\n", musiccomments);

		for (int i = 0; i < AList.num_musics; i++)
		{
			id = W2A(listm.GetItemText(i, 1));
			path = W2A(listm.GetItemText(i, 2));
			str = listm.GetItemText(i, 3);

			if (!str.Compare(L"Yes"))
				strcpy(loop, "LOOP");
			else
				strcpy(loop, "NOLOOP");

			fprintf(f, "MUSIC %s \"%s\" %s \n", id, path, loop);
		}

		fclose(f);

		if (!AList.issaved)
			strcpy(AList.file, W2A(saveas.GetPathName()));

		AList.issaved = 1;
		AList.changes = 0;
		savelist.EnableWindow(0);

		CString Wtext;

		Wtext.Format(_T("mAudio - %s"), saveas.GetPathName());

		SetWindowText(Wtext.GetString());
	}
}

void CmAudioDlg::OnBnClickedButton1()
{
	// TODO: Add your control notification handler code here

	CString str;

	LPWSTR dir = 0;

	FILE *f;

	int proceed = 0, value, id;

	char *buf, buf2[512], *tok;

	USES_CONVERSION;

	if (MessageBox(L"Are you sure you want to open a new sound list?\nAll unsaved content will be deleted.", L"Open Sound List", MB_YESNO) == IDYES)
	{

		memset(&AList, 0, sizeof(struct AudioList));

		listct.DeleteAllItems();
		listm.DeleteAllItems();

		CFileDialog openfile(TRUE, _T(".list"), L"sound", NULL, _T("List files|*.list||"));

		if (openfile.DoModal() == IDOK)
		{

			buf = W2A(openfile.GetPathName());

			if ((f = fopen(buf, "r")) == NULL)
			{
				MessageBox(L"Could not open sound list", L"Error", MB_OK);
				proceed = 1;
			}

			if (!proceed)
			{
				while (!feof(f))
				{
					memset(buf2, 0, 512);
					fgets(buf2, 512, f);

					if (buf2[0] == '/' && buf2[1] == '/')
						continue;

					if (buf2[0] == '\0' || buf2[0] == '\n')
						continue;

					tok = strtok(buf2, " \"");

					if (strcmp(tok, "SOUND") == NULL)
					{
						id = listct.InsertItem(AList.num_sounds, L"Sound");

						tok = strtok(NULL, " \"");

						listct.SetItemText(id, 1, A2W(tok));

						tok = strtok(NULL, " \"");

						listct.SetItemText(id, 2, A2W(tok));

						tok = strtok(NULL, " \"");

						if (strcmp(tok, "NOLOOP") == NULL)
							listct.SetItemText(id, 3, L"No");

						if (strcmp(tok, "LOOP") == NULL)
							listct.SetItemText(id, 3, L"Yes");

						tok = strtok(NULL, " \"");

						if (strcmp(tok, "AMBIENT") == NULL)
						{
							listct.SetItemText(id, 4, L"AMBIENT");
						}

						if (strcmp(tok, "FX") == NULL)
						{
							listct.SetItemText(id, 4, L"FX");
						}

						if (strcmp(tok, "PLAYER") == NULL)
						{
							listct.SetItemText(id, 4, L"PLAYER");
						}

						if (strcmp(tok, "NPC") == NULL)
						{
							listct.SetItemText(id, 4, L"NPC");
						}

						if (strcmp(tok, "TALK") == NULL)
						{
							listct.SetItemText(id, 4, L"TALK");
						}

						tok = strtok(NULL, " \"");

						listct.SetItemText(id, 5, A2W(tok));

						AList.num_sounds++;
					}
					else
						if (strcmp(tok, "MUSIC") == NULL)
						{
							id = listm.InsertItem(AList.num_sounds, L"Music");

							tok = strtok(NULL, " \"");

							listm.SetItemText(id, 1, A2W(tok));

							tok = strtok(NULL, " \"");

							listm.SetItemText(id, 2, A2W(tok));

							tok = strtok(NULL, " \" \n\r\0");

							if (strcmp(tok, "NOLOOP") == NULL)
								listm.SetItemText(id, 3, L"No");

							if (strcmp(tok, "LOOP") == NULL)
								listm.SetItemText(id, 3, L"Yes");

							AList.num_musics++;
						}
				}

				fclose(f);

				strcpy(AList.file, W2A(openfile.GetPathName()));

				IDView.EnableWindow(0);
				PathView.EnableWindow(0);
				LoopControl.EnableWindow(0);
				SoundType.EnableWindow(0);
				SoundPr.EnableWindow(0);
				RemoveSND.EnableWindow(0);

				IDmusic.EnableWindow(0);
				PathViewM.EnableWindow(0);
				LoopMusic.EnableWindow(0);
				RemoveMSC.EnableWindow(0);
				PlayAudio.EnableWindow(0);
				PlaySND.EnableWindow(0);
				StopAudio.EnableWindow(0);
				StopSND.EnableWindow(0);

				savelist.EnableWindow(0);

				AList.issaved = 1;
				AList.changes = 0;

				CString Wtext;

				Wtext.Format(_T("mAudio - %s"), openfile.GetPathName());

				SetWindowText(Wtext.GetString());
			}
		}
	}
}


void CmAudioDlg::OnBnClickedButton2()
{
	// TODO: Add your control notification handler code here

	CString str;

	LPWSTR dir = 0;

	char *buf, dir_buf[512];

	USES_CONVERSION;

	if (SoundSelected > -1)
	{
		CFileDialog openfile(TRUE, _T("Audio"), NULL, NULL, _T("Audio files|*.wav; *.mp3; *.ogg||"));

		if (openfile.DoModal() == IDOK)
		{

			buf = W2A(openfile.GetPathName());

			GetCurrentDirectoryA(512, dir_buf);

			buf = GetRelativePath(dir_buf, buf);

			//strcpy(AList.sound[SoundSelected].file, buf);

			str.SetString(A2W(buf));

			listct.SetItemText(SoundSelected, 2, openfile.GetPathName());

			AList.changes = 1;

			savelist.EnableWindow(1);
		}
	}
}


void CmAudioDlg::OnBnClickedButton8()
{
	// TODO: Add your control notification handler code here

	CString str;

	LPWSTR dir = 0;

	char *buf, dir_buf[512];

	USES_CONVERSION;

	if (MusicSelected > -1)
	{
		CFileDialog openfile(TRUE, _T("Audio"), NULL, NULL, _T("Audio files|*.wav; *.mp3; *.ogg||"));

		if (openfile.DoModal() == IDOK)
		{

			buf = W2A(openfile.GetPathName());

			GetCurrentDirectoryA(512, dir_buf);

			buf = GetRelativePath(dir_buf, buf);

			//strcpy(AList.sound[SoundSelected].file, buf);

			str.SetString(A2W(buf));

			listm.SetItemText(MusicSelected, 2, openfile.GetPathName());

			AList.changes = 1;

			savelist.EnableWindow(1);
		}
	}
}


void CmAudioDlg::OnBnClickedButton9()
{
	// TODO: Add your control notification handler code here

	AList.multiplesndselection = 0;
	SoundSelected = -1;
	AudioSelected = -1;
	
	for (int i = 0; i < AList.num_sounds; i++)
		listct.SetCheck(i, 0);

	IDView.EnableWindow(0);
	PathView.EnableWindow(0);
	LoopControl.EnableWindow(0);
	SoundType.EnableWindow(0);
	SoundPr.EnableWindow(0);
	RemoveSND.EnableWindow(0);
}


void CmAudioDlg::OnBnClickedButton12()
{
	// TODO: Add your control notification handler code here

	AList.multiplemusselection = 0;
	MusicSelected = -1;
	AudioSelected = -1;

	for (int i = 0; i < AList.num_musics; i++)
		listm.SetCheck(i, 0);

	IDmusic.EnableWindow(0);
	PathViewM.EnableWindow(0);
	LoopMusic.EnableWindow(0);
	RemoveMSC.EnableWindow(0);
}


void CmAudioDlg::OnBnClickedButton13()
{
	// TODO: Add your control notification handler code here

	FMOD_RESULT re;
	FMOD_BOOL p;
	CString error;

	USES_CONVERSION;

	if (AudioSelected == 0 || AudioSelected == 1)
	{
		FMOD_Channel_IsPlaying(snd_channel, &p);

		if (p == 1)
		{
			FMOD_Channel_Stop(snd_channel);
			FMOD_Sound_Release(audio);
		}

		//if (AudioSelected == 0)
			//re = FMOD_System_CreateSound(snd_system, W2A(listct.GetItemText(SoundSelected, 2)), FMOD_HARDWARE, NULL, &audio);

		//if (AudioSelected == 1)
			re = FMOD_System_CreateSound(snd_system, W2A(listm.GetItemText(MusicSelected, 2)), FMOD_HARDWARE, NULL, &audio);

		if (re != FMOD_OK)
		{
			error.Format(_T("%s"), FMOD_ErrorString(re));
			MessageBox(error, L"Error", MB_OK);
		}
		else
		{
			re = FMOD_System_PlaySound(snd_system, FMOD_CHANNEL_FREE, audio, NULL, &snd_channel);

			if (re != FMOD_OK)
			{
				error.Format(_T("%s"), FMOD_ErrorString(re));
				MessageBox(error, L"Error", MB_OK);
			}
		}
	}
}


void CmAudioDlg::OnBnClickedButton14()
{
	// TODO: Add your control notification handler code here

	FMOD_BOOL p;

	FMOD_Channel_IsPlaying(snd_channel, &p);
	if (p)
	{
		FMOD_Channel_Stop(snd_channel);
		FMOD_Sound_Release(audio);
	}
}

void CmAudioDlg::OnBnClickedButton15()
{
	// TODO: Add your control notification handler code here

	FMOD_RESULT re;
	FMOD_BOOL p;
	CString error;

	USES_CONVERSION;

	if (AudioSelected == 0 || AudioSelected == 1)
	{
		FMOD_Channel_IsPlaying(snd_channel, &p);

		if (p == 1)
		{
			FMOD_Channel_Stop(snd_channel);
			FMOD_Sound_Release(audio);
		}

		//if (AudioSelected == 0)
		re = FMOD_System_CreateSound(snd_system, W2A(listct.GetItemText(SoundSelected, 2)), FMOD_HARDWARE, NULL, &audio);

		//if (AudioSelected == 1)
		//re = FMOD_System_CreateSound(snd_system, W2A(listm.GetItemText(MusicSelected, 2)), FMOD_HARDWARE, NULL, &audio);

		if (re != FMOD_OK)
		{
			error.Format(_T("%s"), FMOD_ErrorString(re));
			MessageBox(error, L"Error", MB_OK);
		}
		else
		{
			re = FMOD_System_PlaySound(snd_system, FMOD_CHANNEL_FREE, audio, NULL, &snd_channel);

			if (re != FMOD_OK)
			{
				error.Format(_T("%s"), FMOD_ErrorString(re));
				MessageBox(error, L"Error", MB_OK);
			}
		}
	}
}


void CmAudioDlg::OnBnClickedButton16()
{
	// TODO: Add your control notification handler code here

	FMOD_BOOL p;

	FMOD_Channel_IsPlaying(snd_channel, &p);
	if (p)
	{
		FMOD_Channel_Stop(snd_channel);
		FMOD_Sound_Release(audio);
	}
}
