//
//  ViewController.m
//  ModernKey
//
//  Created by Tuyen on 1/18/19.
//  Copyright © 2019 Tuyen Mai. All rights reserved.
//

#import "ViewController.h"
#import "OpenKeyManager.h"
#import "AppDelegate.h"
#import "MyTextField.h"

extern AppDelegate* appDelegate;
extern void OnSpellCheckingChanged(void);

ViewController* viewController;
extern int vFreeMark;
extern int vCheckSpelling;
extern int vUseModernOrthography;
extern int vSwitchKeyStatus;
extern int vQuickTelex;
extern int vRestoreIfWrongSpelling;
extern int vFixRecommendBrowser;
extern int vUseMacro;
extern int vUseMacroInEnglishMode;
extern int vSendKeyStepByStep;
extern int vUseSmartSwitchKey;
extern int vUpperCaseFirstChar;
extern int vTempOffSpelling;
extern int vAllowConsonantZFWJ;
extern int vQuickStartConsonant;
extern int vQuickEndConsonant;
extern int vRememberCode;
extern int vOtherLanguage;
extern int vTempOffOpenKey;
extern int vShowIconOnDock;
extern int vAutoCapsMacro;
extern int vFixChromiumBrowser;
extern int vPerformLayoutCompat;

@implementation ViewController {
    __weak IBOutlet NSButton *CustomSwitchCommand;
    __weak IBOutlet NSButton *CustomSwitchOption;
    __weak IBOutlet NSButton *CustomSwitchControl;
    __weak IBOutlet NSButton *CustomSwitchShift;
    __weak IBOutlet MyTextField *CustomSwitchKey;
    __weak IBOutlet NSButton *CustomBeepSound;
    NSArray* tabviews, *tabbuttons;
    NSRect tabViewRect;
    NSMutableArray* excludedApps;
}

- (void)viewDidLoad {
    [super viewDidLoad];
    viewController = self;
    CustomSwitchKey.Parent = self;
    
    self.appOK.hidden = YES;
    self.permissionWarning.hidden = YES;
    self.retryButton.enabled = NO;
 
    NSRect parentRect = self.viewParent.frame;
    parentRect.size.height = 580;
    self.viewParent.frame = parentRect;
    
    //set correct tabgroup
    tabviews = [NSArray arrayWithObjects:self.tabviewPrimary, self.tabviewMacro, self.tabviewSystem, self.tabviewExclusion, self.tabviewInfo, nil];
    tabbuttons = [NSArray arrayWithObjects:self.tabbuttonPrimary, self.tabbuttonMacro, self.tabbuttonSystem, self.tabbuttonExclusion, self.tabbuttonInfo, nil];
    tabViewRect = self.tabviewPrimary.frame;
    for (NSBox* b in tabviews) {
        b.frame = tabViewRect;
    }
    
    [self showTab:0];
    
    NSArray* inputTypeData = [[NSArray alloc] initWithObjects:@"Telex", @"VNI", @"Simple Telex 1", @"Simple Telex 2", nil];
    NSArray* codeData = [OpenKeyManager getTableCodes];
    
    //preset data
    [_popupInputType removeAllItems];
    [_popupInputType addItemsWithTitles:inputTypeData];
    
    [self.popupCode removeAllItems];
    [self.popupCode addItemsWithTitles:codeData];
    
    [self initKey];
    
    // Initialize excluded apps
    [self initExcludedApps];
    
    [self fillData];
    
    // set version info
    self.VersionInfo.stringValue = [NSString stringWithFormat:@"Phiên bản %@ (build %@) - Ngày cập nhật %@",
    [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleShortVersionString"],
    [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleVersion"],
    [OpenKeyManager getBuildDate]] ;
}

- (void)viewDidAppear {
    [super viewDidAppear];
    NSString* str = @"OpenKey %@ - Bộ gõ Tiếng Việt";
    self.view.window.title = [NSString stringWithFormat:str, [[NSBundle mainBundle] objectForInfoDictionaryKey: @"CFBundleShortVersionString"]];
    
    // Setup table view after view appears to ensure outlets are connected
    [self setupExclusionTableView];
}

- (void)setupExclusionTableView {
    if (self.excludedAppsTableView) {
        self.excludedAppsTableView.dataSource = self;
        self.excludedAppsTableView.delegate = self;
        [self.excludedAppsTableView reloadData];
    }
}

- (void)viewWillAppear {
    [self initKey];
}

-(void)initKey {
    dispatch_async(dispatch_get_main_queue(), ^{
        if (![OpenKeyManager initEventTap]) {
            //self.permissionWarning.hidden = NO;
            //self.retryButton.enabled = YES;
        } else {
            //self.appOK.hidden = NO;
        }
    });
}

- (void)setRepresentedObject:(id)representedObject {
    [super setRepresentedObject:representedObject];

    // Update the view, if already loaded.
}

-(void)showTab:(NSInteger)index {
    NSRect tempRect = tabViewRect;
    tempRect.origin.y = 1000;
    for (NSBox* b in tabviews) {
        [b setHidden:YES];
        b.frame = tempRect;
    }
    for (NSButton* b in tabbuttons) {
        [b setState:NSControlStateValueOff];
    }
    NSBox* b = [tabviews objectAtIndex:index];
    [b setHidden:NO];
    b.frame = tabViewRect;
    
    NSButton* button = [tabbuttons objectAtIndex:index];
    [button setState:NSControlStateValueOn];
}

- (IBAction)onTabButton:(NSButton *)sender {
    [self showTab:sender.tag];
}

- (IBAction)onInputTypeChanged:(NSPopUpButton *)sender {
    [appDelegate onInputTypeSelectedIndex:(int)[self.popupInputType indexOfSelectedItem]];
}

- (IBAction)onCodeTableChanged:(NSPopUpButton *)sender {
    [appDelegate onCodeTableChanged:(int)[self.popupCode indexOfSelectedItem]];
}

- (IBAction)onLanguageChanged:(id)sender {
    [appDelegate onInputMethodSelected];
}

- (IBAction)onRestart:(id)sender {
    self.appOK.hidden = YES;
    self.permissionWarning.hidden = YES;
    self.retryButton.enabled = NO;
    
    [self initKey];
}

- (IBAction)onFreeMark:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"FreeMark"];
    vFreeMark = (int)val;
}

