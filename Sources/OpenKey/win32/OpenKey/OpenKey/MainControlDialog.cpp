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
#include "MainControlDialog.h"
#include "AppDelegate.h"
#include <Shlobj.h>
#include <Uxtheme.h>
#include <Commdlg.h>
#include <algorithm>
#include <Shellapi.h>
#include <Commctrl.h>

#pragma comment(lib, "UxTheme.lib")
#pragma comment(lib, "Comdlg32.lib")
#pragma comment(lib, "Shell32.lib")
#pragma comment(lib, "Comctl32.lib")
#pragma comment(lib, "Version.lib")

static Uint16 _lastKeyCode;

MainControlDialog::MainControlDialog(const HINSTANCE& hInstance, const int& resourceId)
    : BaseDialog(hInstance, resourceId) {
}

MainControlDialog::~MainControlDialog() {
}

void MainControlDialog::initDialog() {
    HINSTANCE hIns = GetModuleHandleW(NULL);
    //dialog icon
    SET_DIALOG_ICON(IDI_APP_ICON);

    //set title version
    TCHAR title[256];
    TCHAR titleBuffer[256];
    LoadString(hIns, IDS_MAIN_DIALOG_TITLE, title, 256);
    wsprintfW(titleBuffer, title, OpenKeyHelper::getVersionString().c_str());
    SetWindowText(hDlg, titleBuffer);

    //input type
    comboBoxInputType = GetDlgItem(hDlg, IDC_COMBO_INPUT_TYPE);
    vector<LPCTSTR>& inputType = OpenKeyManager::getInputType();
    for (int i = 0; i < inputType.size(); i++) {
        SendMessage(comboBoxInputType, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(inputType[i]));
    }
    createToolTip(comboBoxInputType, IDS_STRING_INPUT);

    //code table
    comboBoxTableCode = GetDlgItem(hDlg, IDC_COMBO_TABLE_CODE);
    vector<LPCTSTR>& tableCode = OpenKeyManager::getTableCode();
    for (int i = 0; i < tableCode.size(); i++) {
        SendMessage(comboBoxTableCode, CB_ADDSTRING, i, reinterpret_cast<LPARAM>(tableCode[i]));
    }
    createToolTip(comboBoxTableCode, IDS_STRING_CODE);

    //init tabview
    hTab = GetDlgItem(hDlg, IDC_TAB_CONTROL);
    TCITEM tci = { 0 };
    tci.mask = TCIF_TEXT;
    tci.pszText = (LPWSTR)_T("Bộ gõ");
    TabCtrl_InsertItem(hTab, 0, &tci);
    tci.pszText = (LPWSTR)_T("Gõ tắt");
    TabCtrl_InsertItem(hTab, 1, &tci);
    tci.pszText = (LPWSTR)_T("Hệ thống");
    TabCtrl_InsertItem(hTab, 2, &tci);
    tci.pszText = (LPWSTR)_T("Loại trừ");
    TabCtrl_InsertItem(hTab, 3, &tci);
    tci.pszText = (LPWSTR)_T("Thông tin");
    TabCtrl_InsertItem(hTab, 4, &tci);
    RECT r;
    TabCtrl_GetItemRect(hTab, 0, &r);
    TabCtrl_SetItemSize(hTab, r.right - r.left, (r.bottom - r.top) * 1.428f);

    //create tab page
    hTabPage1 = CreateDialogParam(hIns, MAKEINTRESOURCE(IDD_DIALOG_TAB_GENERAL), hDlg, tabPageEventProc, (LPARAM)this);
    hTabPage2 = CreateDialogParam(hIns, MAKEINTRESOURCE(IDD_DIALOG_TAB_MACRO), hDlg, tabPageEventProc, (LPARAM)this);
    hTabPage3 = CreateDialogParam(hIns, MAKEINTRESOURCE(IDD_DIALOG_TAB_SYSTEM), hDlg, tabPageEventProc, (LPARAM)this);
    hTabPage4 = CreateDialogParam(hIns, MAKEINTRESOURCE(IDD_DIALOG_TAB_EXCLUSION), hDlg, tabPageEventProc, (LPARAM)this);
    hTabPage5 = CreateDialogParam(hIns, MAKEINTRESOURCE(IDD_DIALOG_TAB_INFO), hDlg, tabPageEventProc, (LPARAM)this);
    RECT rc;//find tab control's rectangle
    GetWindowRect(hTab, &rc);
    POINT offset = { 0 };
    ScreenToClient(hDlg, &offset);
    OffsetRect(&rc, offset.x, offset.y); //convert to client coordinates
    rc.top += (LONG)((r.bottom - r.top) * 1.428f);
    SetWindowPos(hTabPage1, 0, rc.left + 1, rc.top + 3, rc.right - rc.left - 5, rc.bottom - rc.top - 5, SWP_HIDEWINDOW);
    SetWindowPos(hTabPage2, 0, rc.left + 1, rc.top + 3, rc.right - rc.left - 5, rc.bottom - rc.top - 6, SWP_HIDEWINDOW);
    SetWindowPos(hTabPage3, 0, rc.left + 1, rc.top + 3, rc.right - rc.left - 5, rc.bottom - rc.top - 6, SWP_HIDEWINDOW);
    SetWindowPos(hTabPage4, 0, rc.left + 1, rc.top + 3, rc.right - rc.left - 5, rc.bottom - rc.top - 6, SWP_HIDEWINDOW);
    SetWindowPos(hTabPage5, 0, rc.left + 1, rc.top + 3, rc.right - rc.left - 5, rc.bottom - rc.top - 6, SWP_HIDEWINDOW);
    onTabIndexChanged();

    checkCtrl = GetDlgItem(hDlg, IDC_CHECK_SWITCH_KEY_CTRL);
    createToolTip(checkCtrl, IDS_STRING_CTRL);

    checkAlt = GetDlgItem(hDlg, IDC_CHECK_SWITCH_KEY_ALT);
    createToolTip(checkAlt, IDS_STRING_ALT);

    checkWin = GetDlgItem(hDlg, IDC_CHECK_SWITCH_KEY_WIN);
    createToolTip(checkWin, IDS_STRING_WIN);

    checkShift = GetDlgItem(hDlg, IDC_CHECK_SWITCH_KEY_SHIFT);
    createToolTip(checkShift, IDS_STRING_SHIFT);

    textSwitchKey = GetDlgItem(hDlg, IDC_SWITCH_KEY_KEY);
    createToolTip(textSwitchKey, IDS_STRING_SWITCH_KEY);

    checkBeep = GetDlgItem(hDlg, IDC_CHECK_SWITCH_KEY_BEEP);
    createToolTip(checkBeep, IDS_STRING_BEEP);

    checkVietnamese = GetDlgItem(hDlg, IDC_RADIO_METHOD_VIETNAMESE);
    createToolTip(checkVietnamese, IDS_STRING_VIET);

    checkEnglish = GetDlgItem(hDlg, IDC_RADIO_METHOD_ENGLISH);
    createToolTip(checkEnglish, IDS_STRING_ENG);

    /*--------end common---------*/

    checkModernOrthorgraphy = GetDlgItem(hTabPage1, IDC_CHECK_MODERN_ORTHORGRAPHY);
    createToolTip(checkModernOrthorgraphy, IDS_STRING_MORDEN_ORTHORGRAPHY);

    checkFixRecommendBrowser = GetDlgItem(hTabPage1, IDC_CHECK_FIX_RECOMMEND_BROWSER);
    createToolTip(checkFixRecommendBrowser, IDS_STRING_FIX_BROWSER);

    checkSpelling = GetDlgItem(hTabPage1, IDC_CHECK_SPELLING);
    createToolTip(checkSpelling, IDS_STRING_SPELLING_CHECK);

    checkRestoreIfWrongSpelling = GetDlgItem(hTabPage1, IDC_CHECK_RESTORE_IF_WRONG_SPELLING);
    createToolTip(checkRestoreIfWrongSpelling, IDS_STRING_RESTORE_IF_WRONG_SPELLING);

    checkAllowZWJF = GetDlgItem(hTabPage1, IDC_CHECK_ALLOW_ZJWF);
    createToolTip(checkAllowZWJF, IDS_STRING_ALLOW_ZWFJ);

    checkTempOffSpelling = GetDlgItem(hTabPage1, IDC_CHECK_TEMP_OFF_SPELLING);
    createToolTip(checkTempOffSpelling, IDS_STRING_TEMP_OFF_SPELLING);

    checkSmartSwitchKey = GetDlgItem(hTabPage1, IDC_CHECK_SMART_SWITCH_KEY);
    createToolTip(checkSmartSwitchKey, IDS_STRING_SMART_SWITCH_KEY);

    checkCapsFirstChar = GetDlgItem(hTabPage1, IDC_CHECK_CAPS_FIRST_CHAR);
    createToolTip(checkCapsFirstChar, IDS_STRING_CAPS_FIRST_CHAR);

    checkRememberTableCode = GetDlgItem(hTabPage1, IDC_CHECK_SMART_SWITCH_CODE);
    createToolTip(checkRememberTableCode, IDS_STRING_REMEMBER_TABLE_CODE);

    checkAllowOtherLanguages = GetDlgItem(hTabPage1, IDC_CHECK_OTHER_LANGUAGES);
    createToolTip(checkAllowOtherLanguages, IDS_STRING_OTHER_LANGUAGES);

    checkTempOffOpenKey = GetDlgItem(hTabPage1, IDC_CHECK_TEMP_OFF_OPEN_KEY);
    createToolTip(checkTempOffOpenKey, IDS_STRING_TEMP_OFF_OPENKEY);

    /*------------end tab 1----------------*/

    checkQuickStartConsonant = GetDlgItem(hTabPage2, IDC_CHECK_QUICK_START_CONSONANT);
    createToolTip(checkQuickStartConsonant, IDS_STRING_START_CONSONANT);

    checkQuickEndConsonant = GetDlgItem(hTabPage2, IDC_CHECK_QUICK_END_CONSONANT);
    createToolTip(checkQuickEndConsonant, IDS_STRING_END_CONSONANT);

    checkQuickTelex = GetDlgItem(hTabPage2, IDC_CHECK_QUICK_TELEX);
    createToolTip(checkQuickTelex, IDS_STRING_QUICK_TELEX);

    checkUseMacro = GetDlgItem(hTabPage2, IDC_CHECK_USE_MACRO);
    createToolTip(checkUseMacro, IDS_STRING_MACRO);

    checkUseMacroInEnglish = GetDlgItem(hTabPage2, IDC_CHECK_USE_MACRO_IN_ENGLISH);
    createToolTip(checkUseMacroInEnglish, IDS_STRING_MACRO_IN_ENG);

    checkMacroAutoCaps = GetDlgItem(hTabPage2, IDC_CHECK_AUTO_CAPS);
    createToolTip(checkMacroAutoCaps, IDS_STRING_MACRO_AUTO_CAP);

    hUpdateButton = GetDlgItem(hDlg, IDC_BUTTON_CHECK_UPDATE);

    /*------------end tab 2----------------*/

    checkModernIcon = GetDlgItem(hTabPage3, IDC_CHECK_MODERN_ICON);
    createToolTip(checkModernIcon, IDS_STRING_MODERN_ICON);

    checkShowOnStartup = GetDlgItem(hTabPage3, IDC_CHECK_SHOW_ON_STARTUP);
    createToolTip(checkShowOnStartup, IDS_STRING_SHOW_ON_STARTUP);

    checkRunWithWindows = GetDlgItem(hTabPage3, IDC_CHECK_RUN_WITH_WINDOWS);
    createToolTip(checkRunWithWindows, IDS_STRING_RUN_ON_STARTUP);

    checkCreateDesktopShortcut = GetDlgItem(hTabPage3, IDC_CHECK_CHECK_CREATE_SHORTCUT);
    createToolTip(checkCreateDesktopShortcut, IDS_STRING_CREATE_DESKTOP);

    checkRunAsAdmin = GetDlgItem(hTabPage3, IDC_CHECK_RUN_AS_ADMIN);
    createToolTip(checkRunAsAdmin, IDS_STRING_RUN_AS_ADMIN);

    checkCheckNewVersion = GetDlgItem(hTabPage3, IDC_CHECK_CHECK_UPDATE);
    createToolTip(checkCheckNewVersion, IDS_STRING_CHECK_UPDATE);

    checkSupportMetroApp = GetDlgItem(hTabPage3, IDC_CHECK_SUPPORT_METRO_APP);
    createToolTip(checkSupportMetroApp, IDS_STRING_SUPPORT_METRO);

    checkUseClipboard = GetDlgItem(hTabPage3, IDC_CHECK_USE_CLIPBOARD);
    createToolTip(checkUseClipboard, IDS_STRING_USE_CLIPBOARD);

    checkFixChromium = GetDlgItem(hTabPage3, IDC_CHECK_FIX_CHROMIUM);
    createToolTip(checkFixChromium, IDS_STRING_FIX_CHROMIUM);

    /*------------end tab 3----------------*/

    // Initialize exclusion tab (tab 4)
    initExclusionTab();

    /*------------end tab 4----------------*/

    SendDlgItemMessage(hDlg, IDBUTTON_OK, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hIns, MAKEINTRESOURCEW(IDI_ICON_OK_BUTTON)));
    SendDlgItemMessage(hDlg, ID_BTN_DEFAULT, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hIns, MAKEINTRESOURCEW(IDI_ICON_DEFAULT_BUTTON)));
    SendDlgItemMessage(hDlg, IDBUTTON_EXIT, BM_SETIMAGE, IMAGE_ICON, (LPARAM)LoadIcon(hIns, MAKEINTRESOURCEW(IDI_ICON_EXIT_BUTTON)));
    fillData();
}

