//
//  speech_recognition.swift
//  make_the_thing_ios
//
//  Created by Toby Rosenberg on 4/9/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

import Foundation
import Speech
import Accelerate
import AVFoundation


public typealias ResultHandler = (_ result: String, _ user_data: UnsafeRawPointer?) -> Bool;

@objc final public class SpeechRecognition : Speech_Recognition_ObjC {
    
    static let USE_ONDEVICE_RECOGNITION = true
        
    override init() {
        super.init();
        super.speechRecognizer = nil;
        super.recognitionRequest = nil;
        super.recognitionTask = nil;
        super.audioEngine = AVAudioEngine()
        super.locale = Locale(identifier: /*"de-DE"*/"en_US")
    }
    
    @objc public static func staticfunc(sr : SpeechRecognition) {
        
    }
    
//    public var speechRecognizer: SFSpeechRecognizer!
//    public var recognitionRequest: SFSpeechAudioBufferRecognitionRequest!
//    public var recognitionTask: SFSpeechRecognitionTask?
//    public var audioEngine = AVAudioEngine()
    
    
    public var latestHypothesis : String!
    public var latestResult : String!
    
    //public var speechSynth : AVSpeechSynthesizer!
    
//    public var initTime : Float64 = 0
    public var initTimeHost : UInt64 = 0
//    public var initTimeHostFirst : UInt64 = 0
//    public var runningHostTime : UInt64 = 0
//    public var initTimeProcess : Float64 = 0
    
    public static let logger : Logger = Logger()
    
    func endAudio() {
        self.recognitionRequest.endAudio()
        self.recognitionRequest = nil
    }
    
    @objc func enable() {
        if !audioEngine.isRunning {
            begin()
        }
    }
    
    
    @objc public func restart() {
        // restart while already running should
        // do nothing
        if audioEngine.isRunning {
            return
        }
        
        self.is_active = true
        //self.transcriptInProgress = false
//        if audioEngine.isRunning {
//            return
//        }
        //self.enqueue_complete_task_event(TASK_COMPLETION_REASON_ENABLE)
        
//        self.setCompletionReasonFor(self.recognitionTask, with: TASK_COMPLETION_REASON_ENABLE)
        super.set_speech_state(SPEECH_STATE_TALKING_UNLIKELY)
        if recognitionRequest != nil && recognitionTask != nil {
            print("WARNING: recognitionTask and recognitionRequest should be nil")
            
            return
        }
        
        self.enqueue_complete_task_event(TASK_COMPLETION_REASON_ENABLE)
    }
    
    @objc public func disable() {
        self.is_active = false
        if audioEngine.isRunning {
            audioEngine.pause()
            //audioEngine.stop()
            //audioEngine.inputNode.removeTap(onBus: 0)
            //wantToDisable = true
            //recognitionTask = nil
            //assert(self.recognitionTask != nil)
//            self.setCompletionReasonFor(self.recognitionTask, with: TASK_COMPLETION_REASON_DISABLE)
            
            if recognitionRequest != nil {
                self.endAudio()
            }
            
            //self.transcriptInProgress = false
            
            
//            if recognitionTask != nil {
//                recognitionTask!.finish()
//            }
        }
        
        
        
        super.set_speech_state(SPEECH_STATE_TALKING_UNLIKELY)
    }
    
    @objc public func restartForCommand() {
        if (!audioEngine.isRunning) {
            return;
        }
        
        audioEngine.pause()
        //self.transcriptInProgress = false
        
        //assert(self.recognitionTask != nil)
        if (self.recognitionTask != nil) {
            self.setCompletionReasonFor(self.recognitionTask, with: TASK_COMPLETION_REASON_COMMAND_COMPLETE)
        }
        
        if recognitionRequest != nil {
            self.endAudio()
        }
        super.set_speech_state(SPEECH_STATE_TALKING_UNLIKELY)
        
        //self.isPlaying = false
//        self.isPlaying = true
//
//        self.doTaskNoException(
//            supportsOnDeviceRecognition: self.speechRecognizer.supportsOnDeviceRecognition
//        )
    }
    
    @objc public func restartForDiscard() {
        if (!audioEngine.isRunning) {
            return;
        }
        
        audioEngine.pause()
        
        assert(self.recognitionTask != nil)
        if recognitionTask != nil {
            self.setCompletionReasonFor(self.recognitionTask, with: TASK_COMPLETION_REASON_DISCARD)
        }
        if recognitionRequest != nil {
            self.endAudio()
        }
        super.set_speech_state(SPEECH_STATE_TALKING_UNLIKELY)
        
//        self.doTaskNoException(
//            supportsOnDeviceRecognition: self.speechRecognizer.supportsOnDeviceRecognition
//        )
    }
    
//    @objc public override func split() -> Bool {
//        return super.split()
//    }
    