- (IBAction)onModernOrthography:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"ModernOrthography"];
    vUseModernOrthography = (int)val;
}

- (IBAction)onCheckSpelling:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"Spelling"];
    vCheckSpelling = (int)val;
    [self.RestoreIfInvalidWord setEnabled:val];
    [self.AllowZWJF setEnabled:val];
    [self.TempOffSpellChecking setEnabled:val];
    OnSpellCheckingChanged();
}

- (IBAction)onShowUIOnStartup:(NSButton *)sender {
    [self setCustomValue:sender keyToSet:@"ShowUIOnStartup"];
}

- (IBAction)onRunOnStartup:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"RunOnStartup"];
    [appDelegate setRunOnStartup:val];
}

- (IBAction)onGrayIcon:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"GrayIcon"];
    [appDelegate setGrayIcon:val];
}

- (IBAction)onQuickTelex:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"QuickTelex"];
    vQuickTelex = (int)val;
}

- (IBAction)onRestoreIfInvalidWord:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"RestoreIfInvalidWord"];
    vRestoreIfWrongSpelling = (int)val;
}

- (IBAction)omTempOffSpellChecking:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"vTempOffSpelling"];
    vTempOffSpelling = (int)val;
}

- (IBAction)onAllowZFWJ:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"vAllowConsonantZFWJ"];
    vAllowConsonantZFWJ = (int)val;
}

- (IBAction)onFixRecommendBrowser:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"FixRecommendBrowser"];
    vFixRecommendBrowser = (int)val;
    [self.FixChromiumBrowser setEnabled:val];
}

- (IBAction)onControlSwitchKey:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:nil];
    vSwitchKeyStatus &= (~0x100);
    vSwitchKeyStatus |= val << 8;
    [[NSUserDefaults standardUserDefaults] setInteger:vSwitchKeyStatus forKey:@"SwitchKeyStatus"];
}

- (IBAction)onOptionSwitchKey:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:nil];
    vSwitchKeyStatus &= (~0x200);
    vSwitchKeyStatus |= val << 9;
    [[NSUserDefaults standardUserDefaults] setInteger:vSwitchKeyStatus forKey:@"SwitchKeyStatus"];
}

