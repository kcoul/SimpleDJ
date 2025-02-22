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

#ifndef VF_SEMAPHORE_VFHEADER
#define VF_SEMAPHORE_VFHEADER

/*============================================================================*/
/**
  A semaphore.

  This provides a traditional semaphore synchronization primitive. There is no
  upper limit on the number of signals.

  @note There is no tryWait() or timeout facility for acquiring a resource.

  @ingroup vf_core
*/
class Semaphore
{
public:
  /** Create a semaphore with the specified number of resources.

      @param initialCount The starting number of resources.
  */
  explicit Semaphore (int initialCount);

  ~Semaphore ();

  /** Increase the number of available resources.

      @param amount The number of new resources available.
  */
  void signal (int amount = 1);

  /** Wait for a resource.
  */
  void wait ();

private:
  class WaitingThread
    : public LockFreeStack <WaitingThread>::Node
    , LeakChecked <WaitingThread>
  {
  public:
    WaitingThread ();

    void wait ();
    void signal ();

  private:
  juce::WaitableEvent m_event;
  };

  typedef juce::SpinLock LockType;

  LockType m_mutex;
  juce::Atomic <int> m_counter;
  LockFreeStack <WaitingThread> m_waitingThreads;
  LockFreeStack <WaitingThread> m_deleteList;
};

#endif