INT_PTR MainControlDialog::eventProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {
    case WM_INITDIALOG:
        this->hDlg = hDlg;
        initDialog();
        return TRUE;
    case WM_COMMAND: {
        int wmId = LOWORD(wParam);
        switch (wmId) {
        case IDBUTTON_OK:
            AppDelegate::getInstance()->closeDialog(this);
            break;
        case IDBUTTON_EXIT:
            AppDelegate::getInstance()->onOpenKeyExit();
            break;
        case ID_BTN_DEFAULT: {
            int msgboxID = MessageBox(
                hDlg,
                _T("Bạn có chắc chắn muốn thiết lập lại cài đặt gốc?"),
                _T("OpenKey"),
                MB_ICONEXCLAMATION | MB_YESNO
            );
            if (msgboxID == IDYES) {
                AppDelegate::getInstance()->onDefaultConfig();
            }
            break;
        }
        case IDC_BUTTON_MACRO_TABLE:
            AppDelegate::getInstance()->onMacroTable();
            break;
        case IDC_BUTTON_CHECK_UPDATE:
            onUpdateButton();
            break;
        case IDC_BUTTON_GO_SOURCE_CODE:
            ShellExecute(NULL, _T("open"), _T("https://github.com/vothaianh/OpenKey"), NULL, NULL, SW_SHOWNORMAL);
            break;
        case IDC_BUTTON_ADD_APP:
            onAddAppButton();
            break;
        case IDC_BUTTON_ADD_MANUAL:
            onAddManualButton();
            break;
        case IDC_BUTTON_REMOVE_APP:
            removeSelectedApp();
            break;
        default:
            if (HIWORD(wParam) == CBN_SELCHANGE) {
                this->onComboBoxSelected((HWND)lParam, LOWORD(wParam));
            }
            else if (HIWORD(wParam) == BN_CLICKED) {
                this->onCheckboxClicked((HWND)lParam);
            }
            else if (HIWORD(wParam) == EN_CHANGE) {
                _lastKeyCode = OpenKeyManager::_lastKeyCode;
                if (_lastKeyCode > 0) {
                    OpenKeyManager::_lastKeyCode = 0;
                    this->onCharacter((HWND)lParam, _lastKeyCode);
                }
            }
            break;
        }
        break;
    }
    case WM_NOTIFY: {
        switch (((LPNMHDR)lParam)->code) {
        case TCN_SELCHANGE:
            onTabIndexChanged();
            break;
        case NM_CLICK:
        case NM_RETURN: {
            PNMLINK link = (PNMLINK)lParam;
            if (link->hdr.idFrom == IDC_SYSLINK_HOME_PAGE)
                ShellExecute(NULL, _T("open"), _T("http://open-key.org"), NULL, NULL, SW_SHOWNORMAL);
            else if (link->hdr.idFrom == IDC_SYSLINK_FANPAGE)
                ShellExecute(NULL, _T("open"), _T("https://www.facebook.com/OpenKeyVN"), NULL, NULL, SW_SHOWNORMAL);
            else if (link->hdr.idFrom == IDC_SYSLINK_AUTHOR_EMAIL)
                ShellExecute(NULL, _T("open"), _T("mailto:maivutuyen.91@gmail.com"), NULL, NULL, SW_SHOWNORMAL);
            break;
        }
        }
        break;
    }
    }

    return FALSE;
}

