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

#ifndef CDECK_HEADER
#define CDECK_HEADER

#include "Deck.h"

class CDeck
  : public Component
  , public FileDragAndDropTarget
  , public vf::ResizableLayout
  , private Deck::Listener
{
public:
  CDeck (int deckNumber, Deck::Ptr deck);
  ~CDeck ();

  void paint (Graphics& g);
  void paintOverChildren (Graphics& g);

protected:
  bool isInterestedInFileDrag (const StringArray& files);
  void fileDragEnter (const StringArray& files, int x, int y);
  void fileDragExit (const StringArray& files);
  void filesDropped (const StringArray& files, int x, int y);

  void onDeckSelect (Deck* deck, Playable::Ptr playable);

private:
  Deck::Ptr m_deck;
  Component* m_speedControl;
  Label* m_text;
  Label* m_title;
  Label* m_artist;
  Label* m_album;

  bool m_hasDropFocus;

  JUCE_DECLARE_NON_COPYABLE_WITH_LEAK_DETECTOR (CDeck)
};

#endif
