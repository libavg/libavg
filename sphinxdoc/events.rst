Input Handling
==============

.. automodule:: libavg.avg
    :no-members:

    .. inheritance-diagram:: Event CursorEvent MouseEvent MouseWheelEvent TouchEvent KeyEvent TangibleEvent
        :parts: 1

    .. inheritance-diagram:: Contact
        :parts: 1

    .. autoclass:: Contact

        A Contact encapsulates the information of one touch on an input device from
        the down event to an up event. It exposes some aggregate information about the
        touch - distance and direction travelled etc. - and supports event handlers that
        are only called for this single contact.

        For compatibility reasons, a mouse device also produces contacts. A mouse contact
        exists from the press of a button to its release. If multiple buttons are
        pressed without a complete release (e.g. LEFTDOWN-RIGHTDOWN-LEFTUP-RIGHTUP), the
        mouse contact exists for the complete sequence. 

        **Messages:**

            All message parameters are of type :py:class:`CursorEvent`.  
            To get these messages, call :py:meth:`Publisher.subscribe`. All subscribers
            are unsubscribed automatically after the up event.

            .. py:method:: CURSOR_MOTION(cursorevent)
            
                Emitted whenever the contact moves.

            .. py:method:: CURSOR_UP(cursorevent)
            
                Emitted when the mouse button is released or the touch leaves the surface.

        .. py:attribute:: age

            Time that has passed since the down event in milliseconds. Read-only.

        .. py:attribute:: distancefromstart

            Distance of the current position from the initial position in pixels. 
            Read-only.

        .. py:attribute:: distancetravelled

            The total distance travelled since the initial down event. Read-only.

        .. py:attribute:: events

            An array containing all events that this contact has generated in the past.
            Read-only.

        .. py:attribute:: id

            A numerical id for this contact. This corresponds to the 
            :py:attr:`CursorEvent.cursorid` field. Contacts for touch events have unique
            ids, while contacts for mouse events always have the :py:attr:`id`
            :py:const:`-1`. ids are not reused. Read-only.

        .. py:attribute:: motionangle

            Angle of the current position from the initial position in radians. Like all
            angles in libavg, :py:attr:`motionangle` is 0 on the positive x axis and 
            increases clockwise. Read-only.

        .. py:attribute:: motionvec

            The difference of the current position and the initial position as a
            :py:class:`Point2D`. Read-only.

        .. py:method:: connectListener(motionCallback, upCallback) -> id

            .. deprecated:: 1.8
                Use the message interface instead.

            Registers event handlers that get called when CURSORMOTION and CURSORUP 
            events for this :py:class:`Contact` occur. Event handlers can be unregistered
            using :py:meth:`disconnectListener`. They are automatically unregistered
            after the up event. The :py:attr:`id` returned is unique for this contact.

        .. py:method:: disconnectListener(id)

            .. deprecated:: 1.8
                Use the message interface instead.

            Unregisters an event handler. The parameter is the :py:attr:`id` returned in 
            :py:meth:`connectListener`. It is an error to call 
            :py:meth:`disconnectListener` with an invalid id.

        .. py:method:: getRelPos(node, abspos) -> relpos

            Transforms a position in window coordinates to a position in coordinates
            relative to :py:attr:`node`. :py:attr:`node` must be one of the nodes
            the original down event of the :py:class:`Contact` was over.
            In contrast to :py:meth:`Node.getRelPos`, this method transforms window
            coordinates even if the node is inside a canvas.


    .. autoclass:: CursorEvent

        Base class for all events which contain a position in the global coordinate
        system.
    
        .. py:attribute:: contact

            The :py:class:`Contact` that the event belongs to, if there is one. 
            Read-only.

        .. py:attribute:: cursorid

            A numerical identifier for the current cursor.

        .. py:attribute:: node

            The :py:class:`Node` that the event occured in. If this is :py:const:`None`,
            the event happened outside of the application window and the cursor was 
            captured by the application.
            Read-only.

        .. py:attribute:: pos

            Position in the global coordinate system. Read-only.

        .. py:attribute:: source

            The type of the device that emitted the event. See :py:attr:`Event.source`. 
            Read-only.

        .. py:attribute:: x

            x position in the global coordinate system. Read-only.

        .. py:attribute:: y

            y position in the global coordinate system. Read-only.

        .. py:attribute:: userid

            If the input source supports user identification, this is the id of the user
            that touched.


    .. autoclass:: Event(type, source, [when])

        Base class for user input events.

        :param type type:

            The type of the event. See :py:attr:`Event.type`.

        :param source source:

            The source of the event. See :py:attr:`Event.source`.

        :param Integer when:

            The time the event occured

        .. py:attribute:: inputdevice
            
            The address of the device that emitted the event.
            Read-only

        .. py:attribute:: inputdevicename

            The name of the device that emitted the event.
            Read-only.

        .. py:attribute:: source

            One of :py:const:`MOUSE`, :py:const:`TOUCH`, :py:const:`TRACK`,
            :py:const:`CUSTOM` or :py:const:`NONE`. Read-only

        .. py:attribute:: type

            One of :py:const:`KEYUP`, :py:const:`KEYDOWN`, :py:const:`CURSORMOTION`,
            :py:const:`CURSORUP`, :py:const:`CURSORDOWN`, :py:const:`CURSOROVER` 
            or :py:const:`CURSOROUT`. Read-only.

        .. py:attribute:: when

            The time when the event occured in milliseconds since program start. 
            Read-only.

    .. autoclass:: KeyEvent

        Generated when a key is pressed or released.

        .. py:attribute:: keyname

            The name of the key according to the current keyboard layout. This can be a
            character like "A" or a word like "Up" for the up arrow key. Read-only.

        .. py:attribute:: text

            The text that the key represents, if any. Handles shifted (i.e., 
            uppercase) characters and dead key combinations (i.e., "á" if "'" and "a" are
            pressed in succession).

        .. py:attribute:: modifiers

            Any modifier keys pressed, or'ed together. Possible Modifiers are
            :py:const:`KEYMOD_NONE`, :py:const:`KEYMOD_LSHIFT`, :py:const:`KEYMOD_RSHIFT`,
            :py:const:`KEYMOD_LCTRL`, :py:const:`KEYMOD_RCTRL`, :py:const:`KEYMOD_LALT`,
            :py:const:`KEYMOD_RALT`, :py:const:`KEYMOD_LGUI`, :py:const:`KEYMOD_RGUI`,
            :py:const:`KEYMOD_NUM`, :py:const:`KEYMOD_CAPS`, :py:const:`KEYMOD_GUI`,
            :py:const:`KEYMOD_CTRL`, :py:const:`KEYMOD_SHIFT`, and :py:const:`KEYMOD_ALT`.
            Read-only.

        .. py:attribute:: scancode

            A value that represents the physical position of the key on the keyboard.
            Independent of the keyboard language/layout. Read-only.


    .. autoclass:: MouseEvent(type, leftButtonState, middleButtonState, rightButtonState, pos, button, [speed, when])

        Generated when a mouse-related event occurs.

        .. py:attribute:: button

            The button that caused the event. Read-only.

        .. py:attribute:: cursorid

            Always :samp:`-1` for mouse events, but can be used to handle mouse and 
            tracking events in one handler. Read-only.

        .. py:attribute:: leftbuttonstate

            :py:const:`True` if the left mouse button is currently pressed. Read-only.

        .. py:attribute:: middlebuttonstate

            :py:const:`True` if the middle mouse button is currently pressed. Read-only.

        .. py:attribute:: rightbuttonstate

            :py:const:`True` if the right mouse button is currently pressed. Read-only.

        .. py:attribute:: source

            Always :py:const:`MOUSE`. Read-only

        .. py:attribute:: speed

            Current speed of the mouse in pixels per millisecond as a
            :py:class:`Point2D`. Read-only.

    .. autoclass:: MouseWheelEvent(pos, motion, [when])

        Generated when the mouse wheel is moved.

        .. py:attribute:: motion

            Direction and magnitude of movement as an (x,y) vector.

    .. autoclass:: TangibleEvent(id, markerID, type, pos, speed, orientation)

        Generated when a tangible event occurs. Tangible events happen when a surface that
        supports marker-based tracking is active. Supported only for TUIO-based surfaces.

        .. py:attribute:: markerID

            The id of the marker. Unlike :py:attr:`cursorid`, :py:attr:`markerID` is
            persistent and stays the same if a tangible is removed from the surface and
            re-placed again.

        .. py:attribute:: orientation

            The angle of the marker in radians.

    .. autoclass:: TouchEvent(id, type, pos, source, [speed])

        Generated when a touch or other tracking event occurs. Touch events happen 
        only when a multi-touch sensitive surface or other camera tracker is 
        active. 

        .. py:attribute:: area

            Size of the blob found in pixels. Read-only.

        .. py:attribute:: center

            Position as :py:class:`Point2D`, with sub-pixel accuracy. Used for 
            calibration. Read-only.

        .. py:attribute:: cursorid

            An identifier for the current touch. A single touch will generate a down 
            event, zero or more motion events and a single up event in its lifetime, all
            with the same :py:attr:`cursorid`.

        .. py:attribute:: eccentricity

        .. py:attribute:: handorientation

            The angle of the hand relative to the finger. :py:attr:`handorientation` is
            only defined for events with :py:attr:`source` = :py:const:`TOUCH`. If the 
            tracker has detected a hovering hand attached to the finger, this is the 
            actual hand-finger angle. If no hand was detected, the angle is approximated 
            using the position of the touch on the surface. :py:attr:`handorientation`
            ranges from :py:const:`-pi` to :py:const:`pi`, with 0 being the positive x
            axis. Angles increase in a clockwise fashion.

            For :py:const:`CURSORUP` events, the angle is always approximated.

        .. py:attribute:: majoraxis

            Major axis of an ellipse that is similar to the blob. Read-only.

        .. py:attribute:: minoraxis

            Minor axis of an ellipse that is similar to the blob. Read-only.

        .. py:attribute:: orientation

            Angle of the blob in radians. For hovering hands, this is roughly the 
            direction of the hand, modulo 180 degrees. Read-only.

        .. py:attribute:: source

            :py:attr:`source` can be either :py:const:`TRACK` or :py:const:`TOUCH`.
            In most cases, actual touches will generate :py:const:`TOUCH` events. When 
            used with a DI device, the internal tracker also generates :py:const:`TRACK` 
            events for hands above the surface. When used with an FTIR device, the 
            internal tracker generates :py:const:`TRACK` events for the actual touches.
            Read-only.

        .. py:attribute:: speed

            Current speed of the touch in pixels per millisecond as a
            :py:class:`Point2D`. Read-only.

        .. py:method:: getRelatedEvents() -> events

            Only if supported by the hardware: Returns a python tuple 
            containing the events 'related' to this one. For :py:const:`TOUCH` events 
            (fingers), the tuple contains one element: the corresponding 
            :py:const:`TRACK` event (hand). For :py:const:`TRACK` events,
            the tuple contains all :py:const:`TOUCH` events that belong to the same hand.