INT_PTR MainControlDialog::tabPageEventProc(HWND hDlg, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    if (uMsg == WM_INITDIALOG) {
#ifdef _WIN64
        SetWindowLongPtr(hDlg, GWLP_USERDATA, lParam);
#else
        SetWindowLong(hDlg, GWL_USERDATA, lParam);
#endif
        return TRUE;
    }
    else if (uMsg == WM_ERASEBKGND) {
        return TRUE;
    }
    else if ((uMsg == WM_CTLCOLORSTATIC || uMsg == WM_CTLCOLORBTN) && IsThemeActive()) {
        SetBkMode((HDC)wParam, TRANSPARENT);
        return (LRESULT)GetStockObject(COLOR_WINDOW + 1);
    }
    else if (uMsg == WM_PAINT && IsThemeActive()) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(hDlg, &ps);

        // All painting occurs here, between BeginPaint and EndPaint.
        FillRect(hdc, &ps.rcPaint, (HBRUSH)(COLOR_WINDOW + 1));

        EndPaint(hDlg, &ps);

        return 0;
    }

#ifdef _WIN64
    LONG_PTR attr = GetWindowLongPtr(hDlg, GWLP_USERDATA);
#else
    long attr = GetWindowLong(hDlg, GWL_USERDATA);
#endif
    if (attr != 0) {
        return ((MainControlDialog*)attr)->eventProc(hDlg, uMsg, wParam, lParam);
    }
    return FALSE;
}

void MainControlDialog::fillData() {
    SendMessage(comboBoxInputType, CB_SETCURSEL, vInputType, 0);
    SendMessage(comboBoxTableCode, CB_SETCURSEL, vCodeTable, 0);

    SendMessage(checkCtrl, BM_SETCHECK, HAS_CONTROL(vSwitchKeyStatus) ? 1 : 0, 0);
    SendMessage(checkAlt, BM_SETCHECK, HAS_OPTION(vSwitchKeyStatus) ? 1 : 0, 0);
    SendMessage(checkWin, BM_SETCHECK, HAS_COMMAND(vSwitchKeyStatus) ? 1 : 0, 0);
    SendMessage(checkShift, BM_SETCHECK, HAS_SHIFT(vSwitchKeyStatus) ? 1 : 0, 0);
    setSwitchKeyText(textSwitchKey, (vSwitchKeyStatus >> 24) & 0xFF);
    SendMessage(checkBeep, BM_SETCHECK, HAS_BEEP(vSwitchKeyStatus) ? 1 : 0, 0);

    SendMessage(checkVietnamese, BM_SETCHECK, vLanguage, 0);
    SendMessage(checkEnglish, BM_SETCHECK, !vLanguage, 0);

    SendMessage(checkModernOrthorgraphy, BM_SETCHECK, vUseModernOrthography ? 1 : 0, 0);
    SendMessage(checkFixRecommendBrowser, BM_SETCHECK, vFixRecommendBrowser ? 1 : 0, 0);
    SendMessage(checkShowOnStartup, BM_SETCHECK, vShowOnStartUp ? 1 : 0, 0);
    SendMessage(checkRunWithWindows, BM_SETCHECK, vRunWithWindows ? 1 : 0, 0);
    SendMessage(checkSpelling, BM_SETCHECK, vCheckSpelling ? 1 : 0, 0);
    SendMessage(checkRestoreIfWrongSpelling, BM_SETCHECK, vRestoreIfWrongSpelling ? 1 : 0, 0);
    SendMessage(checkModernIcon, BM_SETCHECK, vUseGrayIcon ? 1 : 0, 0);
    SendMessage(checkAllowZWJF, BM_SETCHECK, vAllowConsonantZFWJ ? 1 : 0, 0);
    SendMessage(checkTempOffSpelling, BM_SETCHECK, vTempOffSpelling ? 1 : 0, 0);
    SendMessage(checkQuickStartConsonant, BM_SETCHECK, vQuickStartConsonant ? 1 : 0, 0);
    SendMessage(checkQuickEndConsonant, BM_SETCHECK, vQuickEndConsonant ? 1 : 0, 0);
    SendMessage(checkRememberTableCode, BM_SETCHECK, vRememberCode ? 1 : 0, 0);
    SendMessage(checkAllowOtherLanguages, BM_SETCHECK, vOtherLanguage ? 1 : 0, 0);
    SendMessage(checkTempOffOpenKey, BM_SETCHECK, vTempOffOpenKey ? 1 : 0, 0);

    SendMessage(checkSmartSwitchKey, BM_SETCHECK, vUseSmartSwitchKey ? 1 : 0, 0);
    SendMessage(checkCapsFirstChar, BM_SETCHECK, vUpperCaseFirstChar ? 1 : 0, 0);
    SendMessage(checkQuickTelex, BM_SETCHECK, vQuickTelex ? 1 : 0, 0);
    SendMessage(checkUseMacro, BM_SETCHECK, vUseMacro ? 1 : 0, 0);
    SendMessage(checkUseMacroInEnglish, BM_SETCHECK, vUseMacroInEnglishMode ? 1 : 0, 0);

    SendMessage(checkMacroAutoCaps, BM_SETCHECK, vAutoCapsMacro ? 1 : 0, 0);
    SendMessage(checkSupportMetroApp, BM_SETCHECK, vSupportMetroApp ? 1 : 0, 0);
    SendMessage(checkCreateDesktopShortcut, BM_SETCHECK, vCreateDesktopShortcut ? 1 : 0, 0);
    SendMessage(checkRunAsAdmin, BM_SETCHECK, vRunAsAdmin ? 1 : 0, 0);
    SendMessage(checkCheckNewVersion, BM_SETCHECK, vCheckNewVersion ? 1 : 0, 0);
    SendMessage(checkUseClipboard, BM_SETCHECK, vSendKeyStepByStep ? 0 : 1, 0);
    SendMessage(checkFixChromium, BM_SETCHECK, vFixChromiumBrowser ? 1 : 0, 0);

    // Exclusion tab data
    SendMessage(checkExclusionEnabled, BM_SETCHECK, vExclusionEnabled ? 1 : 0, 0);
    EnableWindow(listExcludedApps, vExclusionEnabled);
    EnableWindow(buttonAddApp, vExclusionEnabled);
    EnableWindow(buttonAddManual, vExclusionEnabled);
    EnableWindow(buttonRemoveApp, vExclusionEnabled);
    loadExcludedApps();

    EnableWindow(checkRestoreIfWrongSpelling, vCheckSpelling);

    //tab info
    wchar_t buffer[256];
    wsprintfW(buffer, _T("Phiên bản %s cho Windows - Ngày cập nhật: %s"), OpenKeyHelper::getVersionString().c_str(), _T(__DATE__));
    SendDlgItemMessage(hTabPage5, IDC_STATIC_APP_VERSION_INFO, WM_SETTEXT, 0, LPARAM(buffer));
}

