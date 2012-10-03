User Interface Classes
======================

The namespace libavg.ui contains python modules that expose higher-level user interface
functionality

.. automodule:: libavg.ui
    :no-members:

    .. inheritance-diagram:: DragRecognizer TapRecognizer TransformRecognizer DoubletapRecognizer HoldRecognizer
        :parts: 1

    .. inheritance-diagram:: Button ToggleButton Keyboard
        :parts: 1


    .. autoclass:: Button(upNode, downNode, [disabledNode=None, activeAreaNode=None, fatFingerEnlarge=False, clickHandler=None])

        A button that shows different user-supplied nodes depending on it's
        state. Possible button states are up, down and disabled. The nodes are attached
        as children to the Button on construction. For a simple button, image nodes can 
        be passed. Uses the :py:class:`TapRecognizer` to detect clicks.
        
        .. image:: ButtonStates.png

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

        **Callbacks:**

            .. py:method:: clickHandler(event)

                Called when the button is clicked.

        .. py:attribute:: enabled

            :py:const:`True` if the button accepts input. If the button is disabled,
            it shows the :py:attr:`disabledNode`.

        .. py:classmethod:: fromSrc(upSrc, downSrc[, disabledSrc=None, **kwargs]) -> Button

            Factory method that creates a button from filenames of the images to be
            displayed for different states.


    .. autoclass:: DoubletapRecognizer(node, [maxTime=MAX_DOUBLETAP_TIME, initialEvent=None, possibleHandler=None, failHandler=None, detectedHandler=None])

        A :py:class:`DoubletapRecognizer` detects doubletaps: Two short touches in quick
        succession without a large change of the cursor position.

        :param maxTime: The maximum time that each phase of the tap may take.


    .. autoclass:: DragRecognizer(eventNode, [coordSysNode=None, initialEvent=None, direction=ANY_DIRECTION, directionTolerance=DIRECTION_TOLERANCE, friction=-1, possibleHandler=None, failHandler=None, detectedHandler=None, moveHandler=None, upHandler=None, endHandler=None])

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

                :param event: 
                
                    The corresponding cursor motion event. If there was no event, 
                    this parameter is :py:const:`None`.

                :param avg.Point2D offset: 
                
                    The current offset from the start of the drag in coordinates relative
                    to the :py:attr:`coordSysNode`'s parent.

            .. py:method:: Recognizer.UP(offset)

                Emitted when the cursor is released. If inertia is enabled, there may be 
                move events after the up event.

                :param event: 
                
                    The corresponding :py:const:`CURSORUP` event.

                :param avg.Point2D offset: 
                
                    The current offset from the start of the drag in coordinates relative
                    to the :py:class:`coordSysNode`'s parent.

        .. py:method:: abort()

            Aborts the present recognized gesture and sliding caused by inertia

        .. py:method:: abortInertia()

            Causes inertia processing to end immediately.


    .. autoclass:: HoldRecognizer(node, [delay=HOLD_DELAY, initialEvent=None, possibleHandler=None, failHandler=None, detectedHandler=None, stopHandler=None])

        A :py:class:`HoldRecognizer` detects if a touch is held for a certain amount of 
        time. Holds are continuous events: the :py:meth:`stopHandler` is called when the
        contact up event arrives.

        :param delay: 
        
            The amount of time that has to pass before the hold is recognized.
    

    .. autoclass:: Keyboard(bgHref, ovlHref, keyDefs, shiftKeyCode, [altGrKeyCode, stickyShift, selHref, textarea])

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

                [(<keycode>, <shift keycode>, <altgr keycode>), <feedback>, <repeat>, 
                <pos>, <size>]

            or command keys:

                [<keycode>, <feedback>, <repeat>, <pos>, <size>]

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

        :param string selHref:

            Filename of an image that contains the keyboard feedback by pressed keys.
            If this parameter not set the feedback funktion is turned off.

        :param textarea textarea:

            Connect the keyboard upHandler instant to the textarea input.

        .. py:method:: reset()

            Resets any sticky keys (shift, altgr) to their default state.

        .. py:method:: setKeyHandler(self, downHandler, [upHandler])

            Set callbacks to invoke on key press and -release. Handlers take three 
            paramters: (event, char, cmd)

            :param downHandler: Callable to invoke on key down event or :py:const:`None`.
            :param upHandler: Callable to invoke on key up event or :py:const:`None`.

        .. py:classmethod:: makeRowKeyDefs(startPos, keySize, spacing, feedbackStr, keyStr, shiftKeyStr, [altGrKeyStr])

            Creates key definitions for a row of uniform keys. Useful for creating the 
            keyDefs parameter of the Keyboard constructor. All the keys get no repeat 
            functionality.

            :param avg.Point2D startPos: Top left position of the row.

            :param avg.Point2D keySize: Size of each key.

            :param int spacing: Number of empty pixels between two keys.

            :param string feedbackStr:

                String containing if the key has a feedback use f for :py:const:`False`
                and t for py:const:`True` (i.e. :samp:`"fttttttttttf"`)

            :param string keyStr: 
            
                Unicode string containing the unshifted keycodes (i.e. 
                :samp:`u"qwertzuiopżś"`)

            :param string shiftKeyStr: 
            
                Unicode string containing the shifted keycodes
                (i.e. :samp:`u"QWERTZUIOPńć"`)

            :param string altGrKeyStr: 
            
                Unicode string containing the keycodes when altgr is pressed.
    
    
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


    .. autoclass:: TapRecognizer(node, [maxTime=MAX_TAP_TIME, initialEvent=None, possibleHandler=None, failHandler=None, detectedHandler=None])

        A :py:class:`TapRecognizer` detects short touches without a large change of the 
        cursor position.

        :param maxTime: The maximum time that the tap may take in milliseconds.


    .. autoclass:: ToggleButton(uncheckedUpNode, uncheckedDownNode, checkedUpNode, checkedDownNode, [uncheckedDisabledNode=None, checkedDisabledNode=None, activeAreaNode=None, fatFingerEnlarge=False, checkHandler=None, uncheckHandler=None, enabled=True, checked=False])

        A button that can be used to toggle between checked and unchecked states. 
        Classical GUI checkboxes are an example of this kind of button.
        
        A :py:class:`ToggleButton` has a total of six visual states. In addition to the
        distinction between checked and unchecked, a button can be enabled or disabled.
        Buttons also change their appearance as soon as they are touched, leading to two
        further states. For each visual state, a node is passed as constructor parameter.
        The constructor attaches the nodes to the :py:class:`ToggleButton`. Simple
        ToggleButtons can be constructed by passing image filenames to 
        the :py:func:`fromSrc` factory function.

        Uses the :py:class:`TapRecognizer` to detect clicks.

        .. image:: ToggleButtonStates.png

        :param avg.Node uncheckedUpNode: 
        
            The node displayed when the button is unchecked and not touched.

        :param avg.Node uncheckedDownNode: 
            
            The node displayed when the button is unchecked and touched.

        :param avg.Node checkedUpNode: 
        
            The node displayed when the button is checked and not touched.

        :param avg.Node checkedDownNode: 
        
            The node displayed when the button is checked and not touched.

        :param avg.Node uncheckedDisabledNode: 
        
            The node displayed when the button is unchecked and disabled.

        :param avg.Node checkedDisabledNode: 
        
            The node displayed when the button is checked and disabled.

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

        :param bool checked:

            If this parameter is set to :py:const:`True`, the button starts in the checked
            state.

        :param bool enabled:

            If this parameter is set to :py:const:`True`, the button starts in the 
            disabled state.

        **Callbacks:**

            .. py:method:: checkedHandler(event)

                Called when the button is checked.

            .. py:method:: uncheckedHandler(event)

                Called when the button is unchecked.

        .. py:attribute:: checked

            The state of the toggle.

        .. py:attribute:: enabled

            Determines whether the button accepts input.

        .. py:method:: getState() -> String

            Returns the visual state of the button as a string.

        .. py:classmethod:: fromSrc(uncheckedUpSrc, uncheckedDownSrc, checkedUpSrc, checkedDownSrc, [uncheckedDisabledSrc=None, checkedDisabledSrc=None **kwargs]) -> ToggleButton

            Factory method that creates a togglebutton from filenames of the images to be
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