- (IBAction)onCommandSwitchKey:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:nil];
    vSwitchKeyStatus &= (~0x400);
    vSwitchKeyStatus |= val << 10;
    [[NSUserDefaults standardUserDefaults] setInteger:vSwitchKeyStatus forKey:@"SwitchKeyStatus"];
}

- (IBAction)onShiftSwitchKey:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:nil];
    vSwitchKeyStatus &= (~0x800);
    vSwitchKeyStatus |= val << 11;
    [[NSUserDefaults standardUserDefaults] setInteger:vSwitchKeyStatus forKey:@"SwitchKeyStatus"];
}

-(void)onMyTextFieldKeyChange:(unsigned short)keyCode character:(unsigned short)character {
    vSwitchKeyStatus &= 0xFFFFFF00;
    vSwitchKeyStatus |= keyCode;
    vSwitchKeyStatus &= 0x00FFFFFF;
    vSwitchKeyStatus |= ((unsigned int)character<<24);
    [[NSUserDefaults standardUserDefaults] setInteger:vSwitchKeyStatus forKey:@"SwitchKeyStatus"];
}

- (IBAction)onBeepSound:(NSButton *)sender {
    unsigned int val = (unsigned int)[self setCustomValue:sender keyToSet:nil];
    vSwitchKeyStatus &= (~0x8000);
    vSwitchKeyStatus |= val << 15;
    [[NSUserDefaults standardUserDefaults] setInteger:vSwitchKeyStatus forKey:@"SwitchKeyStatus"];
}

- (IBAction)onSendKeyStepByStep:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"SendKeyStepByStep"];
    vSendKeyStepByStep = (int)val;
}

- (IBAction)onPerformLayoutCompat:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"vPerformLayoutCompat"];
    vPerformLayoutCompat = (int)val;
}

- (NSInteger)setCustomValue:(NSButton*)sender keyToSet:(NSString*) key {
    NSInteger val = 0;
    if (sender.state == NSControlStateValueOn) {
        val = 1;
    } else {
        val = 0;
    }
    if (key != nil)
        [[NSUserDefaults standardUserDefaults] setInteger:val forKey:key];
    return val;
}

- (IBAction)onMacroButton:(id)sender {
    [appDelegate onMacroSelected];
}

- (IBAction)onMacroChanged:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"UseMacro"];
    vUseMacro = (int)val;
}

- (IBAction)onUseMacroInEnglishModeChanged:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"UseMacroInEnglishMode"];
    vUseMacroInEnglishMode = (int)val;
}

- (IBAction)onAutoRememberSwitchKey:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"UseSmartSwitchKey"];
    vUseSmartSwitchKey = (int)val;
}

- (IBAction)onUpperCaseFirstChar:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"UpperCaseFirstChar"];
    vUpperCaseFirstChar = (int)val;
}
- (IBAction)onQuickStartConsonant:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"vQuickStartConsonant"];
    vQuickStartConsonant = (int)val;
}

- (IBAction)onQuickEndConsonant:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"vQuickEndConsonant"];
    vQuickEndConsonant = (int)val;
}

- (IBAction)onTempOffOpenKeyByHotKey:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"vTempOffOpenKey"];
    vTempOffOpenKey = (int)val;
}

- (IBAction)onRememberTableCode:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"vRememberCode"];
    vRememberCode = (int)val;
}
- (IBAction)onOtherLanguage:(id)sender {
    
    NSInteger val = [self setCustomValue:sender keyToSet:@"vOtherLanguage"];
    vOtherLanguage = (int)val;
}


- (IBAction)onAutoCapsMacro:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"vAutoCapsMacro"];
    vAutoCapsMacro = (int)val;
}

- (IBAction)onShowIconOnDock:(id)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"vShowIconOnDock"];
    vShowIconOnDock = (int)val;
    if (!vShowIconOnDock) {
        [self.view.window close];
    }
    [appDelegate showIconOnDock:vShowIconOnDock];
}

- (IBAction)onCheckNewVersionOnStartup:(NSButton *)sender {
    NSInteger val = sender.state == NSControlStateValueOn ? 0 : 1;
    [[NSUserDefaults standardUserDefaults] setInteger:val forKey:@"DontCheckUpdate"];
}

- (IBAction)onFixChromiumBrowser:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"vFixChromiumBrowser"];
    vFixChromiumBrowser = (int)val;
}

