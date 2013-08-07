Gesture Support
===============

The namespace libavg.gesture exposes a group of configurable gesture recognizers. 

.. automodule:: libavg.gesture
    :no-members:

    .. inheritance-diagram:: DragRecognizer SwipeRecognizer TapRecognizer TransformRecognizer DoubletapRecognizer HoldRecognizer
        :parts: 1
    
    .. inheritance-diagram:: Transform
        :parts: 1


    .. autoclass:: DoubletapRecognizer(node, [maxTime=MAX_DOUBLETAP_TIME, maxDist=MAX_TAP_DIST, initialEvent=None, possibleHandler=None, failHandler=None, detectedHandler=None])

        A :py:class:`DoubletapRecognizer` detects doubletaps: Two short touches in quick
        succession without a large change of the cursor position.

        :param maxTime: The maximum time that each phase of the tap may take.

        :param maxDist: The maximum distance the contact may move in millimeters.


    .. autoclass:: DragRecognizer(eventNode, [coordSysNode=None, initialEvent=None, direction=ANY_DIRECTION, directionTolerance=DIRECTION_TOLERANCE, friction=-1, minDragDist=None, possibleHandler=None, failHandler=None, detectedHandler=None, moveHandler=None, upHandler=None, endHandler=None])

        A :py:class:`DragRecognizer` attaches itself to a node's cursor events and 
        delivers higher-level callbacks that can be used to implement dragging or 
        drag-like functionality.

        :py:class:`DragRecognizer` supports inertia after the node is released.
        
        :param avg.Node coordSysNode:    

            Used to determine the coordinate system for the offsets returned by the 
            callbacks. If :py:attr:`coordSysNode` is not given, :py:attr:`eventNode` is 
            used as default. The :py:class:`DragRecognizer` never modifies any nodes 
            itself. :py:attr:`coordSysNode` can be used to separate the node that
            is the 'handle' for the events from the node that is being moved - for 
            instance, to allow moving a window by dragging the title bar.

        :param direction:

            Can be used to constrain the recognizer to :py:const:`VERTICAL` or 
            :py:const:`HORIZONTAL` drags only. If one of these constants is passed as 
            :py:attr:`direction`, the recognizer invokes :py:meth:`onPossible`
            when the down event arrives, then determines whether the drag is a 
            predominantly horizontal or vertical drag and invokes either 
            :py:meth:`onDetected` or :py:meth:`onFail` depending on the result.

        :param float directionTolerance:

            A tolerance angle in radians for the detection of horizontal and vertical
            drags.

        :param avg.Node eventNode: 
        
            The node to attach to. The :py:class:`DragRecognizer` registers an event 
            handler to react to any contacts for this node. 
            
        :param float friction:

            If set, this parameter enables inertia processing. It describes how 
            quickly the drag comes to a stop after the cursor is released.

        :param float minDragDist:

            Minimum distance in mm that the cursor must move for the recognizer to switch
            from :py:const:`POSSIBLE` to :py:const:`DETECTED`. Default is either 0 (for 
            :py:const:`ANY_DIRECTION` recognizers) or :py:const:`MIN_DRAG_DIST` (for
            constrained recognizers).

        :param moveHandler:

            A shortcut for 
            :samp:`Recognizer.subscribe(Recognizer.MOTION, moveHandler)`.

        :param upHandler:

            A shortcut for 
            :samp:`Recognizer.subscribe(Recognizer.UP, upHandler)`.

        **Messages:**

            To get these messages, call :py:meth:`Publisher.subscribe`.

            .. py:method:: Recognizer.MOTION(offset)

                Emitted when the drag should cause a position change. This usually happens
                in response to a :py:const:`CURSORMOTION` event, but may also happen
                because of inertia.

                :param avg.Point2D offset: 
                
                    The current offset from the start of the drag in coordinates relative
                    to the :py:attr:`coordSysNode`'s parent.

            .. py:method:: Recognizer.UP(offset)

                Emitted when the cursor is released. If inertia is enabled, there may be 
                move events after the up event.

                :param avg.Point2D offset: 
                
                    The current offset from the start of the drag in coordinates relative
                    to the :py:class:`coordSysNode`'s parent.

        .. py:method:: abort()

            Aborts the present recognized gesture and sliding caused by inertia

        .. py:method:: abortInertia()

            Causes inertia processing to end immediately.


    .. autoclass:: HoldRecognizer(node, [delay=HOLD_DELAY, maxDist=MAX_TAP_DIST, initialEvent=None, possibleHandler=None, failHandler=None, detectedHandler=None, stopHandler=None])

        A :py:class:`HoldRecognizer` detects if a touch is held for a certain amount of 
        time. Holds are continuous events: the :py:meth:`stopHandler` is called when the
        contact up event arrives.

        :param delay: The amount of time that has to pass before the hold is recognized.

        :param maxDist: The maximum distance the contact may move in millimeters.


    .. autoclass:: Recognizer(node, isContinuous, maxContacts, initialEvent[, possibleHandler=None, failHandler=None, detectedHandler=None, endHandler=None])

        Base class for gesture recognizers that attach to a node's cursor events and 
        emit higher-level events. Gesture recognizers have a standard set of states and
        callbacks, but derived classes may add their own callbacks and do not need to
        invoke all base class callbacks. The possible states vary depending on the value 
        of :py:attr:`isContinuous`:

        .. image:: Recognizer.png

        A usage example for the recognizers can be found under
        :samp:`src/samples/gestures.py`. Many of the recognizers have default timeouts 
        and distance limits which can be changed by modifying :file:`avgrc`. The sample
        file under :file:`src/avgrc` contains explanations.

        :param Node node: Node to attach to.

        :param bool isContinuous: 
            
            :py:const:`True` if the gesture stays active after it has been detected.
        

        :param maxContacts:

            The maximum number of contacts that the recognizer should handle. 
            :py:const:`None` if there is no maximum.

        :param initialEvent:

            A cursordown event to pass to the recognizer immediately.
            
        :param possibleHandler:

            A shortcut for 
            :samp:`Recognizer.subscribe(Recognizer.POSSIBLE, possibleHandler)`.

        :param failHandler:

            A shortcut for :samp:`Recognizer.subscribe(Recognizer.FAIL, failHandler)`.

        :param detectedHandler:

            A shortcut for 
            :samp:`Recognizer.subscribe(Recognizer.DETECTED, detectedHandler)`.

        :param endHandler:

            A shortcut for :samp:`Recognizer.subscribe(Recognizer.END, endHandler)`.

        **Messages:**

            Gesture recognizers emit messages whenever they change state - see the state
            diagrams above. The messages have a parameter of type :py:class:`CursorEvent`.

            To get these messages, call :py:meth:`Publisher.subscribe`.

            .. py:method:: POSSIBLE()

                Emit when gesture recognition begins - usually after a cursordown event.
                Some continuous gestures (such as unconstrained drags) never emit 
                :py:meth:`POSSIBLE` but emit :py:meth:`DETECTED` immediately.

            .. py:method:: FAILED() 

                Emitted when gesture recognition is rejected. For instance, in the case 
                of a :py:class:`DoubleTapRecognizer`, a :py:meth:`FAILED` message is
                emitted if the touch stays on the surface for too long.

            .. py:method:: DETECTED()

                Emitted when the gesture is recognized. For discrete gestures, this 
                signifies the end of gesture processing. 

            .. py:method:: END()

                Emitted when a continuous gesture ends. This is often a result of an
                up event, but e.g. in the case of inertia, :py:meth:`END` is emitted
                when movement stops.
                

        .. py:method:: abort()

            Aborts the present recognized gesture

        .. py:method:: enable(isEnabled)

            Enables or disables the :py:class:`Recognizer`.

        .. py:method:: getState() -> String

            Returns the state ("IDLE", "POSSIBLE" or "RUNNING") of the recognizer.


    .. autoclass:: SwipeRecognizer(node, direction, [numContacts=1, directionTolerance=SWIPE_DIRECTION_TOLERANCE, minDist=MIN_SWIPE_DIST, maxContactDist=MAX_SWIPE_CONTACT_DIST, initialEvent=None, possibleHandler=None, failHandler=None, detectedHandler=None])

        A :py:class:`SwipeRecognizer` detects movement of one or more contacts in a
        specified direction and with a minimal distance. Whether the gesture is recognized
        is determined when an up event occurs.

        :param direction: 
        
            One of :py:const:`SwipeRecognizer.UP`, :py:const:`DOWN`, :py:const:`LEFT` or 
            :py:const:`RIGHT`.

        :param numContacts: The minimum number of contacts for the swipe.

        :param directionTolerance: 
        
            Maximum deviation from the ideal direction that the touch(es) may have in
            radians.

        :param minDist: 
        
            Minimum distance between start position and end position of each contact in 
            millimeters.

        :param maxInterContactDist:

            Maximum distance between the start positions of the different contacts.


    .. autoclass:: TapRecognizer(node, [maxTime=MAX_TAP_TIME, maxDist=MAX_TAP_DIST, initialEvent=None, possibleHandler=None, failHandler=None, detectedHandler=None])

        A :py:class:`TapRecognizer` detects short touches without a large change of the 
        cursor position.

        :param maxTime: The maximum time that the tap may take in milliseconds.

        :param maxDist: The maximum distance the contact may move in millimeters.


    .. autoclass:: Transform(trans, [rot=0, scale=1, pivot=(0,0)])

        Encapsulates a coordinate transformation and can be used to change the position,
        rotation and scale of a node.

        .. py:attribute:: pivot

            The point around which rot and scale are applied.

        .. py:attribute:: rot

            Rotation in radians.

        .. py:attribute:: scale

            Multiplies the size of the node.

        .. py:attribute:: trans

            The translation.

        .. py:method:: moveNode(node)

            Changes a :py:attr:`node`'s pos, angle and size by applying the transform.


    .. autoclass:: TransformRecognizer(eventNode, [coordSysNode=None, initialEvent=None, friction=-1, detectedHandler=None, moveHandler=None, upHandler=None, endHandler=None])

        A :py:class:`TransformRecognizer` is used to support drag/zoom/rotate 
        functionality. From any number of touches on a node, it calculates an aggregate
        transform that can be used to change the position, size and angle of a node.
        The class supports intertia after the node is released.

        :param avg.Node eventNode: 
        
            The node to attach to. The :py:class:`TransformRecognizer` registers an event
            handler to react to any contacts for this node. 
            
        :param avg.Node coordSysNode: 

            Used to determine the coordinate system for the transforms returned by the 
            callbacks.  If :py:attr:`coordSysNode` is not given, :py:attr:`eventNode` is 
            used as default. The :py:class:`TransformRecognizer` never modifies any nodes 
            itself. :py:attr:`coordSysNode` can be used to separate the node that
            is the 'handle' for the events from the node that is being moved - for 
            instance, to allow moving and rotating a window by dragging the title bar.

        :param float friction:

            If set, this parameter enables inertia processing. It describes how 
            quickly the transform comes to a stop after the cursor is released.

        :param moveHandler:

            A shortcut for 
            :samp:`Recognizer.subscribe(Recognizer.MOTION, moveHandler)`.

        :param upHandler:

            A shortcut for 
            :samp:`Recognizer.subscribe(Recognizer.UP, upHandler)`.

        **Messages:**

            To get these messages, call :py:meth:`Publisher.subscribe`.

            .. py:method:: Recognizer.MOTION(transform)

                Emitted whenever the transform changes. This usually happens
                in response to one or more :py:const:`CURSORMOTION` events, but may also
                happen because of inertia.

                :param Transform transform:
                
                    The change in transformation since the last call of move or up.

            .. py:method:: Recognizer.UP(transform)

                Called when the last touch is released. If inertia is enabled, there may
                be move events after the up event.

                :param Transform transform:
                
                    The change in transformation since the last call of move.

        .. py:method:: abort()

            Aborts the present recognized gesture and sliding caused by inertia.

