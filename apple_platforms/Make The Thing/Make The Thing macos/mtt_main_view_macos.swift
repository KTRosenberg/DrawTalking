//
//  mtt_main_view_macos.swift
//  Make The Thing macos
//
//  Created by Toby Rosenberg on 11/8/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//
import Foundation
import AppKit

@objc public class MTTMainView_macos : NSView {
    public var vc : MTTViewController!
    
    override init(frame: CGRect) {
        super.init(frame: frame)
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }
    
    public override var isFlipped: Bool {
        true
        
    }
    
    
    @objc public override func magnify(with event: NSEvent) {
        vc.magnify(with: event)
    }
    @objc public override func rotate(with event: NSEvent) {
        vc.rotate(with: event)
    }
    @objc public override func swipe(with event: NSEvent) {
        vc.swipe(with: event)
    }
    
    
    
    
    @objc public override func keyDown(with event: NSEvent) {
        vc.keyDown(with: event)
    }
    
    @objc public override func keyUp(with event: NSEvent) {
        vc.keyUp(with: event)
    }
    
    public override var acceptsFirstResponder: Bool { return true }

    

    

}
