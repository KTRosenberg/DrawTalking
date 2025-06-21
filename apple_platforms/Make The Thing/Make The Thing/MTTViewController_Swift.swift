//
//  MTTViewController_Swift.swift
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/12/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//
#if os(macOS)
import AppKit
#else
import UIKit
import ARKit
#endif
import Metal
import MetalKit

import Speech
//import NaturalLanguage

public func secondsToTicks() -> Double {
    var tinfo = mach_timebase_info()
    _ = mach_timebase_info(&tinfo)
    let timecon = Double(tinfo.denom) / Double(tinfo.numer)
    return timecon * 1_000_000_000
}

#if !os(macOS)
public class UIButtonToggle : UIButton
{
    public var toggleStateOn : Bool = false
    
    public override init(frame: CGRect) {
        super.init(frame: frame)
    }

    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }
}
#else
public class NSButtonToggle : NSButton
{
    public var toggleStateOn : Bool = false
    
    public override init(frame: CGRect) {
        super.init(frame: frame)
    }
    
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }
}
#endif


#if !os(macOS)
public class UITextViewTouchable : UITextView {
    public var isTouched = false

    override public func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        autoreleasepool {
            super.touchesBegan(touches, with: event)
            self.isTouched = true
        }
    }
    override public func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        autoreleasepool {
            super.touchesMoved(touches, with: event)
            self.isTouched = true
        }
    }
    override public func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        autoreleasepool {
            super.touchesEnded(touches, with: event)
            self.isTouched = false
        }
    }
    override public func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        autoreleasepool {
            super.touchesBegan(touches, with: event)
            self.isTouched = false
        }
    }
}
#endif


#if !os(macOS)
public class Debug_UI {
    var debugLogButton: UIButtonToggle!
    var textView: UITextViewTouchable!
    var shareButton: UIButton!
    var speechRecognitionButton: UIButtonToggle!
    var drawButton: UIButtonToggle!
    var brushButton: UIButton!
    var clearButton: UIButton!
    
    public struct Out_Redirect {
        public var pipe : Pipe!
        public var saved_stdout : int32!
        
        public var swap_descriptor : int32!
        
        public init() {
            self.pipe = Pipe()
        }
    }
    public var out_redirect : Out_Redirect!
    
    var views : [UIView]!
    
    var parentView : MTTMainView!
    
