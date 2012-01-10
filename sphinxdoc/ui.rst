User Interface Classes
======================

The namespace libavg.ui contains python modules that expose higher-level user interface
functionality

.. automodule:: libavg.ui
    :no-members:

    .. inheritance-diagram:: DragRecognizer TapRecognizer TransformRecognizer DoubletapRecognizer HoldRecognizer
        :parts: 1

    .. inheritance-diagram:: Button TouchButton Keyboard
        :parts: 1

    .. autoclass:: Button(upNode, downNode[, disabledNode=None, activeAreaNode=None, pressHandler=None, clickHandler=None, stateChangeHandler=None])

        A generic button that shows different user-supplied nodes depending on it's
        state. Possible button states are up, down and disabled. The nodes are attached
        as children to the Button on construction. For a simple button, image nodes can 
        be passed. Button behaviour corresponds to standard GUI buttons.

        :param avg.Node upNode: The node displayed when the button is not pressed.

        :param avg.Node downNode: The node displayed when the button is pressed.

        :param avg.Node disabledNode: The node displayed when the button is disabled.

        :param avg.Node activeAreaNode: 
        
            A node that is used only to determine if a click is over the button. Usually,
            this node is invisible. :py:attr:`activeAreaNode` is useful for small touch
            buttons, where the active area should be larger than the visible button to
            accout for touch inaccuracies.

        Callbacks:

            .. py:method:: pressHandler(event)

                Called when the button is pressed. This happens on a down event.
                
                :param event: The corresponding cursor down event. 

            .. py:method:: clickHandler(event)

                Called when the button is clicked. A click is generated when an up event
                happens inside the button.

            .. py:method:: stateChangeHandler(state)

                Called whenever the button state changes.

        .. py:method:: delete()

        .. py:method:: getUpNode() -> Node

        .. py:method:: getDownNode() -> Node

        .. py:method:: getDisabledNode() -> Node

        .. py:method:: setEnabled(isEnabled)

        .. py:method:: isEnabled()


    .. autoclass:: DoubletapRecognizer(node, [eventSource=avg.TOUCH | avg.MOUSE, maxTime=MAX_DOUBLETAP_TIME, initialEvent=None, possibleHandler=None, failHandler=None, detectedHandler=None])

        A :py:class:`DoubletapRecognizer` detects doubletaps: Two short touches in quick
        succession without a large change of the cursor position.

        :param maxTime: The maximum time that each phase of the tap may take.


    .. autoclass:: DragRecognizer(eventNode, [coordSysNode=None, eventSource=avg.TOUCH | avg.MOUSE, initialEvent=None, direction=ANY_DIRECTION, directionTolerance=pi/4, friction=-1, possibleHandler=None, failHandler=None, detectedHandler=None, moveHandler=None, upHandler=None, endHandler=None])

        A :py:class:`DragRecognizer` attaches itself to a node's cursor events and 
        delivers higher-level callbacks that can be used to implement dragging or 
        drag-like functionality.

        :py:class:`DragRecognizer` supports inertia after the node is released.
        
        :param avg.Node eventNode: 
        
            The node to attach to. The :py:class:`DragRecognizer` registers an event 
            handler to react to any contacts for this node. 
            
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

        :param float friction:

            If set, this parameter enables inertia processing. It describes how 
            quickly the drag comes to a stop after the cursor is released.

        Callbacks:

            .. py:method:: moveHandler(event, offset)

                Called when the drag should cause a position change. This usually happens
                in response to a :py:const:`CURSORMOTION` event, but may also happen
                because of inertia.

                :param event: 
                
                    The corresponding cursor motion event. If there was no event, 
                    this parameter is :py:const:`None`.

                :param avg.Point2D offset: 
                
                    The current offset from the start of the drag in coordinates relative
                    to the :py:attr:`coordSysNode`'s parent.

            .. py:method:: upHandler(event, offset)

                Called when the cursor is released. If inertia is enabled, there may be 
                move events after the up event.

                :param event: 
                
                    The corresponding :py:const:`CURSORUP` event.

                :param avg.Point2D offset: 
                
                    The current offset from the start of the drag in coordinates relative
                    to the :py:class:`coordSysNode`'s parent.

            .. py:method:: endHandler()

                Called when movement stops. This is either directly after the up event
                or when inertia has run its course.

        .. py:method:: abort()

            Aborts the present recognized gesture and sliding caused by inertia

        .. py:method:: abortInertia()

            Causes inertia processing to end immediately.


    .. autoclass:: HoldRecognizer(node, [eventSource=avg.TOUCH | avg.MOUSE, delay=HOLD_DELAY, initialEvent=None, possibleHandler=None, failHandler=None, detectedHandler=None, stopHandler=None])

        A :py:class:`HoldRecognizer` detects if a touch is held for a certain amount of 
        time. Holds are continuous events: the :py:meth:`stopHandler` is called when the
        contact up event arrives.

        :param delay: 
        
            The amount of time that has to pass before the hold is recognized.
    

    .. autoclass:: Keyboard(bgHref, ovlHref, keyDefs, shiftKeyCode, [altGrKeyCode, stickyShift])

        Implements an onscreen keyboard that turns mouse clicks or touches into key 
        presses. The keyboard is completely configurable. Keyboard graphics are determined
        by the two image files in bgHref and ovlHref. Keys can be defined as rectangles 
        anywhere on these images. Works for both single-touch and multitouch devices. 
        When a key is pressed, a callback function is invoked.

        Needs offscreen rendering support on the machine.

        :param string bgHref: 
        
            Filename of an image that contains the keyboard with unpressed keys.

        :param string ovlHref:
        
            Filename of an image that contains the keyboard with pressed keys.

        :param list keyDefs:

            List of key definitions. Keys can be either character keys:

                [(<keycode>, <shift keycode>, <altgr keycode>), <pos>, <size>]

            or command keys:

                [<keycode>, <pos>, <size>]

            For character keys, the shift and altgr keycodes are optional. To define
            entire rows of evenly-spaced keys, use :py:meth:`makeRowKeyDefs`.

        :param shiftKeyCode:

            One of the command keycodes. When the key with this code is pressed,
            pressing other keys causes them to return the shifted keycode.

        :param altGrKeyCode:

            One of the command keycodes. When the key with this code is pressed,
            pressing other keys causes them to return the altgr keycode.

        :param bool stickyShift:

            For single-touch devices, the shift key must stay in the pressed state
            until the next normal key is pressed to have any effect. This is the 
            behaviour if :py:attr:`stickyShift` is :py:const:`True`. If it is 
            :py:const:`False` (the default), a 
            multitouch device is assumed and shift works like on a physical keyboard.

        .. py:method:: reset()

            Resets any sticky keys (shift, altgr) to their default state.

        .. py:method:: setKeyHandler(self, downHandler, [upHandler])

            Set callbacks to invoke on key press and -release. Handlers take three 
            paramters: (event, char, cmd)

            :param downHandler: Callable to invoke on key down event or `None`.
            :param upHandler: Callable to invoke on key up event or :py:const:`None`.

        .. py:classmethod:: makeRowKeyDefs(startPos, keySize, spacing, keyStr, shiftKeyStr, [altGrKeyStr])

            Creates key definitions for a row of uniform keys. Useful for creating the 
            keyDefs parameter of the Keyboard constructor.

            :param avg.Point2D startPos: Top left position of the row.

            :param avg.Point2D keySize: Size of each key.

            :param int spacing: Number of empty pixels between two keys.

            :param string keyStr: 
            
                Unicode string containing the unshifted keycodes (i.e. 
                :samp:`u"qwertzuiopżś"`)

            :param string shiftKeyStr: 
            
                Unicode string containing the shifted keycodes
                (i.e. :samp:`u"QWERTZUIOPńć"`)

            :param string altGrKeyStr: 
            
                Unicode string containing the keycodes when altgr is pressed.
    
    
    .. autoclass:: Recognizer(node, isContinuous, eventSource, maxContacts, initialEvent[, possibleHandler=None, failHandler=None, detectedHandler=None, endHandler=None])

        Base class for gesture recognizers that attach to a node's cursor events and 
        emit higher-level events. Gesture recognizers have a standard set of states and
        callbacks, but derived classes may add their own callbacks and do not need to
        invoke all base class callbacks. The possible states vary depending on the value 
        of :py:attr:`isContinuous`:

        .. image:: Recognizer.png

        A usage example for the recognizers can be found under
        :samp:`src/samples/gestures.py`.

        :param Node node: Node to attach to.

        :param bool isContinuous: 
            
            :py:const:`True` if the gesture stays active after it has been detected.
        

        :param eventSource: 
        
            One of the standard event sources (:py:const:`TRACK`, :py:const:`TOUCH` 
            etc.).

        :param maxContacts:

            The maximum number of contacts that the recognizer should handle. 
            :py:const:`None` if there is no maximum.

        :param initialEvent:

            A cursordown event to pass to the recognizer immediately.

        Callbacks:

            .. py:method:: possibleHandler(event)

                Called when gesture recognition begins - usually after a cursordown event.
                Some continuous gestures (such as unconstrained drags) never invoke 
                :py:meth:`possibleHandler` but call :py:meth:`detectedHandler` 
                immediately.

            .. py:method:: failHandler(event) 

                Called when gesture recognition is rejected.

            .. py:method:: detectedHandler(event)

                Called when the gesture is recognized. For discrete gestures, this 
                signifies the end of gesture processing. 

            .. py:method:: endHandler(event)

                Called when a continuous gesture ends.

        .. py:method:: abort()

            Aborts the present recognized gesture

        .. py:method:: enable(isEnabled)

            Enables or disables the :py:class:`Recognizer`.

        .. py:method:: getState() -> String

            Returns the state ("IDLE", "POSSIBLE" or "RUNNING") of the recognizer.


    .. autoclass:: TapRecognizer(node, [eventSource=avg.TOUCH | avg.MOUSE, maxTime=MAX_TAP_TIME, initialEvent=None, possibleHandler=None, failHandler=None, detectedHandler=None])

        A :py:class:`TapRecognizer` detects short touches without a large change of the 
        cursor position.

        :param maxTime: The maximum time that the tap may take in milliseconds.


    .. autoclass:: TouchButton(upNode, downNode, disabledNode=None, activeAreaNode=None, fatFingerEnlarge=False, clickHandler=None])

        A button made specifically for touch input. Uses the :py:class:`TapRecognizer` to
        detect clicks.

        :param avg.Node upNode: The node displayed when the button is not pressed.

        :param avg.Node downNode: The node displayed when the button is pressed.

        :param avg.Node disabledNode: The node displayed when the button is disabled.

        :param avg.Node activeAreaNode: 
        
            A node that is used only to determine if a click is over the button. Usually,
            this node is invisible. :py:attr:`activeAreaNode` is useful for small touch
            buttons, where the active area should be larger than the visible button to
            account for touch inaccuracies.

        :param bool fatFingerEnlarge:

            If this parameter is set to :py:const:`True`, the button generates it's own 
            internal :py:attr:`activeAreaNode` that is at least 20x20mm large. 
            :py:attr:`fatFingerEnlarge` is incompatible with a custom 
            :py:attr:`activeAreaNode`.

        Callbacks:

            .. py:method:: clickHandler(event)

                Called when the button is clicked.

        .. py:attribute:: enabled

            :py:const:`True` if the button accepts input. If the button is disabled,
            it shows the :py:attr:`disabledNode`.

        .. py:classmethod:: fromSrc(upSrc, downSrc[, disabledSrc=None, **kwargs]) -> Button

            Factory method that creates a button from filenames of the images to be
            displayed for different states.

    
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


    .. autoclass:: TransformRecognizer(eventNode, [coordSysNode=None, eventSource=avg.TOUCH | avg.MOUSE, initialEvent=None, friction=-1, detectedHandler=None, moveHandler=None, upHandler=None, endHandler=None])

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

        Callbacks:

            .. py:method:: moveHandler(transform)

                Called whenever the transform changes.

                :param Transform transform:
                
                    The change in transformation since the last call of move or up.

            .. py:method:: upHandler(transform)

                Called when the last touch is released.

                :param transform:
                
                    The change in transformation since the last call of move.

            .. py:method:: endHandler()

                Called when movement stops. This is either directly after the up event
                or when inertia has run its course.

        .. py:method:: abort()

            Aborts the present recognized gesture and sliding caused by inertia

