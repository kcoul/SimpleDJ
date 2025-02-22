/*============================================================================*/
/*
  VFLib: https://github.com/vinniefalco/VFLib

  Copyright (C) 2008 by Vinnie Falco <vinnie.falco@gmail.com>

  This library contains portions of other open source products covered by
  separate licenses. Please see the corresponding source files for specific
  terms.
  
  VFLib is provided under the terms of The MIT License (MIT):

  Permission is hereby granted, free of charge, to any person obtaining a copy
  of this software and associated documentation files (the "Software"), to deal
  in the Software without restriction, including without limitation the rights
  to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
  copies of the Software, and to permit persons to whom the Software is
  furnished to do so, subject to the following conditions:

  The above copyright notice and this permission notice shall be included in
  all copies or substantial portions of the Software.

  THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
  IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
  FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
  AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
  LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
  FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
  IN THE SOFTWARE.
*/
/*============================================================================*/

ChannelImageType::ChannelPixelData::ChannelPixelData (Image sourceImage, int channelNumber)
  : ImagePixelData ((channelNumber < 0) ? Image::RGB : Image::SingleChannel,
                    sourceImage.getWidth (),
                    sourceImage.getHeight ())
  , m_bits (sourceImage,
            0,
            0,
            sourceImage.getWidth(),
            sourceImage.getHeight ())
  , m_channelNumber (channelNumber)
{
  jassert (sourceImage.getFormat () == Image::RGB || sourceImage.getFormat () == Image::ARGB);
}

ChannelImageType::ChannelPixelData::~ChannelPixelData ()
{
}

std::unique_ptr<LowLevelGraphicsContext> ChannelImageType::ChannelPixelData::createLowLevelContext ()
{
  return std::make_unique<LowLevelGraphicsSoftwareRenderer> (Image (this));
}

ImagePixelData::Ptr ChannelImageType::ChannelPixelData::clone ()
{
  return this;
}

std::unique_ptr<ImageType> ChannelImageType::ChannelPixelData::createType() const
{
  jassertfalse;
  return nullptr;
}

void ChannelImageType::ChannelPixelData::initialiseBitmapData (
  Image::BitmapData& bd, int x, int y, Image::BitmapData::ReadWriteMode)
{
  if (m_channelNumber >= 0)
  {
    bd.data = m_bits.getPixelPointer (x, y) + m_channelNumber;
    bd.pixelFormat = Image::SingleChannel;
    bd.lineStride  = m_bits.lineStride;
    bd.pixelStride = m_bits.pixelStride;
  }
  else
  {
    bd.data = m_bits.getPixelPointer (x, y) + 0;
    bd.pixelFormat = Image::RGB;
    bd.lineStride  = m_bits.lineStride;
    bd.pixelStride = m_bits.pixelStride;
  }
}

//------------------------------------------------------------------------------

Image ChannelImageType::fromImage (Image sourceImage, int channelNumber)
{
  return Image (new ChannelPixelData (sourceImage, channelNumber));
}