    @objc public override func send_override(_ msg: String!, withMode mode: Int32) -> Bool {
        if (!super.send_override(msg, withMode: mode)) {
            return false
        }
        
        // hypothesis / update
        if mode == 0 {
            //self.modificationCount += 1
        } else {
            self.recognitionID += 1
        
            self.modificationCount = 1
        }
        
        return true
    }
    
    @objc public override func discard() -> Bool {
        
        super.discard()
        
        return true
    }
    
    var deferredDisableAndRestartActive = false
    var transcriptInProgress = false
    
    @objc func splitDeferred() {
        self.deferredDisableAndRestartActive = true;
    }
    
    
    
//    @objc public override func set_speech_state(_ state: SPEECH_STATE) {
//        super.set_speech_state(state)
//        if (self.get_speech_state() == SPEECH_STATE_TALKING_UNLIKELY) {
//            if (self.deferredDisableAndRestartActive) {
//                self.deferredDisableAndRestartActive = false
//                _ = self.split()
////                self.disableAndRestart()
//            }
//
//        }
//    }
//    
    @objc public func transcriptIsInProgress() -> Bool {
        return self.transcriptInProgress
    }
    
    
    
//    @objc func clear() {
//        speechRecognizer.delegate = nil;
//        if self.audioEngine.isRunning {
//            audioEngine.stop()
//            recognitionRequest?.endAudio()
//            self.isPlaying = false
//        }
//        self.alreadyStarted = false
//        recognitionTask?.cancel()
//        audioEngine.inputNode.removeTap(onBus: 0)
//    }
    
    @objc func doTaskNoException(supportsOnDeviceRecognition: Bool) {
        do {
            try self.doTask(
                supportsOnDeviceRecognition: self.speechRecognizer.supportsOnDeviceRecognition
            )
            
        } catch {
            print("doTask error")
        }
    }
    
    @objc func run() {
        begin()
    }
    
    public var resultHandler : ResultHandler = defaultHandler
    public var user_data : UnsafeRawPointer?

    
    @objc public func getActiveRecognitionID() -> usize {
        return self.recognitionID
    }
    
    @objc public func getActiveRecognitionModificationCount() -> usize {
        return self.modificationCount
    }

    
    @objc public func setCallback(_ handler: @escaping ResultHandler, _ user_data : UnsafeRawPointer?) {
        self.resultHandler = handler
        self.user_data = user_data;
        
        print("Speech_Recognition swift, setting callback", handler, user_data ?? "no user_data");
    }
    
    @objc static func defaultHandler(_ result: String, _ user_data: UnsafeRawPointer?) -> Bool {
        print(result)
        return true
    }
    
    @objc public func systemUptime(transcriptTimestamp : float64) -> float64
    {
        return transcriptTimestamp
    }
    