    public func attach(to: MTTMainView) {
            
        self.parentView = to
        
        self.views = [UIView]()

        
        let yheight = 16
        let xwidth = 75
        let pad = 4
        var xpos = Int(parentView.bounds.maxX) - xwidth
        let ypos = Int(Int(parentView.bounds.maxY) - yheight)
        
        
        
        
        
        do {
            self.debugLogButton = UIButtonToggle(type: .roundedRect)
            Button_init(button: self.debugLogButton, title: "show log", color: UIColor.systemFill, xpos: xpos, ypos: ypos, textColor: UIColor.systemOrange, width: xwidth, height: yheight)
            xpos -= xwidth

            self.debugLogButton.addTarget(self, action: #selector(self.debugButtonPressed), for: .touchUpInside)

            views.append(self.debugLogButton)
        
            self.textView = UITextViewTouchable(frame: CGRect(x: -xwidth, y: -36 * 4 - pad, width: 150, height: 150))
            self.textView.textColor = UIColor.systemOrange
            self.textView.isEditable = false
            self.textView.font = UIFont.monospacedSystemFont(ofSize: 14, weight: UIFont.Weight.medium)
            self.textView.layer.borderColor = UIColor.gray.cgColor
            self.textView.layer.borderWidth = 1
            self.debugLogButton.addSubview(self.textView)
            self.textView.isHidden = true
            
            
        }
        if true {
            do {
                self.shareButton = UIButton(type: .roundedRect)
                Button_init(button: self.shareButton, title: "share", color: UIColor.systemFill, xpos: xpos, ypos: ypos, textColor: UIColor.systemOrange, width: xwidth, height: yheight)
                
                
                self.shareButton.addTarget(self, action: #selector(self.share), for: .touchUpInside)
                
                views.append(self.shareButton)
            }
        }
        
        for view in self.views {
            to.addSubview(view)
        }
    }
    
    public func Button_init(button : UIButton, title : String, color : UIColor, xpos : Int, ypos : Int, textColor : UIColor = UIColor.systemOrange, width: Int = 75, height: Int = 40) {
        
        button.autoresizingMask = [.flexibleWidth, .flexibleHeight]
        button.setTitle(title, for: .normal)
        button.titleLabel?.font = UIFont.systemFont(ofSize: 12, weight: UIFont.Weight.light)
        button.backgroundColor = color
        button.setTitleColor(textColor, for: .normal)
        button.layer.borderWidth = 1
        button.layer.borderColor = UIColor.darkGray.cgColor
        
        button.frame = CGRect(x: xpos, y: ypos, width: width, height: height)
    }
    
    
    init() {
  
//        if ((false)) {
//        do {
//            self.speechRecognitionButton = UIButtonToggle(type: .roundedRect)
//            Button_init(button: self.speechRecognitionButton, title: "enable speech rec", color: UIColor.systemFill, xpos: xpos, ypos: ypos)
//            xpos += xwidth + pad
//            
//            self.speechRecognitionButton.addTarget(self, action: #selector(self.speechButtonPressed(_:)), for: .touchUpInside)
//            
//            views.append(self.speechRecognitionButton)
//        }
//        }
//        
//        if ((false)) {
//        do {
//            self.drawButton = UIButtonToggle(type: .roundedRect)
//            Button_init(button: self.drawButton, title: "perform", color: UIColor.systemFill, xpos: xpos, ypos: ypos)
//            xpos += xwidth + pad
//            
//            self.drawButton.addTarget(self, action: #selector(self.drawButtonPressed(_:)), for: .touchUpInside)
//            
//            views.append(self.drawButton)
//        }
//        }
//        if ((false)) {
//        do {
//            
//            self.clearButton = UIButton(type: .roundedRect)
//            Button_init(button: self.clearButton,
//                title:
//                "clear",
//                
//                color: UIColor.systemFill, xpos: xpos, ypos: ypos, textColor: UIColor.systemOrange)
//            xpos += xwidth + pad
//            
//            self.clearButton.addTarget(self, action: #selector(self.clearButtonPressed(_:)), for: .touchUpInside)
//            
//            views.append(self.clearButton)
//        }
//        }
    }
    
    @objc public func share(_ sender: Any) {
        self.parentView.vc.share(sender)
    }
    
    @objc public func debugButtonPressed(_ sender: Any) {
        self.debugLogButton.toggleStateOn = !self.debugLogButton.toggleStateOn
        if self.debugLogButton.toggleStateOn {
            self.debugLogButton.setTitle("hide log", for: .normal)
            self.textView.isHidden = false
            
            if self.out_redirect == nil {
                self.out_redirect = Out_Redirect()
            }
            
            openVisualConsole()
        } else {
            self.debugLogButton.setTitle("show log", for: .normal)
            self.textView.isHidden = true
            
            closeVisualConsole()
        }
    }
    @objc public func speechButtonPressed(_ sender: Any) {
        self.speechRecognitionButton.toggleStateOn = !self.speechRecognitionButton.toggleStateOn
        if self.speechRecognitionButton.toggleStateOn {
            self.speechRecognitionButton.setTitle("disable speech rec", for: .normal)
        } else {
            self.speechRecognitionButton.setTitle("enable speech rec", for: .normal)
        }
    }
    @objc public func drawButtonPressed(_ sender: Any) {
        self.drawButton.toggleStateOn = !self.drawButton.toggleStateOn
        if self.drawButton.toggleStateOn {
            self.drawButton.setTitle("draw", for: .normal)
        } else {
            self.drawButton.setTitle("perform", for: .normal)
        }
    }
    
    @objc public func clearButtonPressed(_ sender: Any) {
        // self.vth.map = [UITouch : Touch_Info]()
        guard let pView = self.parentView else {
            return
        }
    }
    
    @objc public func closeVisualConsole() {
        autoreleasepool {
            setvbuf(stdout, nil, _IONBF, 0)
            dup2(out_redirect.saved_stdout,
                STDOUT_FILENO)
            close(out_redirect.saved_stdout)
        }
    }
    @objc public func openVisualConsole() {
        autoreleasepool {
            out_redirect.saved_stdout = dup(STDOUT_FILENO)
            
            setvbuf(stdout, nil, _IONBF, 0)
            dup2(out_redirect.pipe.fileHandleForWriting.fileDescriptor,
                STDOUT_FILENO)
            // listening on the readabilityHandler
            out_redirect.pipe.fileHandleForReading.readabilityHandler = {
                [weak self] handle in
            
                autoreleasepool {
                    let data = handle.availableData
                    let str = String(data: data, encoding: .utf8) ?? "<Non-ascii data of size\(data.count)>\n"
                    
                    DispatchQueue.main.async {
                        
                        autoreleasepool {

                            self?.textView.text += str
                            let textView = self!.textView!
                            var len = textView.text.count

                            let cap = 4096
                            if len > cap {

                                let sub = self?.textView.text.suffix(cap)
                                self?.textView.text = String(sub!)
                                len = cap
                            }
                            if !textView.isTouched {
                                textView.scrollRangeToVisible(NSRange(location: len - 1, length: 0))
                            }
                            
                        }
                        
                    }
                }
            }
            
        }
    }
}
#endif

@objc public class Application_Launch_Configuration : NSObject {
    @objc public var serverIP   : NSString!
    @objc public var serverPort : NSNumber!
    @objc public var systemDirectory : NSString = ""
    @objc public var loadConfigOnStart : Bool = true
}

public class MTTViewController_Swift: MTTViewController {
    #if !os(macOS)
    var mainView : MTTMainView!
    @objc public func get_main_view() -> UIView {
        return self.mainView
    }
    #else
    var mainView : MTTMainView_macos!
    @objc public func get_main_view() -> NSView {
        return self.mainView
    }
    
    
    @objc func onUseConfigCheck(button : NSButtonToggle) {
        self.appConfig.loadConfigOnStart = button.state == NSControl.StateValue.on ? true : false
        let defaults = UserDefaults.standard
        defaults.set(self.appConfig.loadConfigOnStart, forKey: "use config")
    }
    
    #endif
    
    //var displayLink: CADisplayLink!
    
    
    #if !os(macOS)
    
    var debugMenu : Debug_UI!
    public override var prefersHomeIndicatorAutoHidden : Bool {
        return true;
    }
    

    public override var preferredScreenEdgesDeferringSystemGestures: UIRectEdge {
        return .all  //Or, return .all to "disable" the control center as well
    }
    #endif
    //public override var childForHomeIndicatorAutoHidden: UIViewController? {
       // return nil
    //}

    public var appConfig : Application_Launch_Configuration = Application_Launch_Configuration()
    
    @objc public func get_application_launch_configuration() -> Application_Launch_Configuration {
        return self.appConfig
    }
    
    private func onConfig() {
        #if !os(macOS)
        let alert = UIAlertController(title: "Configuration", message:
        "DrawTalking\nCreated by Karl Toby Rosenberg", preferredStyle: UIAlertController.Style.alert
        )
        
        let serverIPAddressKey = "Speech Server IP Address"
        let serverPortKey = "Speech Server Port"
        
        let submitAction = UIAlertAction(title: "Submit", style: .default) { (action) -> Void in
            let IPTextField = alert.textFields![0] as UITextField
            let portTextField = alert.textFields![1] as UITextField
            
            let defaults = UserDefaults.standard

            defaults.set(IPTextField.text,   forKey: serverIPAddressKey)
            defaults.set(portTextField.text, forKey: serverPortKey)
            
            self.appConfig.serverIP   = NSString(utf8String: IPTextField.text ?? "")
            self.appConfig.serverPort = NSNumber(value: Int(portTextField.text ?? "") ?? 0)
            
            self.onAppearance()
        }
        alert.addAction(submitAction)
        
        
        let submitWithDefaultsAction = UIAlertAction(title: "Submit Default Configuration", style: .default) { (action) -> Void in
            let IPTextField = alert.textFields![0] as UITextField
            let portTextField = alert.textFields![1] as UITextField
            
            let defaults = UserDefaults.standard

            defaults.set(IPTextField.text,   forKey: serverIPAddressKey)
            defaults.set(portTextField.text, forKey: serverPortKey)
            
            self.appConfig.serverIP   = DEFAULT_IP_ADDRESS_HOST as NSString
            self.appConfig.serverPort = NSNumber(value: DEFAULT_PORT_HOST)
            
            self.onAppearance()
        }
        alert.addAction(submitWithDefaultsAction)
        
//        let submitLocalConfigAction = UIAlertAction(title: "Submit Local Test Configuration", style: .default) { (action) -> Void in
//            let IPTextField = alert.textFields![0] as UITextField
//            let portTextField = alert.textFields![1] as UITextField
//
//            let defaults = UserDefaults.standard
//
//            defaults.set(IPTextField.text,   forKey: "Speech Server IP Address")
//            defaults.set(portTextField.text, forKey: "Speech Server Port")
//
//            self.appConfig.serverIP   = DEFAULT_IP_ADDRESS_HOST as NSString//NSString(utf8String: IPTextField.text ?? "")
//            self.appConfig.serverPort = NSNumber(value: DEFAULT_PORT_HOST)//NSNumber(value: Int(portTextField.text ?? "") ?? 0)
//
//            self.onAppearance()
//        }
//        alert.addAction(submitLocalConfigAction)
        
//        alert.addAction(saveDefaultAction)
        alert.addTextField(configurationHandler: {(textField: UITextField!) in
            let defaults = UserDefaults.standard

            let defaultValue = defaults.string(forKey: serverIPAddressKey)
            textField.placeholder = "Server IP Address"
            //textField.text = textField.placeholder

            if defaultValue != nil {
                textField.text = defaultValue
            }
        })
        alert.addTextField(configurationHandler: {(textField: UITextField!) in
            let defaults = UserDefaults.standard

            let defaultValue = defaults.string(forKey: serverPortKey)
            textField.placeholder = "Server Port"
            //textField.text = textField.placeholder

            if defaultValue != nil {
                textField.text = defaultValue
            }
        })
        
        self.present(alert, animated: true, completion: nil)
        
        #else
//        let a = NSAlert()
//        a.messageText = "Configuration"
//        a.addButton(withTitle: "Save")
//        a.addButton(withTitle: "Cancel")
//
//        let inputTextField = NSTextField(frame: NSRect(x: 0, y: 0, width: 300, height: 24))
//        inputTextField.placeholderString = "Enter string"
//        a.accessoryView = inputTextField
//
//        a.beginSheetModal(for: NSApplication.shared.windows[0], completionHandler: { (modalResponse) -> Void in
//            if modalResponse == NSApplication.ModalResponse.alertFirstButtonReturn {
//                let enteredString = inputTextField.stringValue
//                print("Entered string = \"\(enteredString)\"")
//            }
//
//            self.onAppearance()
//        })
        
        let a = NSAlert()
        a.messageText = "Configuration"

        let stackViewer = NSStackView(frame: NSRect(x: 0, y: 0, width: 300, height: 96))
        
        let defaults = UserDefaults.standard
        
        let serverIPInputTextField = NSTextField(frame: NSRect(x: 0, y: 72, width: 300, height: 24))
        
        let serverIPAddressKey = "Speech Server IP Address"
        serverIPInputTextField.placeholderString = "Server IP Address"
        let defaultIPValue   = defaults.string(forKey: serverIPAddressKey)
        if defaultIPValue != nil {
            serverIPInputTextField.stringValue = defaultIPValue!
        }
        
        let serverPortKey = "Speech Server Port"
        let serverPortInputTextField = NSTextField(frame: NSRect(x: 0, y: 48, width: 300, height: 24))
        serverPortInputTextField.placeholderString = "Server Port"
        let defaultPortValue = defaults.string(forKey: serverPortKey)
        if defaultIPValue != nil {
            serverPortInputTextField.stringValue = defaultPortValue!
        }
        
        let systemDirectoryKey = "System Directory"
        let systemDirectoryTextField = NSTextField(frame: NSRect(x: 0, y: 24, width: 300, height: 24))
        let defaultSystemDirectory = "Documents/prog"
        systemDirectoryTextField.placeholderString = "System Directory : Documents/prog"
        let defaultSystemDirectoryValue = defaults.string(forKey: systemDirectoryKey)
        if defaultSystemDirectoryValue != nil {
            systemDirectoryTextField.stringValue = defaultSystemDirectoryValue!
        }
        
        stackViewer.orientation = .vertical
        
        
        stackViewer.addSubview(serverIPInputTextField)
        stackViewer.addSubview(serverPortInputTextField)
        stackViewer.addSubview(systemDirectoryTextField)
        do {
            let button = NSButtonToggle(checkboxWithTitle: "use config", target: self, action: #selector(onUseConfigCheck))
            stackViewer.addSubview(button)
            
            let defaultOnState = defaults.object(forKey: "use config")
            if defaultOnState == nil {
                defaults.set(true, forKey: "use config")
                self.appConfig.loadConfigOnStart = true
            } else {
                self.appConfig.loadConfigOnStart = (defaultOnState as? Bool)!
            }
            button.toggleStateOn = self.appConfig.loadConfigOnStart
            button.state = self.appConfig.loadConfigOnStart ?
                            NSControl.StateValue.on :
                            NSControl.StateValue.off
        }
        
        
        a.accessoryView = stackViewer
        
        a.addButton(withTitle: "Submit")
        a.addButton(withTitle: "Submit Default Configuration")
        a.addButton(withTitle: "Submit Local Host Configuration")
        a.addButton(withTitle: "Exit")
        

        a.beginSheetModal(for: NSApplication.shared.windows[0], completionHandler: { (modalResponse) -> Void in
            if modalResponse == NSApplication.ModalResponse.alertFirstButtonReturn {
                let IPTextField = serverIPInputTextField
                let portTextField = serverPortInputTextField
                
                
                
                let defaults = UserDefaults.standard
                
                defaults.set(IPTextField.stringValue,   forKey: serverIPAddressKey)
                defaults.set(portTextField.stringValue, forKey: serverPortKey)
                defaults.set(systemDirectoryTextField.stringValue, forKey: systemDirectoryKey)
                let strValIP = IPTextField.stringValue
                let strValPort = portTextField.stringValue
                self.appConfig.serverIP   = NSString(utf8String:  strValIP)
                self.appConfig.serverPort = NSNumber(value: Int(strValPort ) ?? 0)
                self.appConfig.systemDirectory = NSString(utf8String: systemDirectoryTextField.stringValue) ?? defaultSystemDirectory as NSString
                if self.appConfig.serverIP.isEqual(to: "") {
                    self.appConfig.serverIP   = DEFAULT_IP_ADDRESS_HOST as NSString
                }
                if self.appConfig.serverPort.isEqual(to: "") {
                    self.appConfig.serverPort = NSNumber(value: DEFAULT_PORT_HOST)
                }
                if self.appConfig.systemDirectory.isEqual(to: "") {
                    self.appConfig.systemDirectory = defaultSystemDirectory as NSString
                }
                
            } else if modalResponse == .alertSecondButtonReturn {
                let IPTextField = serverIPInputTextField
                let portTextField = serverPortInputTextField
                
                
                let defaults = UserDefaults.standard
                
                let strValIP = IPTextField.stringValue
                let strValPort = portTextField.stringValue
                
                defaults.set(strValIP,   forKey: serverIPAddressKey)
                defaults.set(strValPort, forKey: serverPortKey)
                defaults.set(systemDirectoryTextField.stringValue, forKey: systemDirectoryKey)
                
                self.appConfig.serverIP   = DEFAULT_IP_ADDRESS_HOST as NSString
                self.appConfig.serverPort = NSNumber(value: DEFAULT_PORT_HOST)
                self.appConfig.systemDirectory = defaultSystemDirectory as NSString
            } else if modalResponse == .alertThirdButtonReturn {
                self.appConfig.serverIP   = "localhost"
                self.appConfig.serverPort = NSNumber(value: DEFAULT_PORT_HOST)
                self.appConfig.systemDirectory = systemDirectoryTextField.stringValue as NSString
                if self.appConfig.serverPort.isEqual(to: "") {
                    self.appConfig.serverPort = NSNumber(value: DEFAULT_PORT_HOST)
                }
                if self.appConfig.systemDirectory.isEqual(to: "") {
                    self.appConfig.systemDirectory = defaultSystemDirectory as NSString
                }
            } else if modalResponse.rawValue == NSApplication.ModalResponse.alertFirstButtonReturn.rawValue + 3 {
                NSApp.terminate(nil)
            }
                            

            self.onAppearance()
        })

        #endif
    }
    
    public func onAppearance() {

        super.post_init()
        
        return;
        
//        self.debugMenu = Debug_UI()
//        self.debugMenu.attach(to: self.mainView)
    }
    
    // first-time request access
    #if !os(macOS)
    public func onPermissions(action: UIAlertAction) {
        var permissionsPending = 2
        SFSpeechRecognizer.requestAuthorization { authStatus in
            OperationQueue.main.addOperation {
                switch authStatus {
                case .authorized:
                    break
                case .denied:
                    break
                case .restricted:
                    break
                case .notDetermined:
                    break
                default:
                    break
                }
                
                permissionsPending -= 1

                do {
                    let recordingSession = AVAudioSession.sharedInstance()

                    try recordingSession.setCategory(.record, mode: .default)
                    try recordingSession.setActive(true)
                    recordingSession.requestRecordPermission() { [unowned self] allowed in
                        DispatchQueue.main.async {
                            do { try recordingSession.setActive(false, options: .notifyOthersOnDeactivation) } catch { print("failed to de-initialize recording session")}
                            
                            if (permissionsPending == 0) {
                                self.onConfig()
                            }
                        }
                    }
                } catch {
                    print("failed to initialize recording")
                    if (permissionsPending == 0) {
                        self.onConfig()
                    }
                    
                }

            }
        }
        
        
        
        PHPhotoLibrary.requestAuthorization(for: .readWrite) { /*[unowned self]*/ (status) in
            DispatchQueue.main.async { /*[unowned self] in */
                permissionsPending -= 1
                switch status {
                case .notDetermined:
                    print("PH library notDetermined")
                   //onNotDetermined(onDeniedOrRestricted, onAccessHasBeenGranted)
                    break
                case .denied:
                    print("PH library denied")
                   //onDeniedOrRestricted()
                    break
                case .restricted:
                    print("PH library restricted")
                    break
                case .authorized:
                    print("PH library authorized")
                   //onAccessHasBeenGranted()
                    break
                case .limited:
                    print("PH library limited")
                  //onAccessHasBeenGranted()
                    break
                @unknown default:
                   fatalError("PHPhotoLibrary - \"Unknown case\"")
                }
                
                if (permissionsPending == 0) {
                    self.onConfig()
                }
            }
        }
         
    }
    #else
    public func onPermissions()
    {
        SFSpeechRecognizer.requestAuthorization { authStatus in
            OperationQueue.main.addOperation {
                switch authStatus {
                case .authorized:
                    break
                case .denied:
                    break
                case .restricted:
                    break
                case .notDetermined:
                    break
                default:
                    break
                }

                self.onConfig()
            }
        }
    }
    #endif
    
    func onAppearance(_ animated: Bool) {
        if super.is_init {
            return
        }
        
        #if !os(macOS)
        super.viewDidAppear(animated)
        #else
        super.viewDidAppear()
        #endif
        
        let defaults = UserDefaults.standard
        // create the alert
        if defaults.bool(forKey: "hasLaunchedBefore") {
            self.onConfig()
            
            return
        }
        
        defaults.set(true, forKey: "hasLaunchedBefore")
        
        
        #if !os(macOS)
        let alert = UIAlertController(title: "Welcome!", message: "This project is associated with the work \"DrawTalking: Building Interactive Worlds by Sketching and Speaking,\" published at ACM UIST '24. ( https://dl.acm.org/doi/10.1145/3654777.3676334 ) The work is licensed under the Creative Commons Attribution-NonCommercial (CC-BY-NC) license (with possibility for other licensing on a case-by-case basis). Privacy Note: No personal data are collected. Your speech is processed and recognized into text entirely on-device. The system uses a local network by default to do additional required processing on a server that the user runs, ideally on a local, secure machine. No data are sent to external services, provided the user doesn't extend the server to use such services.\n Keep in-mind that this is prototype software for research and personal use. Enjoy!", preferredStyle: UIAlertController.Style.alert)
        
        // add an action (button)
        alert.addAction(UIAlertAction(title: "Let's go!", style: UIAlertAction.Style.default, handler: self.onPermissions(action:)))
        
        // show the alert
        self.present(alert, animated: true, completion: nil)
        #else
        self.onPermissions()
        #endif
    }

    #if !os(macOS)
    @objc public override func viewDidAppear(_ animated: Bool) {
        self.onAppearance(animated);
    }
    #else
    @objc public override func viewDidAppear() {
        self.onAppearance(false);
    }
    #endif
    
    @objc public override func viewDidLoad() {
        super.viewDidLoad()
                
        #if !os(macOS)
        self.mainView = self.view as? MTTMainView
        self.mainView.vc = self;
        #else
        self.mainView = self.view as? MTTMainView_macos
        self.mainView.vc = self;
        #endif
    }
    #if !os(macOS)
    override public var supportedInterfaceOrientations: UIInterfaceOrientationMask {
        get {
          //  return UIInterfaceOrientationMask.landscapeRight.union(UIInterfaceOrientationMask.landscapeLeft)
            
            UIInterfaceOrientationMask.all
        }
    }
    #endif
    

    /*
    // MARK: - Navigation

    // In a storyboard-based application, you will often want to do a little preparation before navigation
    public override func prepare(for segue: UIStoryboardSegue, sender: Any?) {
        // Get the new view controller using segue.destination.
        // Pass the selected object to the new view controller.
    }
    */

}
