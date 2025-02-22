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

#include "StandardIncludes.h"
#include "CDeckMixer.h"
#include "CDeckLevelMeter.h"
#include "CDeckFader.h"

CDeckMixer::CDeckMixer (Deck::Ptr deck, bool rightFacing)
  : vf::ResizableLayout (this)
  , m_deck (deck)
{
  setOpaque (true);
  setSize (64, 200);

  {
    Component* c = new CDeckLevelMeter (deck);
    c->setBounds (0, 70, 30, 130);
    addToLayout (c, anchorTopLeft, anchorBottomLeft);
    addAndMakeVisible (c);
  }

  {
    Component* c = new CDeckFader (deck->param["vol"]);
    c->setBounds (0, 70, 30, 130);
    addToLayout (c, anchorTopLeft, anchorBottomLeft);
    addAndMakeVisible (c);
  }

  setMinimumSize (16, 64);

  activateLayout ();
}

CDeckMixer::~CDeckMixer ()
{
  deleteAllChildren ();
}

void CDeckMixer::paint (Graphics& g)
{
  juce::Rectangle <int> r (getLocalBounds ());

  g.setColour (Colours::white);
  g.drawRect (r, 1);

  g.setColour (Colours::grey.darker ());
  g.fillRect (r);
}