void MainControlDialog::setSwitchKey(const unsigned short& code) {
    vSwitchKeyStatus &= 0xFFFFFF00;
    vSwitchKeyStatus |= code;
    vSwitchKeyStatus &= 0x00FFFFFF;
    vSwitchKeyStatus |= ((unsigned int)code << 24);
    APP_SET_DATA(vSwitchKeyStatus, vSwitchKeyStatus);
}

void MainControlDialog::onComboBoxSelected(const HWND& hCombobox, const int& comboboxId) {
    if (hCombobox == comboBoxInputType) {
        APP_SET_DATA(vInputType, (int)SendMessage(hCombobox, CB_GETCURSEL, 0, 0));
    }
    else if (hCombobox == comboBoxTableCode) {
        APP_SET_DATA(vCodeTable, (int)SendMessage(hCombobox, CB_GETCURSEL, 0, 0));
        if (vRememberCode) {
            setAppInputMethodStatus(OpenKeyHelper::getFrontMostAppExecuteName(), vLanguage | (vCodeTable << 1));
            saveSmartSwitchKeyData();
        }
    }
    SystemTrayHelper::updateData();
}

void MainControlDialog::onCheckboxClicked(const HWND& hWnd) {
    int val = 0;
    if (hWnd == checkCtrl) {
        val = (int)SendMessage(checkCtrl, BM_GETCHECK, 0, 0);
        vSwitchKeyStatus &= (~0x100);
        vSwitchKeyStatus |= val << 8;
        APP_SET_DATA(vSwitchKeyStatus, vSwitchKeyStatus);
    }
    else if (hWnd == checkAlt) {
        val = (int)SendMessage(checkAlt, BM_GETCHECK, 0, 0);
        vSwitchKeyStatus &= (~0x200);
        vSwitchKeyStatus |= val << 9;
        APP_SET_DATA(vSwitchKeyStatus, vSwitchKeyStatus);
    }
    else if (hWnd == checkWin) {
        val = (int)SendMessage(checkWin, BM_GETCHECK, 0, 0);
        vSwitchKeyStatus &= (~0x400);
        vSwitchKeyStatus |= val << 10;
        APP_SET_DATA(vSwitchKeyStatus, vSwitchKeyStatus);
    }
    else if (hWnd == checkShift) {
        val = (int)SendMessage(checkShift, BM_GETCHECK, 0, 0);
        vSwitchKeyStatus &= (~0x800);
        vSwitchKeyStatus |= val << 11;
        APP_SET_DATA(vSwitchKeyStatus, vSwitchKeyStatus);
    }
    else if (hWnd == checkBeep) {
        val = (int)SendMessage(checkBeep, BM_GETCHECK, 0, 0);
        vSwitchKeyStatus &= (~0x8000);
        vSwitchKeyStatus |= val << 15;
        APP_SET_DATA(vSwitchKeyStatus, vSwitchKeyStatus);
    }
    else if (hWnd == checkVietnamese) {
        val = (int)SendMessage(checkVietnamese, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vLanguage, val ? 1 : 0);
        if (vUseSmartSwitchKey) {
            setAppInputMethodStatus(OpenKeyHelper::getFrontMostAppExecuteName(), vLanguage | (vCodeTable << 1));
            saveSmartSwitchKeyData();
        }
    }
    else if (hWnd == checkEnglish) {
        val = (int)SendMessage(checkVietnamese, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vLanguage, val ? 1 : 0);
        if (vUseSmartSwitchKey) {
            setAppInputMethodStatus(OpenKeyHelper::getFrontMostAppExecuteName(), vLanguage | (vCodeTable << 1));
            saveSmartSwitchKeyData();
        }
    }
    else if (hWnd == checkModernOrthorgraphy) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vUseModernOrthography, val ? 1 : 0);
    }
    else if (hWnd == checkFixRecommendBrowser) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vFixRecommendBrowser, val ? 1 : 0);
        EnableWindow(checkFixChromium, vFixRecommendBrowser);
    }
    else if (hWnd == checkShowOnStartup) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vShowOnStartUp, val ? 1 : 0);
    }
    else if (hWnd == checkRunWithWindows) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vRunWithWindows, val ? 1 : 0);
        OpenKeyHelper::registerRunOnStartup(vRunWithWindows);
    }
    else if (hWnd == checkSpelling) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vCheckSpelling, val ? 1 : 0);
        vSetCheckSpelling();
        EnableWindow(checkRestoreIfWrongSpelling, vCheckSpelling);
        EnableWindow(checkAllowZWJF, vCheckSpelling);
        EnableWindow(checkTempOffSpelling, vCheckSpelling);
    }
    else if (hWnd == checkRestoreIfWrongSpelling) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vRestoreIfWrongSpelling, val ? 1 : 0);
    }
    else if (hWnd == checkUseClipboard) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vSendKeyStepByStep, val ? 0 : 1);
    }
    else if (hWnd == checkSmartSwitchKey) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vUseSmartSwitchKey, val ? 1 : 0);
    }
    else if (hWnd == checkCapsFirstChar) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vUpperCaseFirstChar, val ? 1 : 0);
    }
    else if (hWnd == checkQuickTelex) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vQuickTelex, val ? 1 : 0);
    }
    else if (hWnd == checkUseMacro) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vUseMacro, val ? 1 : 0);
    }
    else if (hWnd == checkUseMacroInEnglish) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vUseMacroInEnglishMode, val ? 1 : 0);
    }
    else if (hWnd == checkModernIcon) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vUseGrayIcon, val ? 1 : 0);
    }
    else if (hWnd == checkAllowZWJF) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vAllowConsonantZFWJ, val ? 1 : 0);
    }
    else if (hWnd == checkTempOffSpelling) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vTempOffSpelling, val ? 1 : 0);
    }
    else if (hWnd == checkQuickStartConsonant) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vQuickStartConsonant, val ? 1 : 0);
    }
    else if (hWnd == checkQuickEndConsonant) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vQuickEndConsonant, val ? 1 : 0);
    }
    else if (hWnd == checkSupportMetroApp) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vSupportMetroApp, val ? 1 : 0);
    }
    else if (hWnd == checkMacroAutoCaps) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vAutoCapsMacro, val ? 1 : 0);
    }
    else if (hWnd == checkCreateDesktopShortcut) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vCreateDesktopShortcut, val ? 1 : 0);
        //create desktop shortcut
        if (val)
            OpenKeyManager::createDesktopShortcut();
    }
    else if (hWnd == checkRunAsAdmin) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vRunAsAdmin, val ? 1 : 0);
        requestRestartAsAdmin();
    }
    else if (hWnd == checkCheckNewVersion) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vCheckNewVersion, val ? 1 : 0);
    }
    else if (hWnd == checkRememberTableCode) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vRememberCode, val ? 1 : 0);
    }
    else if (hWnd == checkAllowOtherLanguages) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vOtherLanguage, val ? 1 : 0);
    }
    else if (hWnd == checkTempOffOpenKey) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vTempOffOpenKey, val ? 1 : 0);
    }
    else if (hWnd == checkFixChromium) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vFixChromiumBrowser, val ? 1 : 0);
    }
    else if (hWnd == checkExclusionEnabled) {
        val = (int)SendMessage(hWnd, BM_GETCHECK, 0, 0);
        APP_SET_DATA(vExclusionEnabled, val ? 1 : 0);
        EnableWindow(listExcludedApps, vExclusionEnabled);
        EnableWindow(buttonAddApp, vExclusionEnabled);
        EnableWindow(buttonAddManual, vExclusionEnabled);
        EnableWindow(buttonRemoveApp, vExclusionEnabled);
    }
    SystemTrayHelper::updateData();
}