- (IBAction)onTerminateApp:(id)sender {
    [NSApp terminate:0];
}

-(void)fillData {
    NSInteger value;
    
    NSInteger intInputMethod = [[NSUserDefaults standardUserDefaults] integerForKey:@"InputMethod"];
    if (intInputMethod == 1) {
        self.VietButton.state = NSControlStateValueOn;
    } else if (intInputMethod == 0) {
        self.EngButton.state = NSControlStateValueOn;
    }
    
    NSInteger intInputType = [[NSUserDefaults standardUserDefaults] integerForKey:@"InputType"];
    [self.popupInputType selectItemAtIndex:intInputType];
    
    NSInteger intCodeTable = [[NSUserDefaults standardUserDefaults] integerForKey:@"CodeTable"];
    [self.popupCode selectItemAtIndex:intCodeTable];
    
    //option
    NSInteger showui = [[NSUserDefaults standardUserDefaults] integerForKey:@"ShowUIOnStartup"];
    self.ShowUIButton.state = showui ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger freeMark = [[NSUserDefaults standardUserDefaults] integerForKey:@"FreeMark"];
    self.FreeMarkButton.state = freeMark ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger useModernOrthography = [[NSUserDefaults standardUserDefaults] integerForKey:@"ModernOrthography"];
    self.UseModernOrthography.state = useModernOrthography ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger spelling = [[NSUserDefaults standardUserDefaults] integerForKey:@"Spelling"];
    self.CheckSpellingButton.state = spelling ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger runOnStartup = [[NSUserDefaults standardUserDefaults] integerForKey:@"RunOnStartup"];
    self.RunOnStartupButton.state = runOnStartup ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger useGrayIcon = [[NSUserDefaults standardUserDefaults] integerForKey:@"GrayIcon"];
    self.UseGrayIcon.state = useGrayIcon ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger quicTelex = [[NSUserDefaults standardUserDefaults] integerForKey:@"QuickTelex"];
    self.QuickTelex.state = quicTelex ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger restoreIfInvalidWord = [[NSUserDefaults standardUserDefaults] integerForKey:@"RestoreIfInvalidWord"];
    self.RestoreIfInvalidWord.state = restoreIfInvalidWord ? NSControlStateValueOn : NSControlStateValueOff;
    [self.RestoreIfInvalidWord setEnabled:spelling];
    
    NSInteger tempOffSpelling = [[NSUserDefaults standardUserDefaults] integerForKey:@"vTempOffSpelling"];
    self.TempOffSpellChecking.state = tempOffSpelling ? NSControlStateValueOn : NSControlStateValueOff;
    [self.TempOffSpellChecking setEnabled:spelling];
    
    NSInteger allowZFWJ = [[NSUserDefaults standardUserDefaults] integerForKey:@"vAllowConsonantZFWJ"];
    self.AllowZWJF.state = allowZFWJ ? NSControlStateValueOn : NSControlStateValueOff;
    [self.AllowZWJF setEnabled:spelling];
    
    NSInteger fixRecommendBrowser = [[NSUserDefaults standardUserDefaults] integerForKey:@"FixRecommendBrowser"];
    self.FixRecommendBrowser.state = fixRecommendBrowser ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger useMacro = [[NSUserDefaults standardUserDefaults] integerForKey:@"UseMacro"];
    self.UseMacro.state = useMacro ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger useMacroInEnglish = [[NSUserDefaults standardUserDefaults] integerForKey:@"UseMacroInEnglishMode"];
    self.UseMacroInEnglishMode.state = useMacroInEnglish ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger sendKeySbS = [[NSUserDefaults standardUserDefaults] integerForKey:@"SendKeyStepByStep"];
    self.SendKeyStepByStep.state = sendKeySbS ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger useSmartSwitchKey = [[NSUserDefaults standardUserDefaults] integerForKey:@"UseSmartSwitchKey"];
    self.AutoRememberSwitchKey.state = useSmartSwitchKey ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger upperCaseFirstChar = [[NSUserDefaults standardUserDefaults] integerForKey:@"UpperCaseFirstChar"];
    self.UpperCaseFirstChar.state = upperCaseFirstChar ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger quickStartConsonant = [[NSUserDefaults standardUserDefaults] integerForKey:@"vQuickStartConsonant"];
    self.QuickStartConsonant.state = quickStartConsonant ? NSControlStateValueOn : NSControlStateValueOff;
    
    NSInteger quickEndConsonant = [[NSUserDefaults standardUserDefaults] integerForKey:@"vQuickEndConsonant"];
    self.QuickEndConsonant.state = quickEndConsonant ? NSControlStateValueOn : NSControlStateValueOff;
    
    value = [[NSUserDefaults standardUserDefaults] integerForKey:@"vRememberCode"];
    self.RememberTableCode.state = value ? NSControlStateValueOn : NSControlStateValueOff;
    
    value = [[NSUserDefaults standardUserDefaults] integerForKey:@"vOtherLanguage"];
    self.OtherLanguage.state = value ? NSControlStateValueOn : NSControlStateValueOff;
    
    value = [[NSUserDefaults standardUserDefaults] integerForKey:@"vTempOffOpenKey"];
    self.TempOffOpenKey.state = value ? NSControlStateValueOn : NSControlStateValueOff;
    
    value = [[NSUserDefaults standardUserDefaults] integerForKey:@"vAutoCapsMacro"];
    self.AutoCapsMacro.state = value ? NSControlStateValueOn : NSControlStateValueOff;
    
    value = [[NSUserDefaults standardUserDefaults] integerForKey:@"vShowIconOnDock"];
    self.ShowIconOnDock.state = value ? NSControlStateValueOn : NSControlStateValueOff;
    
    value = [[NSUserDefaults standardUserDefaults] integerForKey:@"DontCheckUpdate"];
    self.CheckNewVersionOnStartup.state = value ? NSControlStateValueOff :NSControlStateValueOn;
    
    value = [[NSUserDefaults standardUserDefaults] integerForKey:@"vFixChromiumBrowser"];
    self.FixChromiumBrowser.state = value ? NSControlStateValueOn : NSControlStateValueOff;
    self.FixChromiumBrowser.enabled = fixRecommendBrowser ? YES : NO;
    
    value = [[NSUserDefaults standardUserDefaults] integerForKey:@"vPerformLayoutCompat"];
    self.PerformLayoutCompat.state = value ? NSControlStateValueOn : NSControlStateValueOff;
    
    // Load exclusion settings
    value = [[NSUserDefaults standardUserDefaults] integerForKey:@"ExclusionEnabled"];
    if (self.enableExclusionButton) {
        self.enableExclusionButton.state = value ? NSControlStateValueOn : NSControlStateValueOff;
    }
    
    CustomSwitchControl.state = (vSwitchKeyStatus & 0x100) ? NSControlStateValueOn : NSControlStateValueOff;
    CustomSwitchOption.state = (vSwitchKeyStatus & 0x200) ? NSControlStateValueOn : NSControlStateValueOff;
    CustomSwitchCommand.state = (vSwitchKeyStatus & 0x400) ? NSControlStateValueOn : NSControlStateValueOff;
    CustomSwitchShift.state = (vSwitchKeyStatus & 0x800) ? NSControlStateValueOn : NSControlStateValueOff;
    CustomBeepSound.state = (vSwitchKeyStatus & 0x8000) ? NSControlStateValueOn : NSControlStateValueOff;
    [CustomSwitchKey setTextByChar:((vSwitchKeyStatus>>24) & 0xFF)];
    
}

