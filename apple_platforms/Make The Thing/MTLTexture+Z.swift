//
//	MTLTexture+Z.swift
//	ZKit
//
//	The MIT License (MIT)
//
//	Copyright (c) 2016 Electricwoods LLC, Kaz Yoshikawa.
//
//	Permission is hereby granted, free of charge, to any person obtaining a copy 
//	of this software and associated documentation files (the "Software"), to deal 
//	in the Software without restriction, including without limitation the rights 
//	to use, copy, modify, merge, publish, distribute, sublicense, and/or sell 
//	copies of the Software, and to permit persons to whom the Software is 
//	furnished to do so, subject to the following conditions:
//
//	The above copyright notice and this permission notice shall be included in 
//	all copies or substantial portions of the Software.
//
//	THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR 
//	IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, 
//	FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE 
//	AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, 
//	WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
//	OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
//	THE SOFTWARE.
//


import Foundation
import CoreGraphics
import MetalKit
import Accelerate

@objc class MTT_MTLTexture_Util : NSObject {
    
    #if os(iOS)
    typealias XImage = UIImage
    #elseif os(macOS)
    typealias XImage = NSImage
    #endif
    
    @objc static func CGImageFromMTLTexture(texture : MTLTexture) -> CGImage? {
        assert(texture.pixelFormat == .bgra8Unorm)
    
        // read texture as byte array
        let rowBytes = texture.width * 4
        let length = rowBytes * texture.height
        var bgraBytes = [UInt8](repeating: 0, count: length)
        let region = MTLRegionMake2D(0, 0, texture.width, texture.height)
//            texture.getBytes(UnsafeMutableRawPointer(mutating: bgraBytes), bytesPerRow: rowBytes, from: region, mipmapLevel: 0)
        bgraBytes.withUnsafeMutableBytes{ texArrayPtr in
            texture.getBytes(texArrayPtr.baseAddress!, bytesPerRow: rowBytes, from: region, mipmapLevel: 0)
        }

        // use Accelerate framework to convert from BGRA to RGBA
        /*
        var bgraBuffer = vImage_Buffer(data: UnsafeMutableRawPointer(mutating: bgraBytes),
                    height: vImagePixelCount(texture.height), width: vImagePixelCount(texture.width), rowBytes: rowBytes)
        */
        var bgraBuffer : vImage_Buffer!
        bgraBytes.withUnsafeMutableBytes{ ptr in
            bgraBuffer = vImage_Buffer(data: ptr.baseAddress!, height: vImagePixelCount(texture.height), width: vImagePixelCount(texture.width), rowBytes: rowBytes)
        }
        
        var rgbaBytes = [UInt8](repeating: 0, count: length)
        /*
        var rgbaBuffer = vImage_Buffer(data: UnsafeMutableRawPointer(mutating: rgbaBytes),
                    height: vImagePixelCount(texture.height), width: vImagePixelCount(texture.width), rowBytes: rowBytes)
         */
        var rgbaBuffer : vImage_Buffer!
        rgbaBytes.withUnsafeMutableBytes{ ptr in
            rgbaBuffer = vImage_Buffer(data: ptr.baseAddress!, height: vImagePixelCount(texture.height), width: vImagePixelCount(texture.width), rowBytes: rowBytes)
        }
        

        
        
        let map: [UInt8] = [2, 1, 0, 3]
        vImagePermuteChannels_ARGB8888(&bgraBuffer, &rgbaBuffer, map, 0)

        // flipping image virtically
        var flippedBytes = bgraBytes // share the buffer
        /*
            var flippedBuffer = vImage_Buffer(data: UnsafeMutableRawPointer(mutating: flippedBytes),
                    height: vImagePixelCount(texture.height), width: vImagePixelCount(texture.width), rowBytes: rowBytes)
        */
        var flippedBuffer : vImage_Buffer!
        flippedBytes.withUnsafeMutableBytes { ptr in
            flippedBuffer = vImage_Buffer(data: ptr.baseAddress!, height: vImagePixelCount(texture.height), width: vImagePixelCount(texture.width), rowBytes: rowBytes)
        }
        vImageVerticalReflect_ARGB8888(&rgbaBuffer, &flippedBuffer, 0)

        // create CGImage with RGBA
        let colorScape = CGColorSpaceCreateDeviceRGB()
        let bitmapInfo = CGBitmapInfo(rawValue: CGImageAlphaInfo.premultipliedLast.rawValue)
        guard let data = CFDataCreate(nil, flippedBytes, length) else { return nil }
        guard let dataProvider = CGDataProvider(data: data) else { return nil }
        let cgImage = CGImage(width: texture.width, height: texture.height, bitsPerComponent: 8, bitsPerPixel: 32, bytesPerRow: rowBytes,
                    space: colorScape, bitmapInfo: bitmapInfo, provider: dataProvider,
                    decode: nil, shouldInterpolate: true, intent: .defaultIntent)
        return cgImage
    }
    @objc static func ImageFromMTLTexture(texture : MTLTexture) -> XImage? {
        guard let cgImage = CGImageFromMTLTexture(texture: texture) else { return nil }
        #if os(iOS)
        return UIImage(cgImage: cgImage)
        #elseif os(macOS)
        return NSImage(cgImage: cgImage, size: CGSize(width: cgImage.width, height: cgImage.height))
        #endif
    }
}