void MainControlDialog::onCharacter(const HWND& hWnd, const UINT16& keyCode) {
    if (keyCode == 0) return;
    if (hWnd == textSwitchKey) {
        UINT16 code = GET_SWITCH_KEY(vSwitchKeyStatus);
        if (keyCode == VK_DELETE || keyCode == VK_BACK) {
            code = 0xFE;
        }
        else if (keyCodeToCharacter(keyCode) != 0) {
            code = keyCode;
        }
        setSwitchKey(code);
        setSwitchKeyText(hWnd, code);
    }
}

void MainControlDialog::setSwitchKeyText(const HWND& hWnd, const UINT16& keyCode) {
    if (keyCode == KEY_SPACE) {
        SetWindowText(hWnd, _T("Space"));
    }
    else if (keyCode == 0xFE) {
        SetWindowText(hWnd, _T(""));
    }
    else {
        Uint16 key[] = { keyCode, 0 };
        SetWindowText(hWnd, (LPCWSTR)&key);
    }
}

void MainControlDialog::onTabIndexChanged() {
    int index = TabCtrl_GetCurSel(hTab);
    ShowWindow(hTabPage1, (index == 0) ? SW_SHOW : SW_HIDE);
    ShowWindow(hTabPage2, (index == 1) ? SW_SHOW : SW_HIDE);
    ShowWindow(hTabPage3, (index == 2) ? SW_SHOW : SW_HIDE);
    ShowWindow(hTabPage4, (index == 3) ? SW_SHOW : SW_HIDE);
    ShowWindow(hTabPage5, (index == 4) ? SW_SHOW : SW_HIDE);
}

void MainControlDialog::onUpdateButton() {
    EnableWindow(hUpdateButton, false);
    string newVersion;
    if (OpenKeyManager::checkUpdate(newVersion)) {
        WCHAR msg[256];
        wsprintf(msg,
            TEXT("OpenKey Có phiên bản mới (%s), bạn có muốn cập nhật không?"),
            utf8ToWideString(newVersion).c_str());

        int msgboxID = MessageBox(
            hDlg,
            msg,
            _T("OpenKey Update"),
            MB_ICONEXCLAMATION | MB_YESNO
        );
        if (msgboxID == IDYES) {
            //Call OpenKeyUpdate
            WCHAR path[MAX_PATH];
            GetCurrentDirectory(MAX_PATH, path);
            wsprintf(path, TEXT("%s\\OpenKeyUpdate.exe"), path);
            ShellExecute(0, L"", path, 0, 0, SW_SHOWNORMAL);

            AppDelegate::getInstance()->onOpenKeyExit();
        }

    }
    else {
        MessageBox(hDlg, _T("Bạn đang dùng phiên bản mới nhất!"), _T("OpenKey Update"), MB_OK);
    }
    EnableWindow(hUpdateButton, true);
}

void MainControlDialog::requestRestartAsAdmin() {
    OpenKeyHelper::registerRunOnStartup(false);
    if (vRunAsAdmin && !IsUserAnAdmin()) {
        int msgboxID = MessageBox(
            hDlg,
            _T("Bạn cần phải khởi động lại OpenKey để kích hoạt chế độ Admin!\nBạn có muốn khởi động lại OpenKey không?"),
            _T("OpenKey"),
            MB_ICONEXCLAMATION | MB_YESNO
        );
        if (msgboxID == IDYES) {
            PostQuitMessage(0);
            ShellExecute(0, L"runas", OpenKeyHelper::getFullPath().c_str(), 0, 0, SW_SHOWNORMAL);
        }
    }
    else {
        OpenKeyHelper::registerRunOnStartup(vRunWithWindows);
    }
}