- (IBAction)onOK:(id)sender {
    [self.view.window close];
}

- (IBAction)onDefaultConfig:(id)sender {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Bạn có chắc chắn muốn thiết lập lại cấu hình mặc định?"];
    [alert addButtonWithTitle:@"Có"];
    [alert addButtonWithTitle:@"Không"];
    [alert beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse returnCode) {
        if (returnCode == 1000) {
            [appDelegate loadDefaultConfig];
            [[NSUserDefaults standardUserDefaults] setInteger:0 forKey:@"ShowUIOnStartup"];
            self.ShowUIButton.state = NSControlStateValueOff;
            
            [[NSUserDefaults standardUserDefaults] setInteger:1 forKey:@"RunOnStartup"];
            self.RunOnStartupButton.state = NSControlStateValueOn;
        }
    }];
}

- (IBAction)onHomePageLink:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:@"https://open-key.org"]];
}

- (IBAction)onFanpageLink:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:@"https://www.facebook.com/OpenKeyVN"]];
}

- (IBAction)onEmailLink:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:@"mailto:maivutuyen.91@gmail.com"]];
}

- (IBAction)onForkerEmailLink:(id)sender {
    [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:@"mailto:vothaianh@gmail.com"]];
}

- (IBAction)onSourceCode:(id)sender {
  [[NSWorkspace sharedWorkspace] openURL: [NSURL URLWithString:@"https://github.com/vothaianh/OpenKey"]];
}

