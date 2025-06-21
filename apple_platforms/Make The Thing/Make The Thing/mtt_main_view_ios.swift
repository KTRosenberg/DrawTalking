//
//  mtt_main_view_ios.swift
//  Make The Thing
//
//  Created by Toby Rosenberg on 11/8/21.
//  Copyright © 2021 Toby Rosenberg. All rights reserved.
//

import Foundation
import UIKit

@objc public class MTTMainView : UIView {
    public var vc : MTTViewController!
    
    override init(frame: CGRect) {
        super.init(frame: frame)
        print("initializing MTTTouchView")
    }
    
    required init?(coder: NSCoder) {
        super.init(coder: coder)
    }
    
    override public func touchesBegan(_ touches: Set<UITouch>, with event: UIEvent?) {
        autoreleasepool {
            super.touchesBegan(touches, with: event)
        }
    }
    override public func touchesMoved(_ touches: Set<UITouch>, with event: UIEvent?) {
        autoreleasepool {
            super.touchesMoved(touches, with: event)
        }
    }
    override public func touchesEnded(_ touches: Set<UITouch>, with event: UIEvent?) {
        autoreleasepool {
            super.touchesEnded(touches, with: event)
        }
    }
    override public func touchesCancelled(_ touches: Set<UITouch>, with event: UIEvent?) {
        autoreleasepool {
            super.touchesCancelled(touches, with: event)
        }
    }
}
