/*----------------------------------------------------------
OpenKey - The Cross platform Open source Vietnamese Keyboard application.

Copyright (C) 2019 Mai Vu Tuyen
Contact: maivutuyen.91@gmail.com
Github: https://github.com/tuyenvm/OpenKey
Fanpage: https://www.facebook.com/OpenKeyVN

This file is belong to the OpenKey project, Win32 version
which is released under GPL license.
You can fork, modify, improve this program. If you
redistribute your new version, it MUST be open source.
-----------------------------------------------------------*/
#pragma once
#include "BaseDialog.h"

class MainControlDialog : public BaseDialog {
private:
	HWND hTab, hTabPage1, hTabPage2, hTabPage3, hTabPage4, hTabPage5;
	HWND comboBoxInputType;
	HWND comboBoxTableCode;
	HWND checkCtrl, checkAlt, checkWin, checkShift, textSwitchKey, checkBeep;
	HWND checkVietnamese, checkEnglish;
	HWND checkModernOrthorgraphy, checkFixRecommendBrowser, checkShowOnStartup, checkRunWithWindows,
		checkSpelling, checkRestoreIfWrongSpelling, checkUseClipboard, checkModernIcon,
		checkAllowZWJF, checkTempOffSpelling, checkQuickStartConsonant, checkQuickEndConsonant;
	HWND checkSmartSwitchKey, checkCapsFirstChar, checkQuickTelex, checkUseMacro, checkUseMacroInEnglish;
	HWND checkCreateDesktopShortcut, checkCheckNewVersion, checkRunAsAdmin, checkSupportMetroApp, checkMacroAutoCaps;
	HWND checkFixChromium, checkRememberTableCode, checkTempOffOpenKey, checkAllowOtherLanguages;
	HWND hUpdateButton;
	
	// Exclusion tab controls
	HWND checkExclusionEnabled, listExcludedApps, buttonAddApp, buttonRemoveApp, buttonAddManual;
private:
	void initDialog();
	void onComboBoxSelected(const HWND& hCombobox, const int& comboboxId);
	void onCheckboxClicked(const HWND& hWnd);
	void onCharacter(const HWND& hWnd, const UINT16& keyCode);
	void setSwitchKeyText(const HWND& hWnd, const UINT16 & keyCode);
	void onTabIndexChanged();
	void onUpdateButton();
	void requestRestartAsAdmin();
	
	// Exclusion methods
	void initExclusionTab();
	void loadExcludedApps();
	void saveExcludedApps();
	void addAppToExclusion(const string& appName);
	void removeSelectedApp();
	void onAddAppButton();
	void onAddManualButton();
	bool isAppExcluded(const string& appName);
	
	// App picker methods
	vector<string> getRecentApps();
	vector<string> searchApps(const string& searchTerm);
	HICON extractAppIcon(const string& appName);
	wstring findAppPath(const string& appName);
	wstring getProductName(const string& appName);
	static INT_PTR CALLBACK appPickerDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam);
protected:
	INT_PTR eventProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
	static INT_PTR CALLBACK tabPageEventProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
public:
	MainControlDialog(const HINSTANCE& hInstance, const int& resourceId);
	virtual ~MainControlDialog();
	virtual void fillData() override;
	void setSwitchKey(const unsigned short& code);

	friend INT_PTR CALLBACK tabPageEventProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam);
};