- (IBAction)onCheckNewVersionButton:(id)sender {
    self.CheckNewVersionButton.title = @"Đang kiểm tra...";
    self.CheckNewVersionButton.enabled = false;
    
    [OpenKeyManager checkNewVersion:self.view.window callbackFunc:^{
        self.CheckNewVersionButton.enabled = true;
        self.CheckNewVersionButton.title = @"Kiểm tra bản mới...";
    }];
}

#pragma mark - App Exclusion Methods

- (void)initExcludedApps {
    NSArray *savedApps = [[NSUserDefaults standardUserDefaults] arrayForKey:@"ExcludedApps"];
    if (savedApps) {
        excludedApps = [savedApps mutableCopy];
    } else {
        excludedApps = [[NSMutableArray alloc] init];
    }
    
    // Load exclusion enabled state
    NSInteger exclusionEnabled = [[NSUserDefaults standardUserDefaults] integerForKey:@"ExclusionEnabled"];
    if (self.enableExclusionButton) {
        self.enableExclusionButton.state = exclusionEnabled ? NSControlStateValueOn : NSControlStateValueOff;
    }
    
    // Initially disable remove button
    if (self.removeAppButton) {
        self.removeAppButton.enabled = NO;
    }
}

- (IBAction)onEnableExclusion:(NSButton *)sender {
    NSInteger val = [self setCustomValue:sender keyToSet:@"ExclusionEnabled"];
    // Update the global exclusion state in OpenKey engine
    [self updateExclusionState];
}

- (IBAction)onAddApp:(NSButton *)sender {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Thêm ứng dụng loại trừ"];
    [alert setInformativeText:@"Chọn cách thêm ứng dụng vào danh sách loại trừ"];
    [alert addButtonWithTitle:@"Chọn App"];
    [alert addButtonWithTitle:@"Nhập Bundle ID"];
    [alert addButtonWithTitle:@"Hủy"];
    
    [alert beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse returnCode) {
        if (returnCode == NSAlertFirstButtonReturn) {
            // Chọn App
            [self showAppPicker];
        } else if (returnCode == NSAlertSecondButtonReturn) {
            // Nhập Bundle ID
            [self showBundleIdInput];
        }
    }];
}

- (void)showAppPicker {
    NSOpenPanel *openPanel = [NSOpenPanel openPanel];
    openPanel.canChooseFiles = YES;
    openPanel.canChooseDirectories = NO;
    openPanel.allowsMultipleSelection = NO;
    openPanel.allowedFileTypes = @[@"app"];
    openPanel.directoryURL = [NSURL fileURLWithPath:@"/Applications"];
    
    [openPanel beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse result) {
        if (result == NSModalResponseOK) {
            NSURL *selectedURL = openPanel.URLs.firstObject;
            NSBundle *appBundle = [NSBundle bundleWithURL:selectedURL];
            NSString *bundleIdentifier = appBundle.bundleIdentifier;
            
            if (!excludedApps) {
                excludedApps = [[NSMutableArray alloc] init];
            }
            
            // Clean bundle identifier
            bundleIdentifier = [bundleIdentifier stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
            
            if (bundleIdentifier && bundleIdentifier.length > 0 && ![excludedApps containsObject:bundleIdentifier]) {
                [excludedApps addObject:bundleIdentifier];
                [self saveExcludedApps];
                [self.excludedAppsTableView reloadData];
                [self updateExclusionState];
            } else if (!bundleIdentifier || bundleIdentifier.length == 0) {
                NSAlert *alert = [[NSAlert alloc] init];
                [alert setMessageText:@"Không thể lấy Bundle Identifier"];
                [alert setInformativeText:@"Ứng dụng được chọn không có Bundle Identifier hợp lệ"];
                [alert addButtonWithTitle:@"OK"];
                [alert beginSheetModalForWindow:self.view.window completionHandler:nil];
            } else {
                NSAlert *alert = [[NSAlert alloc] init];
                [alert setMessageText:@"Ứng dụng đã tồn tại"];
                [alert setInformativeText:[NSString stringWithFormat:@"'%@' đã có trong danh sách", bundleIdentifier]];
                [alert addButtonWithTitle:@"OK"];
                [alert beginSheetModalForWindow:self.view.window completionHandler:nil];
            }
        }
    }];
}

