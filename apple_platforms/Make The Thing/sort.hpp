//
//  sort.hpp
//  Make The Thing
//
//  Created by Toby Rosenberg on 7/6/20.
//  Copyright © 2020 Toby Rosenberg. All rights reserved.
//

#ifndef sort_hpp
#define sort_hpp



template <typename T, typename Compare_Routine>
void insertionSort(T* arr, usize length, Compare_Routine compare) {
      int i, j;
      T tmp;
      for (i = 1; i < length; i++) {
            j = i;
            while (j > 0 && compare(&arr[j], &arr[j - 1])) {
                  tmp = arr[j];
                  arr[j] = arr[j - 1];
                  arr[j - 1] = tmp;
                  j--;
            }
      }
}

// Credit to Casey, Handmade Hero
static inline void radix_sort(usize EntryCount, u64* First, u64* Temp)
{
    u64* Source = First;
    u64* Dest = Temp;
    for(u32 ByteIndex = 0;
        ByteIndex < 64;
        ByteIndex += 8)
    {
        u32 SortKeyOffsets[256] = {};
        
        // NOTE(casey): First pass - count how many of each key
        for(u32 Index = 0;
            Index < EntryCount;
            ++Index)
        {
            u64 RadixValue = Source[Index];
            u64 RadixPiece = (RadixValue >> ByteIndex) & 0xFF;
            ++SortKeyOffsets[RadixPiece];
        }
        
        // NOTE(casey): Change counts to offsets
        u32 Total = 0;
        for(u32 SortKeyIndex = 0;
            SortKeyIndex < 256;
            ++SortKeyIndex)
        {
            u64 Count = SortKeyOffsets[SortKeyIndex];
            SortKeyOffsets[SortKeyIndex] = Total;
            Total += Count;
        }
        
        // NOTE(casey): Second pass - place elements into the right location
        for(u32 Index = 0;
            Index < EntryCount;
            ++Index)
        {
            u64 RadixValue = Source[Index];
            u64 RadixPiece = (RadixValue >> ByteIndex) & 0xFF;
            Dest[SortKeyOffsets[RadixPiece]++] = Source[Index];
        }
        
        u64* SwapTemp = Dest;
        Dest = Source;
        Source = SwapTemp;
    }
}


#endif /* sort_hpp */
