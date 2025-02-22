/*
  ==============================================================================

  Copyright (c) 2012 by Vinnie Falco

  This file is provided under the terms of the MIT License:

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in all
  copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
  OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
  SOFTWARE.

  ==============================================================================
*/

#ifndef PLAYABLE_HEADER
#define PLAYABLE_HEADER

/** Consumable audio data.
*/
class Playable
  : public vf::ConcurrentObject
  , public AudioSource
{
public:
  struct Metadata
  {
    String album;
    String artist;
    String title;
  };

  //TODO: Good discussion of why these might be hard to modernize to std::shared_ptr
  //TODO: https://forum.juce.com/t/referencecountedobject-vs-std-shared-ptr/31185
  typedef ReferenceCountedObjectPtr <Playable> Ptr;

  virtual double getSampleRate () = 0;

  virtual Metadata getMetadata () = 0;
};

#endif