- (void)showBundleIdInput {
    NSAlert *alert = [[NSAlert alloc] init];
    [alert setMessageText:@"Nhập Bundle Identifier"];
    [alert setInformativeText:@"Nhập Bundle ID của ứng dụng. Có thể sử dụng wildcard với .* \nVí dụ: com.apple.Safari hoặc com.todesktop.*"];
    [alert addButtonWithTitle:@"Thêm"];
    [alert addButtonWithTitle:@"Hủy"];
    
    NSTextField *input = [[NSTextField alloc] initWithFrame:NSMakeRect(0, 0, 300, 24)];
    input.placeholderString = @"com.example.app hoặc com.example.*";
    [alert setAccessoryView:input];
    
    [alert beginSheetModalForWindow:self.view.window completionHandler:^(NSModalResponse returnCode) {
        if (returnCode == NSAlertFirstButtonReturn) {
            NSString *bundleId = [input.stringValue stringByTrimmingCharactersInSet:[NSCharacterSet whitespaceAndNewlineCharacterSet]];
            
            if (bundleId.length == 0) {
                NSAlert *errorAlert = [[NSAlert alloc] init];
                [errorAlert setMessageText:@"Bundle Identifier không được để trống"];
                [errorAlert addButtonWithTitle:@"OK"];
                [errorAlert beginSheetModalForWindow:self.view.window completionHandler:nil];
                return;
            }
            
            if ([excludedApps containsObject:bundleId]) {
                NSAlert *errorAlert = [[NSAlert alloc] init];
                [errorAlert setMessageText:@"Bundle Identifier đã tồn tại"];
                [errorAlert setInformativeText:[NSString stringWithFormat:@"'%@' đã có trong danh sách", bundleId]];
                [errorAlert addButtonWithTitle:@"OK"];
                [errorAlert beginSheetModalForWindow:self.view.window completionHandler:nil];
                return;
            }
            
            [excludedApps addObject:bundleId];
            [self saveExcludedApps];
            [self.excludedAppsTableView reloadData];
            [self updateExclusionState];
        }
    }];
}



- (IBAction)onRemoveApp:(NSButton *)sender {
    NSInteger selectedRow = self.excludedAppsTableView.selectedRow;
    if (selectedRow >= 0 && selectedRow < excludedApps.count) {
        [excludedApps removeObjectAtIndex:selectedRow];
        [self saveExcludedApps];
        [self.excludedAppsTableView reloadData];
        [self updateExclusionState];
    }
}

- (void)saveExcludedApps {
    [[NSUserDefaults standardUserDefaults] setObject:excludedApps forKey:@"ExcludedApps"];
    [[NSUserDefaults standardUserDefaults] synchronize];
}

- (void)updateExclusionState {
    // This method will be called to update the OpenKey engine with the current exclusion list
    // Since we're checking exclusion in the OpenKeyCallback function using NSUserDefaults,
    // we don't need to do anything special here - the settings are automatically picked up
    NSLog(@"Exclusion state updated. Enabled: %ld, Apps count: %lu", 
          (long)[[NSUserDefaults standardUserDefaults] integerForKey:@"ExclusionEnabled"],
          (unsigned long)excludedApps.count);
}

