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

#ifndef VF_LISTENERS_VFHEADER
#define VF_LISTENERS_VFHEADER

/*============================================================================*/
/**
  A group of concurrent Listeners.

  A Listener is an object of class type which inherits from a defined
  interface, and registers on a provided instance of Listeners to receive
  asynchronous notifications of changes to concurrent states. Another way of
  defining Listeners, is that it is similar to a Juce ListenerList but with
  the provision that the Listener registers with the CallQueue upon which the
  notification should be made.

  Listeners makes extensive use of CallQueue for providing the notifications,
  and provides a higher level facility for implementing the concurrent
  synchronization strategy outlined in CallQueue. Therefore, the same notes
  which apply to functors in CallQueue also apply to Listener member
  invocations. Their execution time should be brief, limited in scope to
  updating the recipient's view of a shared state, and use reference counting
  for parameters of class type.
  
  To use this system, first declare your Listener interface:

  @code

  struct Listener
  {
    // Sent on every output block
    virtual void onOutputLevelChanged (const float outputLevel) { }
  };

  @endcode

  Now set up the place where you want to send the notifications. In this
  example, we will set up the AudioIODeviceCallback to notify anyone who is
  interested about changes in the current audio output level. We will use
  this to implement a VU meter:

  @code

  Listeners <Listener> listeners;

  // (Process audio data)

  // Calculate output level
  float outputLevel = calcOutputLevel ();

  // Notify listeners
  listeners.call (&Listener::onOutputLevelChanged, outputLevel);

  @endcode

  To receive notifications, derive from Listener and then add yourself to the
  Listeners object using the desired CallQueue.

  @code

  // We want notifications on the message thread
  GuiCallQueue fifo;

  struct VUMeter : public Listener, public Component
  {
    VUMeter () : m_outputLevel (0)
    {
      listeners.add (this, fifo);
    }

    ~VUMeter ()
    {
      listeners.remove (this);
    }

    void onOutputLevelChanged (float outputLevel)
    {
      // Update our copy of the output level shared state.
      m_outputLevel = outputLevel;
        
      // Now trigger a redraw of the control.
      repaint ();
    }

    float m_outputLevel;
  };

  @endcode

  In this example, the VUMeter constructs with the output level set to zero,
  and must wait for a notification before it shows up to date data. For a
  simple VU meter, this is likely not a problem. But if the shared state
  contains complex information, such as dynamically allocated objects with
  rich data, then we need a more solid system.

  We will add some classes to create a complete robust example of the use of
  Listeners to synchronize shared state:

  @code

  // Handles audio device output.
  class AudioDeviceOutput : public AudioIODeviceCallback
  {
  public:
    struct Listener
    {
      // Sent on every output block.
      virtual void onOutputLevelChanged (float outputLevel) { }
    };

    AudioDeviceOutput () : AudioDeviceOutput ("Audio CallQueue")
    {
    }

    ~AudioDeviceOutput ()
    {
      m_fifo.close ();
    }

    void addListener (Listener* listener, CallQueue& callQueue)
    {
      // Acquire read access to the shared state.
      ConcurrentState <State>::ReadAccess state (m_state);

      // Add the listener.
      m_listeners.add (listener, callQueue);

      // Queue an update for the listener to receive the initial state.
      m_listeners.queue1 (listener,
                          &Listener::onOutputLevelChanged,
                          state->outputLevel);
    }

    void removeListener (Listener* listener)
    {
      m_listeners.remove (listener);
    }

  protected:
    void audioDeviceIOCallback (const float** inputChannelData,
					      int numInputChannels,
					      float** outputChannelData,
					      int numOutputChannels,
					      int numSamples)
    {
      // Synchronize our call queue. Not needed for this example but
      // included here as a best-practice for audio device I/O callbacks.
      m_fifo.synchronize ();

      // (Process audio data)

      // Calculate output level.
      float newOutputLevel = calcOutputLevel ();

      // Update shared state.
      {
        ConcurrentState <State>::WriteAccess state (m_state);
          
        m_state->outputLevel = newOutputLevel;
      }

      // Notify listeners.
      listeners.call (&Listener::onOutputLevelChanged, newOutputLevel);
    }

  private:
    struct State
    {
      State () : outputLevel (0) { }

      float outputLevel;
    };

    ConcurrentState <State> m_state;

    ManualCallQueue m_fifo;
  };

  @endcode

  Although the rigor demonstrated in the example above is not strictly
  required when the shared state consists only of a single float, it
  becomes necessary when there are dynamically allocated objects with complex
  interactions in the shared state.

  @see CallQueue

  @class Listeners
  @ingroup vf_concurrent
*/
class ListenersBase
{
public:
  struct ListenersStructureTag { };

  typedef GlobalFifoFreeStore <ListenersStructureTag> AllocatorType;