// Exclusion feature implementations
void MainControlDialog::initExclusionTab() {
    checkExclusionEnabled = GetDlgItem(hTabPage4, IDC_CHECK_EXCLUSION_ENABLED);
    createToolTip(checkExclusionEnabled, IDS_STRING_EXCLUSION_ENABLED);
    
    listExcludedApps = GetDlgItem(hTabPage4, IDC_LIST_EXCLUDED_APPS);
    buttonAddApp = GetDlgItem(hTabPage4, IDC_BUTTON_ADD_APP);
    buttonAddManual = GetDlgItem(hTabPage4, IDC_BUTTON_ADD_MANUAL);
    buttonRemoveApp = GetDlgItem(hTabPage4, IDC_BUTTON_REMOVE_APP);
    
    createToolTip(buttonAddApp, IDS_STRING_ADD_APP);
    createToolTip(buttonAddManual, IDS_STRING_ADD_MANUAL);
    createToolTip(buttonRemoveApp, IDS_STRING_REMOVE_APP);
    
    // Setup list view columns
    LVCOLUMN lvc = { 0 };
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;
    lvc.cx = 208;  // 50% of ListView width (416px / 2 = 208px each column)
    lvc.pszText = (LPWSTR)_T("Tên ứng dụng");
    ListView_InsertColumn(listExcludedApps, 0, &lvc);
    
    lvc.cx = 208;  // 50% of ListView width (416px / 2 = 208px each column)
    lvc.pszText = (LPWSTR)_T("Tên file exe");
    ListView_InsertColumn(listExcludedApps, 1, &lvc);
    
    // Set full row select
    ListView_SetExtendedListViewStyle(listExcludedApps, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
}

void MainControlDialog::loadExcludedApps() {
    ListView_DeleteAllItems(listExcludedApps);
    
    // Store wstrings to keep them alive during ListView operations
    static vector<wstring> displayNames;
    static vector<wstring> exeNames;
    displayNames.clear();
    exeNames.clear();
    
    for (size_t i = 0; i < vExcludedApps.size(); i++) {
        const string& app = vExcludedApps[i];
        wstring wApp = utf8ToWideString(app);
        exeNames.push_back(wApp);
        
        LVITEM lvi = { 0 };
        lvi.mask = LVIF_TEXT;
        lvi.iItem = (int)i;
        lvi.iSubItem = 0;
        
        // Check if it's a wildcard pattern
        if (app.find(".*") != string::npos) {
            displayNames.push_back(wApp);
            lvi.pszText = (LPWSTR)displayNames[i].c_str();
        } else {
            // Get product name (commercial name) instead of exe name
            wstring productName = getProductName(app);
            displayNames.push_back(productName);
            lvi.pszText = (LPWSTR)displayNames[i].c_str();
        }
        
        int index = ListView_InsertItem(listExcludedApps, &lvi);
        
        // Set exe name in second column
        ListView_SetItemText(listExcludedApps, index, 1, (LPWSTR)exeNames[i].c_str());
    }
}

void MainControlDialog::saveExcludedApps() {
    // Save to registry as binary data
    if (vExcludedApps.empty()) {
        OpenKeyHelper::setRegBinary(_T("ExcludedApps"), nullptr, 0);
        return;
    }
    
    // Calculate total size needed
    size_t totalSize = 0;
    for (const string& app : vExcludedApps) {
        totalSize += app.length() + 1; // +1 for null terminator
    }
    
    // Create buffer
    vector<char> buffer(totalSize);
    size_t offset = 0;
    
    for (const string& app : vExcludedApps) {
        memcpy(buffer.data() + offset, app.c_str(), app.length() + 1);
        offset += app.length() + 1;
    }
    
    OpenKeyHelper::setRegBinary(_T("ExcludedApps"), (BYTE*)buffer.data(), (int)totalSize);
}

void MainControlDialog::addAppToExclusion(const string& appName) {
    // Check if already exists
    for (const string& existing : vExcludedApps) {
        if (existing == appName) {
            return; // Already exists
        }
    }
    
    vExcludedApps.push_back(appName);
    saveExcludedApps();
    loadExcludedApps();
}

void MainControlDialog::removeSelectedApp() {
    int selectedIndex = ListView_GetNextItem(listExcludedApps, -1, LVNI_SELECTED);
    if (selectedIndex >= 0 && selectedIndex < (int)vExcludedApps.size()) {
        vExcludedApps.erase(vExcludedApps.begin() + selectedIndex);
        saveExcludedApps();
        loadExcludedApps();
    }
}

void MainControlDialog::onAddAppButton() {
    if (IDOK == DialogBoxParam(GetModuleHandle(NULL), 
        MAKEINTRESOURCE(IDD_DIALOG_APP_PICKER),
        hDlg, 
        appPickerDialogProc,
        (LPARAM)this)) {
        // App was selected and added in the dialog
    }
}

void MainControlDialog::onAddManualButton() {
    WCHAR input[256] = { 0 };
    
    if (IDOK == DialogBoxParam(GetModuleHandle(NULL), 
        MAKEINTRESOURCE(IDD_DIALOG_INPUT),
        hDlg, 
        [](HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) -> INT_PTR {
            static WCHAR* pInput = nullptr;
            
            switch (message) {
            case WM_INITDIALOG:
                pInput = (WCHAR*)lParam;
                SetFocus(GetDlgItem(hDlg, IDC_EDIT_INPUT));
                return TRUE;
                
            case WM_COMMAND:
                if (LOWORD(wParam) == IDOK) {
                    GetDlgItemText(hDlg, IDC_EDIT_INPUT, pInput, 256);
                    EndDialog(hDlg, IDOK);
                    return TRUE;
                } else if (LOWORD(wParam) == IDCANCEL) {
                    EndDialog(hDlg, IDCANCEL);
                    return TRUE;
                }
                break;
            }
            return FALSE;
        }, 
        (LPARAM)input)) {
        
        if (wcslen(input) > 0) {
            string appName = wideStringToUtf8(input);
            addAppToExclusion(appName);
        }
    }
}

bool MainControlDialog::isAppExcluded(const string& appName) {
    if (!vExclusionEnabled) return false;
    
    for (const string& excluded : vExcludedApps) {
        // Check for wildcard patterns
        if (excluded.find(".*") != string::npos) {
            string prefix = excluded.substr(0, excluded.find(".*"));
            if (appName.find(prefix) == 0) {
                return true;
            }
        } else {
            // Exact match
            if (appName == excluded) {
                return true;
            }
        }
    }
    return false;
}

vector<string> MainControlDialog::getRecentApps() {
    vector<string> allApps;
    
    // Get all running processes first
    DWORD processes[2048], cbNeeded, processCount;
    if (EnumProcesses(processes, sizeof(processes), &cbNeeded)) {
        processCount = cbNeeded / sizeof(DWORD);
        
        for (DWORD i = 0; i < processCount; i++) {
            if (processes[i] != 0) {
                HANDLE hProcess = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, FALSE, processes[i]);
                if (hProcess) {
                    WCHAR processName[MAX_PATH];
                    if (GetModuleBaseName(hProcess, NULL, processName, sizeof(processName)/sizeof(WCHAR))) {
                        string appName = wideStringToUtf8(processName);
                        
                        // Skip system processes
                        if (appName != "System" && appName != "Registry" && 
                            appName != "smss.exe" && appName != "csrss.exe" &&
                            appName != "wininit.exe" && appName != "winlogon.exe" &&
                            appName != "services.exe" && appName != "lsass.exe" &&
                            appName != "svchost.exe" && appName != "dwm.exe" &&
                            appName != "ntoskrnl.exe" && appName != "audiodg.exe" &&
                            appName != "conhost.exe" && appName != "dllhost.exe") {
                            
                            // Check if not already in list
                            if (find(allApps.begin(), allApps.end(), appName) == allApps.end()) {
                                allApps.push_back(appName);
                            }
                        }
                    }
                    CloseHandle(hProcess);
                }
            }
        }
    }
    
    // Scan common application directories for installed apps
    vector<wstring> appDirs = {
        L"C:\\Program Files\\",
        L"C:\\Program Files (x86)\\",
    };
    
    for (const wstring& dir : appDirs) {
        WIN32_FIND_DATA findData;
        wstring searchPath = dir + L"*.exe";
        HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if (!(findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                    string appName = wideStringToUtf8(findData.cFileName);
                    
                    // Skip system files and duplicates
                    if (appName != "setup.exe" && appName != "uninstall.exe" &&
                        appName != "install.exe" && appName != "update.exe" &&
                        find(allApps.begin(), allApps.end(), appName) == allApps.end()) {
                        allApps.push_back(appName);
                    }
                }
            } while (FindNextFile(hFind, &findData));
            FindClose(hFind);
        }
        
        // Also scan subdirectories (one level deep)
        wstring subSearchPath = dir + L"*";
        hFind = FindFirstFile(subSearchPath.c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    wcscmp(findData.cFileName, L".") != 0 &&
                    wcscmp(findData.cFileName, L"..") != 0) {
                    
                    wstring subDir = dir + findData.cFileName + L"\\*.exe";
                    WIN32_FIND_DATA subFindData;
                    HANDLE hSubFind = FindFirstFile(subDir.c_str(), &subFindData);
                    
                    if (hSubFind != INVALID_HANDLE_VALUE) {
                        do {
                            if (!(subFindData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY)) {
                                string appName = wideStringToUtf8(subFindData.cFileName);
                                
                                if (appName != "setup.exe" && appName != "uninstall.exe" &&
                                    appName != "install.exe" && appName != "update.exe" &&
                                    find(allApps.begin(), allApps.end(), appName) == allApps.end()) {
                                    allApps.push_back(appName);
                                }
                            }
                        } while (FindNextFile(hSubFind, &subFindData));
                        FindClose(hSubFind);
                    }
                }
            } while (FindNextFile(hFind, &findData));
            FindClose(hFind);
        }
    }
    
    // Add some common applications that might be missed
    vector<string> commonApps = {
        "notepad.exe", "calc.exe", "mspaint.exe", "wordpad.exe",
        "chrome.exe", "firefox.exe", "msedge.exe", "iexplore.exe",
        "winword.exe", "excel.exe", "powerpnt.exe", "outlook.exe",
        "cmd.exe", "powershell.exe", "powershell_ise.exe", "wt.exe",
        "code.exe", "devenv.exe", "notepad++.exe", "sublime_text.exe",
        "discord.exe", "slack.exe", "teams.exe", "zoom.exe",
        "vlc.exe", "wmplayer.exe", "spotify.exe", "itunes.exe",
        "explorer.exe", "taskmgr.exe", "regedit.exe", "msconfig.exe",
        "mstsc.exe", "winver.exe", "control.exe", "appwiz.cpl"
    };
    
    for (const string& app : commonApps) {
        if (find(allApps.begin(), allApps.end(), app) == allApps.end()) {
            allApps.push_back(app);
        }
    }
    
    // Sort alphabetically for better user experience
    sort(allApps.begin(), allApps.end());
    
    return allApps;
}

vector<string> MainControlDialog::searchApps(const string& searchTerm) {
    vector<string> results;
    vector<string> allApps = getRecentApps();
    
    if (searchTerm.empty()) {
        return allApps;
    }
    
    string lowerSearchTerm = searchTerm;
    transform(lowerSearchTerm.begin(), lowerSearchTerm.end(), lowerSearchTerm.begin(), ::tolower);
    
    for (const string& app : allApps) {
        // Search in exe filename
        string lowerApp = app;
        transform(lowerApp.begin(), lowerApp.end(), lowerApp.begin(), ::tolower);
        
        bool found = false;
        if (lowerApp.find(lowerSearchTerm) != string::npos) {
            found = true;
        }
        
        // Also search in product name (commercial name)
        if (!found) {
            wstring productName = getProductName(app);
            string productNameStr = wideStringToUtf8(productName);
            string lowerProductName = productNameStr;
            transform(lowerProductName.begin(), lowerProductName.end(), lowerProductName.begin(), ::tolower);
            
            if (lowerProductName.find(lowerSearchTerm) != string::npos) {
                found = true;
            }
        }
        
        if (found) {
            results.push_back(app);
        }
    }
    
    return results;
}