- (NSString *)getAppNameFromBundleIdentifier:(NSString *)bundleIdentifier {
    // Handle wildcard patterns
    if ([bundleIdentifier hasSuffix:@".*"]) {
        NSString *prefix = [bundleIdentifier substringToIndex:bundleIdentifier.length - 2];
        NSArray *components = [prefix componentsSeparatedByString:@"."];
        if (components.count >= 2) {
            // Try to get a nice name from the bundle ID
            NSString *lastComponent = [components lastObject];
            NSString *capitalizedName = [lastComponent stringByReplacingCharactersInRange:NSMakeRange(0,1) 
                                                                               withString:[[lastComponent substringToIndex:1] uppercaseString]];
            return [NSString stringWithFormat:@"%@ (tất cả)", capitalizedName];
        }
        return [NSString stringWithFormat:@"%@ (tất cả)", prefix];
    }
    
    NSWorkspace *workspace = [NSWorkspace sharedWorkspace];
    NSString *appPath = [workspace absolutePathForAppBundleWithIdentifier:bundleIdentifier];
    if (appPath) {
        NSBundle *appBundle = [NSBundle bundleWithPath:appPath];
        NSString *appName = [appBundle objectForInfoDictionaryKey:@"CFBundleDisplayName"];
        if (!appName) {
            appName = [appBundle objectForInfoDictionaryKey:@"CFBundleName"];
        }
        if (!appName) {
            appName = [[appPath lastPathComponent] stringByDeletingPathExtension];
        }
        return appName;
    }
    return bundleIdentifier;
}

#pragma mark - NSTableViewDataSource

- (NSInteger)numberOfRowsInTableView:(NSTableView *)tableView {
    if (tableView == self.excludedAppsTableView) {
        // Ensure excludedApps is initialized
        if (!excludedApps) {
            [self initExcludedApps];
        }
        return excludedApps.count;
    }
    return 0;
}

#pragma mark - NSTableViewDelegate

- (NSView *)tableView:(NSTableView *)tableView viewForTableColumn:(NSTableColumn *)tableColumn row:(NSInteger)row {
    NSLog(@"DEBUG: viewForTableColumn called - tableView: %@, column: %@, row: %ld", tableView, tableColumn.identifier, (long)row);
    
    if (tableView == self.excludedAppsTableView) {
        // Ensure excludedApps is initialized
        if (!excludedApps) {
            [self initExcludedApps];
        }
        
        NSLog(@"DEBUG: excludedApps count: %lu", (unsigned long)excludedApps.count);
        
        if (row < excludedApps.count) {
            NSString *bundleIdentifier = excludedApps[row];
            NSString *appName = [self getAppNameFromBundleIdentifier:bundleIdentifier];
            
            NSLog(@"DEBUG: Row %ld - Bundle: %@, App: %@", (long)row, bundleIdentifier, appName);
            NSLog(@"DEBUG: Column identifier: %@", tableColumn.identifier);
            
            if ([tableColumn.identifier isEqualToString:@"ExC-Col-Name"]) {
                NSTableCellView *cellView = [tableView makeViewWithIdentifier:@"AppNameCell" owner:self];
                NSLog(@"DEBUG: Created AppNameCell: %@", cellView);
                if (cellView) {
                    cellView.textField.stringValue = appName;
                    NSLog(@"DEBUG: Set app name: %@", appName);
                } else {
                    NSLog(@"DEBUG: Failed to create AppNameCell");
                }
                return cellView;
            } else if ([tableColumn.identifier isEqualToString:@"ExC-Col-Bundle"]) {
                NSTableCellView *cellView = [tableView makeViewWithIdentifier:@"BundleIdCell" owner:self];
                NSLog(@"DEBUG: Created BundleIdCell: %@", cellView);
                if (cellView) {
                    cellView.textField.stringValue = bundleIdentifier;
                    
                    // Highlight wildcard patterns in blue
                    if ([bundleIdentifier hasSuffix:@".*"]) {
                        cellView.textField.textColor = [NSColor systemBlueColor];
                    } else {
                        cellView.textField.textColor = [NSColor controlTextColor];
                    }
                    NSLog(@"DEBUG: Set bundle ID: %@", bundleIdentifier);
                } else {
                    NSLog(@"DEBUG: Failed to create BundleIdCell");
                }
                
                return cellView;
            } else {
                NSLog(@"DEBUG: Unknown column identifier: %@", tableColumn.identifier);
            }
        } else {
            NSLog(@"DEBUG: Row %ld is out of bounds (count: %lu)", (long)row, (unsigned long)excludedApps.count);
        }
    } else {
        NSLog(@"DEBUG: Not excludedAppsTableView");
    }
    
    NSLog(@"DEBUG: Returning nil");
    return nil;
}

- (void)tableViewSelectionDidChange:(NSNotification *)notification {
    if (notification.object == self.excludedAppsTableView) {
        self.removeAppButton.enabled = (self.excludedAppsTableView.selectedRow >= 0);
    }
}

@end