  typedef GlobalFifoFreeStore <ListenersBase> CallAllocatorType;

  class Call : public juce::ReferenceCountedObject,
               public AllocatedBy <CallAllocatorType>
  {
  public:
    typedef juce::ReferenceCountedObjectPtr <Call> Ptr;
    virtual void operator () (void* const listener) = 0;
  };

private:
  typedef unsigned long timestamp_t;

  class Group;
  typedef List <Group> Groups;

  class Proxy;
  typedef List <Proxy> Proxies;

  class CallWork;
  class GroupWork;
  class GroupWork1;

  // Maintains a list of listeners registered on the same CallQueue
  //
  class Group : public Groups::Node,
                public juce::ReferenceCountedObject,
                public AllocatedBy <AllocatorType>
  {
  public:
    typedef juce::ReferenceCountedObjectPtr <Group> Ptr;

    explicit Group    (CallQueue& callQueue);
    ~Group            ();
    void add          (void* listener, const timestamp_t timestamp,
                       AllocatorType& allocator);
    bool remove       (void* listener);
    bool contains     (void* const listener);
    void call         (Call* const c, const timestamp_t timestamp);
    void queue        (Call* const c, const timestamp_t timestamp);
    void call1        (Call* const c, const timestamp_t timestamp,
                       void* const listener);
    void queue1       (Call* const c, const timestamp_t timestamp,
                       void* const listener);
    void do_call      (Call* const c, const timestamp_t timestamp);
    void do_call1     (Call* const c, const timestamp_t timestamp,
                       void* const listener);

    bool empty        () const { return m_list.empty(); }
    CallQueue& getCallQueue () const { return m_fifo; }

  private:
    struct Entry;

    CallQueue& m_fifo;
    List <Entry> m_list;
    void* m_listener;
    CacheLine::Aligned <ReadWriteMutex> m_mutex;
  };

  // A Proxy is keyed to a unique pointer-to-member of a
  // ListenerClass and is used to consolidate multiple unprocessed
  // Calls into a single call to prevent excess messaging. It is up
  // to the user of the class to decide when this behavior is appropriate.
  //
  class Proxy : public Proxies::Node,
                public AllocatedBy <AllocatorType>
  {
  public:
    enum
    {
      maxMemberBytes = 16
    };

    Proxy (void const* const member, const size_t bytes);
    ~Proxy ();

    void add    (Group* group, AllocatorType& allocator);
    void remove (Group* group);
    void update (Call* const c, const timestamp_t timestamp);

    bool match  (void const* const member, const size_t bytes) const;

  private:
    class Work;
    struct Entry;
    typedef List <Entry> Entries;
    char m_member [maxMemberBytes];
    const size_t m_bytes;
    Entries m_entries;
  };

protected:
  ListenersBase ();
  ~ListenersBase ();

  inline CallAllocatorType& getCallAllocator ()
  {
    return *m_callAllocator;
  }

  void add_void     (void* const listener, CallQueue& callQueue);
  void remove_void  (void* const listener);

  void callp        (Call::Ptr c);
  void queuep       (Call::Ptr c);
  void call1p_void  (void* const listener, Call* c);
  void queue1p_void (void* const listener, Call* c);
  void updatep      (void const* const member,
                     const size_t bytes, Call::Ptr cp);

private:
  Proxy* find_proxy (const void* member, int bytes);

private:
  Groups m_groups;
  Proxies m_proxies;
  timestamp_t m_timestamp;
  CacheLine::Aligned <ReadWriteMutex> m_groups_mutex;
  CacheLine::Aligned <ReadWriteMutex> m_proxies_mutex;
  AllocatorType::Ptr m_allocator;
  CallAllocatorType::Ptr m_callAllocator;
};

/*============================================================================*/

template <class ListenerClass>
class Listeners : public ListenersBase
{
private:
  template <class Functor>
  class CallType : public Call
  {
  public:
    CallType (const Functor& f) : m_f (f)
    {
    }

    void operator() (void* const listener)
    {
	  ListenerClass* object = static_cast <ListenerClass*> (listener);
	  m_f.operator() (object);
    }

  private:
    Functor m_f;
  };

  template <class Functor>
  inline void callf (const Functor& f)
  {
    callp (new (getCallAllocator ()) CallType <Functor> (f));
  }

  template <class Functor>
  inline void queuef (const Functor& f)
  {
    queuep (new (getCallAllocator ()) CallType <Functor> (f));
  }

  inline void call1p (ListenerClass* const listener, Call::Ptr c)
  {
    call1p_void (listener, c);
  }

  inline void queue1p (ListenerClass* const listener, Call::Ptr c)
  {
    queue1p_void (listener, c);
  }