wstring MainControlDialog::findAppPath(const string& appName) {
    wstring wAppName = utf8ToWideString(appName);
    
    // Search in common directories
    vector<wstring> searchDirs = {
        L"C:\\Program Files\\",
        L"C:\\Program Files (x86)\\",
        L"C:\\Windows\\System32\\",
        L"C:\\Windows\\SysWOW64\\",
        L"C:\\Windows\\",
        L"C:\\Users\\Public\\Desktop\\",
        L"C:\\ProgramData\\Microsoft\\Windows\\Start Menu\\Programs\\"
    };
    
    for (const wstring& dir : searchDirs) {
        // Direct path
        wstring directPath = dir + wAppName;
        if (GetFileAttributes(directPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
            return directPath;
        }
        
        // For System32 and SysWOW64, don't search subdirectories (too many files)
        if (dir.find(L"System32") != wstring::npos || dir.find(L"SysWOW64") != wstring::npos || dir.find(L"Windows\\") != wstring::npos) {
            continue;
        }
        
        // Search in subdirectories for other directories
        WIN32_FIND_DATA findData;
        wstring searchPath = dir + L"*";
        HANDLE hFind = FindFirstFile(searchPath.c_str(), &findData);
        
        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    wcscmp(findData.cFileName, L".") != 0 &&
                    wcscmp(findData.cFileName, L"..") != 0) {
                    
                    wstring subPath = dir + findData.cFileName + L"\\" + wAppName;
                    if (GetFileAttributes(subPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                        FindClose(hFind);
                        return subPath;
                    }
                }
            } while (FindNextFile(hFind, &findData));
            FindClose(hFind);
        }
    }
    
    // Special handling for Visual Studio and other IDEs
    if (appName == "devenv.exe") {
        // Common Visual Studio installation paths
        vector<wstring> vsPaths = {
            L"C:\\Program Files\\Microsoft Visual Studio\\2022\\Enterprise\\Common7\\IDE\\devenv.exe",
            L"C:\\Program Files\\Microsoft Visual Studio\\2022\\Professional\\Common7\\IDE\\devenv.exe",
            L"C:\\Program Files\\Microsoft Visual Studio\\2022\\Community\\Common7\\IDE\\devenv.exe",
            L"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Enterprise\\Common7\\IDE\\devenv.exe",
            L"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Professional\\Common7\\IDE\\devenv.exe",
            L"C:\\Program Files (x86)\\Microsoft Visual Studio\\2019\\Community\\Common7\\IDE\\devenv.exe"
        };
        
        for (const wstring& vsPath : vsPaths) {
            if (GetFileAttributes(vsPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                return vsPath;
            }
        }
    }
    
    // Try to find in PATH environment variable
    WCHAR pathBuffer[32768];
    DWORD pathLen = GetEnvironmentVariable(L"PATH", pathBuffer, sizeof(pathBuffer)/sizeof(WCHAR));
    if (pathLen > 0 && pathLen < sizeof(pathBuffer)/sizeof(WCHAR)) {
        wstring pathEnv(pathBuffer);
        size_t start = 0;
        size_t end = pathEnv.find(L';');
        
        while (end != wstring::npos) {
            wstring pathDir = pathEnv.substr(start, end - start);
            if (!pathDir.empty() && pathDir.back() != L'\\') {
                pathDir += L'\\';
            }
            
            wstring fullPath = pathDir + wAppName;
            if (GetFileAttributes(fullPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                return fullPath;
            }
            
            start = end + 1;
            end = pathEnv.find(L';', start);
        }
        
        // Check last path
        if (start < pathEnv.length()) {
            wstring pathDir = pathEnv.substr(start);
            if (!pathDir.empty() && pathDir.back() != L'\\') {
                pathDir += L'\\';
            }
            
            wstring fullPath = pathDir + wAppName;
            if (GetFileAttributes(fullPath.c_str()) != INVALID_FILE_ATTRIBUTES) {
                return fullPath;
            }
        }
    }
    
    return L"";
}

HICON MainControlDialog::extractAppIcon(const string& appName) {
    wstring appPath = findAppPath(appName);
    
    if (appPath.empty()) {
        // Return default application icon
        return LoadIcon(NULL, IDI_APPLICATION);
    }
    
    // Extract icon from exe file
    HICON hIcon = NULL;
    
    // Try to extract medium-sized icon first
    SHFILEINFO sfi = { 0 };
    SHGetFileInfo(appPath.c_str(), 0, &sfi, sizeof(sfi), SHGFI_ICON);
    hIcon = sfi.hIcon;
    
    if (!hIcon) {
        // Try to extract large icon
        ExtractIconEx(appPath.c_str(), 0, &hIcon, NULL, 1);
    }
    
    if (!hIcon) {
        // Try to extract small icon as fallback
        ExtractIconEx(appPath.c_str(), 0, NULL, &hIcon, 1);
    }
    
    if (!hIcon) {
        // Return default application icon as fallback
        hIcon = LoadIcon(NULL, IDI_APPLICATION);
    }
    
    return hIcon;
}

wstring MainControlDialog::getProductName(const string& appName) {
    wstring appPath = findAppPath(appName);
    
    if (appPath.empty()) {
        // Fallback to exe name without extension
        size_t dotPos = appName.find_last_of('.');
        string baseName = (dotPos != string::npos) ? appName.substr(0, dotPos) : appName;
        return utf8ToWideString(baseName);
    }
    
    // Get version info size
    DWORD dwSize = GetFileVersionInfoSize(appPath.c_str(), NULL);
    if (dwSize == 0) {
        // Fallback to exe name without extension
        size_t dotPos = appName.find_last_of('.');
        string baseName = (dotPos != string::npos) ? appName.substr(0, dotPos) : appName;
        return utf8ToWideString(baseName);
    }
    
    // Allocate buffer for version info
    vector<BYTE> versionInfo(dwSize);
    if (!GetFileVersionInfo(appPath.c_str(), 0, dwSize, versionInfo.data())) {
        // Fallback to exe name without extension
        size_t dotPos = appName.find_last_of('.');
        string baseName = (dotPos != string::npos) ? appName.substr(0, dotPos) : appName;
        return utf8ToWideString(baseName);
    }
    
    // Try to get ProductName first
    LPWSTR productName = nullptr;
    UINT productNameLen = 0;
    if (VerQueryValue(versionInfo.data(), L"\\StringFileInfo\\040904B0\\ProductName", 
                      (LPVOID*)&productName, &productNameLen) && productName && productNameLen > 1) {
        return wstring(productName);
    }
    
    // Try alternative language code
    if (VerQueryValue(versionInfo.data(), L"\\StringFileInfo\\000004B0\\ProductName", 
                      (LPVOID*)&productName, &productNameLen) && productName && productNameLen > 1) {
        return wstring(productName);
    }
    
    // Try FileDescription as fallback
    LPWSTR fileDescription = nullptr;
    UINT fileDescLen = 0;
    if (VerQueryValue(versionInfo.data(), L"\\StringFileInfo\\040904B0\\FileDescription", 
                      (LPVOID*)&fileDescription, &fileDescLen) && fileDescription && fileDescLen > 1) {
        return wstring(fileDescription);
    }
    
    // Try alternative language code for FileDescription
    if (VerQueryValue(versionInfo.data(), L"\\StringFileInfo\\000004B0\\FileDescription", 
                      (LPVOID*)&fileDescription, &fileDescLen) && fileDescription && fileDescLen > 1) {
        return wstring(fileDescription);
    }
    
    // Final fallback to exe name without extension
    size_t dotPos = appName.find_last_of('.');
    string baseName = (dotPos != string::npos) ? appName.substr(0, dotPos) : appName;
    return utf8ToWideString(baseName);
}

INT_PTR CALLBACK MainControlDialog::appPickerDialogProc(HWND hDlg, UINT message, WPARAM wParam, LPARAM lParam) {
    static MainControlDialog* pThis = nullptr;
    static HWND hListView = nullptr;
    static HWND hSearchEdit = nullptr;
    static HIMAGELIST hImageList = nullptr;
    
    switch (message) {
    case WM_INITDIALOG: {
        pThis = (MainControlDialog*)lParam;
        hListView = GetDlgItem(hDlg, IDC_LIST_RECENT_APPS);
        hSearchEdit = GetDlgItem(hDlg, IDC_EDIT_SEARCH);
        
        // Create image list for icons (larger size)
        hImageList = ImageList_Create(32, 32, ILC_COLOR32 | ILC_MASK, 0, 100);
        ListView_SetImageList(hListView, hImageList, LVSIL_SMALL);
        
        // Setup list view columns
        LVCOLUMN lvc = { 0 };
        lvc.mask = LVCF_TEXT | LVCF_WIDTH;
        lvc.cx = 189;  // 50% of ListView width (378px / 2 = 189px each column)
        lvc.pszText = (LPWSTR)_T("Tên ứng dụng");
        ListView_InsertColumn(hListView, 0, &lvc);
        
        lvc.cx = 189;  // 50% of ListView width (378px / 2 = 189px each column)
        lvc.pszText = (LPWSTR)_T("Tên file exe");
        ListView_InsertColumn(hListView, 1, &lvc);
        
        // Set full row select and increase row height
        ListView_SetExtendedListViewStyle(hListView, LVS_EX_FULLROWSELECT | LVS_EX_GRIDLINES);
        
        // Set item height to accommodate larger icons
        HIMAGELIST hTempImageList = ImageList_Create(1, 36, ILC_COLOR, 1, 1);
        ListView_SetImageList(hListView, hTempImageList, LVSIL_STATE);
        ImageList_Destroy(hTempImageList);
        
        // Load all system apps
        if (pThis) {
            vector<string> recentApps = pThis->getRecentApps();
            for (size_t i = 0; i < recentApps.size(); i++) {
                const string& app = recentApps[i];
                wstring wApp = utf8ToWideString(app);
                
                // Extract icon and add to image list
                HICON hIcon = pThis->extractAppIcon(app);
                int iconIndex = ImageList_AddIcon(hImageList, hIcon);
                if (hIcon && hIcon != LoadIcon(NULL, IDI_APPLICATION)) {
                    DestroyIcon(hIcon);
                }
                
                LVITEM lvi = { 0 };
                lvi.mask = LVIF_TEXT | LVIF_IMAGE;
                lvi.iItem = (int)i;
                lvi.iSubItem = 0;
                lvi.iImage = iconIndex;
                
                // Get product name (commercial name) instead of exe name
                wstring productName = pThis->getProductName(app);
                lvi.pszText = (LPWSTR)productName.c_str();
                
                int index = ListView_InsertItem(hListView, &lvi);
                ListView_SetItemText(hListView, index, 1, (LPWSTR)wApp.c_str());
            }
        }
        
        SetFocus(hSearchEdit);
        return TRUE;
    }
        
    case WM_COMMAND:
        switch (LOWORD(wParam)) {
        case IDOK: {
            int selectedIndex = ListView_GetNextItem(hListView, -1, LVNI_SELECTED);
            if (selectedIndex >= 0) {
                WCHAR appName[256];
                ListView_GetItemText(hListView, selectedIndex, 1, appName, 256);
                string appNameStr = wideStringToUtf8(appName);
                
                if (pThis) {
                    pThis->addAppToExclusion(appNameStr);
                }
                EndDialog(hDlg, IDOK);
            } else {
                MessageBox(hDlg, _T("Vui lòng chọn một ứng dụng từ danh sách."), _T("OpenKey"), MB_OK | MB_ICONWARNING);
            }
            return TRUE;
        }
        case IDCANCEL:
            EndDialog(hDlg, IDCANCEL);
            return TRUE;
            
        case IDC_BUTTON_BROWSE: {
            OPENFILENAME ofn = { 0 };
            WCHAR szFile[MAX_PATH] = { 0 };
            
            ofn.lStructSize = sizeof(ofn);
            ofn.hwndOwner = hDlg;
            ofn.lpstrFile = szFile;
            ofn.nMaxFile = sizeof(szFile);
            ofn.lpstrFilter = _T("Executable Files\0*.exe\0All Files\0*.*\0");
            ofn.nFilterIndex = 1;
            ofn.lpstrFileTitle = NULL;
            ofn.nMaxFileTitle = 0;
            ofn.lpstrInitialDir = NULL;
            ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;
            
            if (GetOpenFileName(&ofn)) {
                // Extract just the filename
                wstring fullPath(szFile);
                size_t lastSlash = fullPath.find_last_of(L"\\");
                wstring fileName = (lastSlash != wstring::npos) ? fullPath.substr(lastSlash + 1) : fullPath;
                
                string appName = wideStringToUtf8(fileName);
                if (pThis) {
                    pThis->addAppToExclusion(appName);
                }
                EndDialog(hDlg, IDOK);
            }
            return TRUE;
        }
        case IDC_EDIT_SEARCH:
            if (HIWORD(wParam) == EN_CHANGE) {
                WCHAR searchText[256];
                GetDlgItemText(hDlg, IDC_EDIT_SEARCH, searchText, 256);
                string searchTerm = wideStringToUtf8(searchText);
                
                // Clear list and repopulate with search results
                ListView_DeleteAllItems(hListView);
                
                // Clear and recreate image list for search results
                if (hImageList) {
                    ImageList_RemoveAll(hImageList);
                }
                
                if (pThis) {
                    vector<string> searchResults = pThis->searchApps(searchTerm);
                    for (size_t i = 0; i < searchResults.size(); i++) {
                        const string& app = searchResults[i];
                        wstring wApp = utf8ToWideString(app);
                        
                        // Extract icon and add to image list
                        HICON hIcon = pThis->extractAppIcon(app);
                        int iconIndex = ImageList_AddIcon(hImageList, hIcon);
                        if (hIcon && hIcon != LoadIcon(NULL, IDI_APPLICATION)) {
                            DestroyIcon(hIcon);
                        }
                        
                        LVITEM lvi = { 0 };
                        lvi.mask = LVIF_TEXT | LVIF_IMAGE;
                        lvi.iItem = (int)i;
                        lvi.iSubItem = 0;
                        lvi.iImage = iconIndex;
                        
                        // Get product name (commercial name) instead of exe name
                        wstring productName = pThis->getProductName(app);
                        lvi.pszText = (LPWSTR)productName.c_str();
                        
                        int index = ListView_InsertItem(hListView, &lvi);
                        ListView_SetItemText(hListView, index, 1, (LPWSTR)wApp.c_str());
                    }
                }
            }
            return TRUE;
        }
        break;
        
    case WM_NOTIFY: {
        LPNMHDR pnmh = (LPNMHDR)lParam;
        if (pnmh->idFrom == IDC_LIST_RECENT_APPS && pnmh->code == NM_DBLCLK) {
            // Double click to select
            SendMessage(hDlg, WM_COMMAND, IDOK, 0);
            return TRUE;
        }
        break;
    }
    
    case WM_DESTROY:
        // Cleanup image list
        if (hImageList) {
            ImageList_Destroy(hImageList);
            hImageList = nullptr;
        }
        break;
    }
    return FALSE;
}
