//
//  clipper_types.h
//  Make The Thing
//
//  Created by Toby Rosenberg on 4/15/23.
//  Copyright © 2023 Toby Rosenberg. All rights reserved.
//

#ifndef clipper_types_h
#define clipper_types_h

namespace Clipper2Lib {

//Note: all clipping operations except for Difference are commutative.
enum class ClipType { None, Intersection, Union, Difference, Xor };

enum class PathType { Subject, Clip };
enum class JoinWith { None, Left, Right };

enum class VertexFlags : uint32_t {
    None = 0, OpenStart = 1, OpenEnd = 2, LocalMax = 4, LocalMin = 8
};

//By far the most widely used filling rules for polygons are EvenOdd
//and NonZero, sometimes called Alternate and Winding respectively.
//https://en.wikipedia.org/wiki/Nonzero-rule
enum class FillRule { EvenOdd, NonZero, Positive, Negative };

}


#endif /* clipper_types_h */