  template <class Functor>
  inline void call1f (ListenerClass* const listener, const Functor& f)
  {
    call1p (listener, new (getCallAllocator ()) CallType <Functor> (f));
  }

  template <class Functor>
  inline void queue1f (ListenerClass* const listener, const Functor& f)
  {
    queue1p (listener, new (getCallAllocator ()) CallType <Functor> (f));
  }

  template <class Member, class Functor>
  inline void updatef (Member member, const Functor& f)
  {
    updatep (reinterpret_cast <void*> (&member), sizeof (Member),
             new (getCallAllocator ()) CallType <Functor> (f));
  }

public:
  /** Add a listener.

      The specified listener is associated with the specified CallQueue and
      added to the list.

      Invariants:

      - All other members of Listeners are blocked during add().

      - The listener is guaranteed to receive every subsequent call.

      - The listener must not already exist in the list.

      - Safe to call from any thread.

      @param listener   The listener to add.
      
      @param callQueue  The CallQueue to associate with the listener.
  */
  void add (ListenerClass* const listener, CallQueue& callQueue)
  {
    add_void (listener, callQueue);
  }

  /** Remove a listener.

      The specified listener, which must have been previously added, is removed
      from the list. A listener always needs to remove itself before the
      associated CallQueue is closed.

      Invariants:

      - All other members of Listeners are blocked during remove().

      - The listener is guaranteed not to receive calls after remove() returns.

      - Safe to call from any thread.

      @param listener The listener to remove.
  */
  void remove (ListenerClass* const listener)
  {
    remove_void (listener);
  }

  /** Call a member function on every added listener, on its associated
      CallQueue.

      A listener's CallQueue will be synchronized if this function is called
      from it's associated thread.

      Invariants:

      - A listener that later removes itself afterwards may not get called.

      - Calls from the same thread always execute in order.

      - A listener can remove itself even if it has a pending call.

      @param mf The member function to call. This may be followed by up to 8
                arguments.
  */
  /** @{ */
  template <class Mf>
  inline void call (Mf mf)
  {
    callf (std::bind (mf, std::placeholders::_1));
  }

  template <class Mf, class T1>
  void call (Mf mf, T1 t1)
  {
    callf (std::bind (mf, std::placeholders::_1, t1));
  }
                      
  template <class Mf, class T1, class T2>
  void call (Mf mf, T1 t1, T2 t2)
  {
    callf (std::bind (mf, std::placeholders::_1, t1, t2));
  }
                      