    public override func speechRecognitionDidDetectSpeech(_ task: SFSpeechRecognitionTask) {
        //        print("DID DETECT SPEECH")
        DispatchQueue.main.async {
        
            self.transcriptInProgress = true
            
            #if DEBUG
            print(#function)
            #endif
            
            super.speechRecognitionDidDetectSpeech(task)
            
        }
        //super.speechRecognitionDidDetectSpeech(task)
    }
    
    public override func speechRecognitionTask(_ task: SFSpeechRecognitionTask, didHypothesizeTranscription transcription: SFTranscription) {
         
        let str = transcription.formattedString.lowercased(with: self.locale)
        
        DispatchQueue.main.async {
        //        super.speechRecognitionTask(task, didHypothesizeTranscription: transcription)
            super.message_update = str
            
            //print(#function, transcription.formattedString.lowercased(with: self.locale))
            
            super.speechRecognitionTask(task, didHypothesizeTranscription: transcription)
            
            //self.modificationCount += 1
            
        }
    }

    public override func speechRecognitionTask(_ task: SFSpeechRecognitionTask, didFinishRecognition recognitionResult: SFSpeechRecognitionResult) {

        let str = recognitionResult.bestTranscription.formattedString.lowercased(with: self.locale)
        DispatchQueue.main.async {
        //super.speechRecognitionTask(task, didFinishRecognition: recognitionResult)
            super.message_update = str
            #if DEBUG
            print(#function, task.state, super.message_update!)
            #endif
            
            super.speechRecognitionTask(task, didFinishRecognition: recognitionResult)
        }
        
    }
    
    public override func speechRecognitionTask(_ task: SFSpeechRecognitionTask, didFinishSuccessfully successfully: Bool) {
        
        DispatchQueue.main.async {
        //super.speechRecognitionTask(task, didFinishSuccessfully : successfully)
            //print(#function, successfully, "REASON: ", self.reason ?? "\"\"", self.reasonCount)
            self.reason = "";
            self.reasonCount += 1
            
            super.speechRecognitionTask(task, didFinishSuccessfully: successfully)
            
        }
    }
    
    public override func speechRecognitionTaskWasCancelled(_ task: SFSpeechRecognitionTask) {
        DispatchQueue.main.async {
            #if DEBUG
            print(#function)
            #endif
            
            super.speechRecognitionTaskWasCancelled(task)
            
        }
    }
    
    
    public override func speechRecognitionTaskFinishedReadingAudio(_ task: SFSpeechRecognitionTask) {
        DispatchQueue.main.async {
            #if DEBUG
            print(#function, task.state)
            #endif
            
            super.speechRecognitionTaskFinishedReadingAudio(task)
        }
    }

    
    var alreadyStarted : Bool = false
    
    @objc public func setAlreadyStarted(val : Bool) {
        self.alreadyStarted = val;
    }
    
    var isInit = false
    
    var amp = Array(repeating: Float(0.0), count: 2)
    @objc public func setAmplitude(amplitude: Float) {
        super.set_amplitude(amplitude)
    }
    
#if os(macOS)
#else
    let audioSession = AVAudioSession.sharedInstance()
#endif
    
    @objc private func doTask(supportsOnDeviceRecognition: Bool) throws {
        //        print("ondevicerecognition", supportsOnDeviceRecognition)
        // Cancel the previous task if it's running.
        
        
        recognitionTask?.cancel()
        
        self.recognitionTask = nil
        
        if !self.isInit {
            self.isInit = true
#if os(macOS)
#else
            try audioSession.setCategory(.playAndRecord, mode: .default)
            try audioSession.setActive(true, options: .notifyOthersOnDeactivation)
#endif
            
            self.initTimeHost = UInt64(ProcessInfo.processInfo.systemUptime);
        }
        
        let inputNode = audioEngine.inputNode
        
        
        // Create and configure the speech recognition request.
        self.recognitionRequest = SFSpeechAudioBufferRecognitionRequest()
        
        guard let recognitionRequest = recognitionRequest else { fatalError("Unable to create a SFSpeechAudioBufferRecognitionRequest object") }
        recognitionRequest.shouldReportPartialResults = true
        recognitionRequest.taskHint = .dictation
        
#if os(macOS)
        if #available(macOS 13, *) {
            recognitionRequest.addsPunctuation = false
        }
#else
        if #available(iOS 16, *) {
            recognitionRequest.addsPunctuation = false
        }
#endif
        
        if #available(iOS 13, *), SpeechRecognition.USE_ONDEVICE_RECOGNITION {
            SpeechRecognition.logger.debug("using on device speech recognition")
            recognitionRequest.requiresOnDeviceRecognition = true
        } else {
            SpeechRecognition.logger.fault("speech recognition unavailable")
            self.isInit = false
            return
        }

//        recognitionTask = speechRecognizer.recognitionTask(with: recognitionRequest, delegate: self);
        
        self.recognitionTask = self.recognitionTask(with: speechRecognizer, with: recognitionRequest, delegate: self)
        
        
        if !self.alreadyStarted {
            let recordingFormat = inputNode.outputFormat(forBus: 0)
            

            inputNode.installTap(onBus: 0, bufferSize: 1024, format: recordingFormat) { (buffer: AVAudioPCMBuffer, when: AVAudioTime) in
                
                self.recognitionRequest?.append(buffer)
  
//                let amplitude = AVAudioPCMBuffer.amplitude(buffer: buffer, at: when, with: .rms, amp: &self.amp)
//                
//                super.set_amplitude(amplitude)
//                guard let channelData = buffer.floatChannelData else {
//                  return
//                }
//
//                let channelDataValue = channelData.pointee
//                // 4
//                let channelDataValueArray = stride(
//                  from: 0,
//                  to: Int(buffer.frameLength),
//                  by: buffer.stride)
//                  .map { channelDataValue[$0] }
//
//                // 5
//                let rms = sqrt(channelDataValueArray.map {
//                  return $0 * $0
//                }
//                .reduce(0, +) / Float(buffer.frameLength))
//
//                // 6
//                let avgPower = 20 * log10(rms)
//                // 7
//
//                var meterLevel : Float
//                do {
//                    // 1
//                    if avgPower.isFinite {
//                        let minDb: Float = -80
//
//                        // 2
//                        if avgPower < minDb {
//                            meterLevel = 0.0
//                        } else if avgPower >= 1.0 {
//                            meterLevel =  1.0
//                        } else {
//                            // 3
//                            meterLevel = (abs(minDb) - abs(avgPower)) / abs(minDb)
//                        }
//                    } else {
//                        meterLevel = 0.0
//                    }
//                }
//                print(meterLevel)
            }
            
            
        }
        
        if (self.alreadyStarted) {
            audioEngine.prepare()
            
        }
        try audioEngine.start()
        
        self.alreadyStarted = true
        self.is_active = true
    }
    
    //let operationQueue = OperationQueue()
    
    @objc public static func requestAuthorization(callback : @escaping ()->Void) {
        SFSpeechRecognizer.requestAuthorization { authStatus in
            OperationQueue.main.addOperation {
                switch authStatus {
                case .authorized:
                    SpeechRecognition.logger.debug("SR authorized")
                    break
                case .denied:
                    SpeechRecognition.logger.debug("SR denied")
                    break
                case .restricted:
                    SpeechRecognition.logger.debug("SR restricted")
                    break
                case .notDetermined:
                    SpeechRecognition.logger.debug("SR not determined")
                    break
                default:
                    SpeechRecognition.logger.fault("SR failed")
                    break
                }
                callback()
            }
        }
    }
    
    @objc private func begin() {
        guard let newSpeechRecognizer = SFSpeechRecognizer(locale: self.locale) else {
            SpeechRecognition.logger.fault("Speech recognizer is not available for this locale!")
            return
        }
        newSpeechRecognizer.queue = super.speech_queue
        newSpeechRecognizer.queue.qualityOfService = .userInteractive
        self.speechRecognizer = newSpeechRecognizer
        
        SpeechRecognition.logger.debug("Speech recognition supported")
        SpeechRecognition.logger.debug("Supported locales: \(SFSpeechRecognizer.supportedLocales(), privacy:.public)")
        
        if speechRecognizer.supportsOnDeviceRecognition {
            SpeechRecognition.logger.debug("Supports on device recognition")
        }
        
        speechRecognizer.delegate = self
        
        SFSpeechRecognizer.requestAuthorization { authStatus in
            OperationQueue.main.addOperation {
                switch authStatus {
                case .authorized:
                    SpeechRecognition.logger.debug("SR authorized")
                    do {
                        try self.doTask(
                            supportsOnDeviceRecognition: self.speechRecognizer.supportsOnDeviceRecognition
                        )
                        
                    } catch {
                        SpeechRecognition.logger.fault("doTask error")
                    }
                    
                    break
                case .denied:
                    SpeechRecognition.logger.fault("SR denied")
                    break
                case .restricted:
                    SpeechRecognition.logger.fault("SR restricted")
                    break
                case .notDetermined:
                    SpeechRecognition.logger.debug("SR not determined")
                    break
                default:
                    SpeechRecognition.logger.fault("SR failed")
                    break
                }
            }
        }
        
//
    }
    
    @objc public override func invokeCallbacks(_ event: SPEECH_SYSTEM_EVENT, with status: Speech_System_Event_Status) {
        super.invokeCallbacks(event, with: status)
    }
    
    @objc public override func resetState() {
        super.resetState()
    }
}

/// Type of analysis
public enum AnalysisMode {
    /// Root Mean Squared
    case rms
    /// Peak
    case peak
}

/// How to deal with stereo signals
public enum StereoMode {
    /// Use left channel
    case left
    /// Use right channel
    case right
    /// Use combined left and right channels
    case center
}

extension AVAudioPCMBuffer {
    
    public static func amplitude(buffer: AVAudioPCMBuffer, at time: AVAudioTime, with analysisMode: AnalysisMode, amp: inout [Float]) -> Float {
        guard let floatData = buffer.floatChannelData else { return 0.0 }

            let channelCount = Int(buffer.format.channelCount)
            let length = UInt(buffer.frameLength)

            // n is the channel
            for n in 0 ..< channelCount {
                let data = floatData[n]

                if analysisMode == .rms {
                    var rms: Float = 0
                    vDSP_rmsqv(data, 1, &rms, UInt(length))
                    amp[n] = rms
                } else {
                    var peak: Float = 0
                    var index: vDSP_Length = 0
                    vDSP_maxvi(data, 1, &peak, &index, UInt(length))
                    amp[n] = peak
                }
            }
        
        return amp.reduce(0, +) / Float(channelCount)

//            switch stereoMode {
//            case .left:
//                handler(leftAmplitude)
//            case .right:
//                handler(rightAmplitude)
//            case .center:
//                handler(amplitude)
//            }
        }

}
