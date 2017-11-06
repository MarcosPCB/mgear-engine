
// mAudio.h : main header file for the PROJECT_NAME application
//

#pragma once

#ifndef __AFXWIN_H__
	#error "include 'stdafx.h' before including this file for PCH"
#endif

#include "resource.h"		// main symbols


#define MAXSOUNDS 2048
#define MAXMUSICS 2048

struct AudioList
{
	int num_sounds;
	int num_musics;
	char file[1024];
	int issaved;
	int changes;

	int multiplesndselection, selectedsnd[MAXSOUNDS];
	int multiplemusselection, selectedmus[MAXSOUNDS];
};

// CmAudioApp:
// See mAudio.cpp for the implementation of this class
//

class CmAudioApp : public CWinApp
{
public:
	CmAudioApp();

// Overrides
public:
	virtual BOOL InitInstance();

// Implementation

	DECLARE_MESSAGE_MAP()
};

extern CmAudioApp theApp;
extern struct AudioList AList;
extern int SoundSelected;
extern int MusicSelected;
extern int AudioSelected;

char* GetRelativePath(char *currentDirectory, char *absoluteFilename);