  template <class Mf, class T1, class T2, class T3>
  void call (Mf mf, T1 t1, T2 t2, T3 t3)
  {
    callf (std::bind (mf, std::placeholders::_1, t1, t2, t3));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4>
  void call (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4)
  {
    callf (std::bind (mf, std::placeholders::_1, t1, t2, t3, t4));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4, class T5>
  void call (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
  {
    callf (std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6>
  void call (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
  {
    callf (std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
  void call (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
  {
    callf (std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6, t7));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
  void call (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
  {
    callf (std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6, t7, t8));
  }
  /** @} */

  /** Queue a member function on every added listener, without synchronizing.

      Operates like call(), but no CallQueue synchronization takes place. This
      can be necessary when the call to queue() is made inside a held lock.

      @param mf The member function to call. This may be followed by up to 8
                arguments.
  */
  /** @{ */
  template <class Mf>
  inline void queue (Mf mf)
  {
    queuef (std::bind (mf, std::placeholders::_1));
  }

  template <class Mf, class T1>
  void queue (Mf mf, T1 t1)
  {
    queuef (std::bind (mf, std::placeholders::_1, t1));
  }
                      
  template <class Mf, class T1, class T2>
  void queue (Mf mf, T1 t1, T2 t2)
  {
    queuef (std::bind (mf, std::placeholders::_1, t1, t2));
  }
                      
  template <class Mf, class T1, class T2, class T3>
  void queue (Mf mf, T1 t1, T2 t2, T3 t3)
  {
    queuef (std::bind (mf, std::placeholders::_1, t1, t2, t3));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4>
  void queue (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4)
  {
    queuef (std::bind (mf, std::placeholders::_1, t1, t2, t3, t4));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4, class T5>
  void queue (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
  {
    queuef (std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6>
  void queue (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
  {
    queuef (std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
  void queue (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
  {
    queuef (std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6, t7));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
  void queue (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
  {
    queuef (std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6, t7, t8));
  }
  /** @} */

  /** Call a member function on every added listener, replacing pending
      calls to the same member.

      This operates like call(), except that if there are pending unprocessed
      calls to the same member function,they will be replaced, with the previous
      parameters destroyed normally. This functionality is useful for
      high frequency notifications of non critical data, where the recipient
      may not catch up often enough. For example, the output level of the
      AudioIODeviceCallback in the example is a candidate for the use of
      update().

      @param mf The member function to call. This may be followed by up to 8
                arguments.
  */
  /** @{ */
  template <class Mf>
  inline void update (Mf mf)
  { updatef (mf, std::bind (mf, std::placeholders::_1)); }

  template <class Mf, class T1>
  void update (Mf mf, T1 t1)
  {
    updatef (mf, std::bind (mf, std::placeholders::_1, t1));
  }
                      
  template <class Mf, class T1, class T2>
  void update (Mf mf, T1 t1, T2 t2)
  {
    updatef (mf, std::bind (mf, std::placeholders::_1, t1, t2));
  }
                      
  template <class Mf, class T1, class T2, class T3>
  void update (Mf mf, T1 t1, T2 t2, T3 t3)
  {
    updatef (mf, std::bind (mf, std::placeholders::_1, t1, t2, t3));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4>
  void update (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4)
  {
    updatef (mf, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4, class T5>
  void update (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
  {
    updatef (mf, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6>
  void update (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
  {
    updatef (mf, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
  void update (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
  {
    updatef (mf, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6, t7));
  }
                      
  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
  void update (Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
  {
    updatef (mf, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6, t7, t8));
  }
  /** @} */

  /** Call a member function on a specific listener.

      Like call(), except that one listener is targeted only. This is useful when
      builing complex behaviors during the addition of a listener, such as
      providing an initial state.

      @param listener The listener to call.

      @param mf       The member function to call. This may be followed by up
                      to 8 arguments.
  */
  /** @{ */
  template <class Mf>
  inline void call1 (ListenerClass* const listener, Mf mf)
  {
    call1f (listener, std::bind (mf, std::placeholders::_1));
  }

  template <class Mf, class T1>
  void call1 (ListenerClass* const listener, Mf mf, T1 t1)
  {
    call1f (listener, std::bind (mf, std::placeholders::_1, t1));
  }

  template <class Mf, class T1, class T2>
  void call1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2)
  {
    call1f (listener, std::bind (mf, std::placeholders::_1, t1, t2));
  }

  template <class Mf, class T1, class T2, class T3>
  void call1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2, T3 t3)
  {
    call1f (listener, std::bind (mf, std::placeholders::_1, t1, t2, t3));
  }

  template <class Mf, class T1, class T2, class T3, class T4>
  void call1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2, T3 t3, T4 t4)
  {
    call1f (listener, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4));
  }

  template <class Mf, class T1, class T2, class T3, class T4, class T5>
  void call1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
  {
    call1f (listener, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5));
  }

  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6>
  void call1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
  {
    call1f (listener, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6));
  }

  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
  void call1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
  {
    call1f (listener, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6, t7));
  }

  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
  void call1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
  {
    call1f (listener, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6, t7, t8));
  }
  /** @} */

  /** Queue a member function on a specific listener.

      Like call1(), except that no CallQueue synchronization takes place.

      @param listener The listener to call.

      @param mf       The member function to call. This may be followed by up
                      to 8 arguments.
  */
  /** @{ */
  template <class Mf>
  inline void queue1 (ListenerClass* const listener, Mf mf)
  {
    queue1f (listener, std::bind (mf, std::placeholders::_1));
  }

  template <class Mf, class T1>
  void queue1 (ListenerClass* const listener, Mf mf, T1 t1)
  {
    queue1f (listener, std::bind (mf, std::placeholders::_1, t1));
  }

  template <class Mf, class T1, class T2>
  void queue1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2)
  {
    queue1f (listener, std::bind (mf, std::placeholders::_1, t1, t2));
  }

  template <class Mf, class T1, class T2, class T3>
  void queue1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2, T3 t3)
  {
    queue1f (listener, std::bind (mf, std::placeholders::_1, t1, t2, t3));
  }

  template <class Mf, class T1, class T2, class T3, class T4>
  void queue1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2, T3 t3, T4 t4)
  {
    queue1f (listener, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4));
  }

  template <class Mf, class T1, class T2, class T3, class T4, class T5>
  void queue1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5)
  {
    queue1f (listener, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5));
  }

  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6>
  void queue1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6)
  {
    queue1f (listener, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6));
  }

  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6, class T7>
  void queue1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7)
  {
    queue1f (listener, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6, t7));
  }

  template <class Mf, class T1, class T2, class T3, class T4, class T5, class T6, class T7, class T8>
  void queue1 (ListenerClass* const listener, Mf mf, T1 t1, T2 t2, T3 t3, T4 t4, T5 t5, T6 t6, T7 t7, T8 t8)
  {
    queue1f (listener, std::bind (mf, std::placeholders::_1, t1, t2, t3, t4, t5, t6, t7, t8));
  }
  /** @} */
};
/** @} */

#endif
