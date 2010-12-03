Input Handling
==============

.. automodule:: libavg.avg
    :no-members:

    .. autoclass:: Event

        Base class for user input events.

        .. py:attribute:: type

            One of :py:const:`KEYUP`, :py:const:`KEYDOWN`, :py:const:`CURSORMOTION`,
            :py:const:`CURSORUP`, :py:const:`CURSORDOWN`, :py:const:`CURSOROVER`, 
            :py:const:`CURSOROUT` or :py:const:`QUIT`. Read-only.

        .. py:attribute:: when

            The time when the event occured in milliseconds since program start. 
            Read-only.

    .. autoclass:: KeyEvent

        Generated when a key is pressed or released.

        .. py:attribute:: keycode

            The keycode of the key according to US keyboard layout. Read-only. 

        .. py:attribute:: keystring

            A character or word describing the key pressed. Read-only.

        .. py:attribute:: modifiers

            Any modifier keys (:kbd:`shift`, :kbd:`ctrl`, etc.) pressed. This is a 
            number of KeyModifier values or'ed together. Read-only. 

        .. py:attribute:: scancode

            The untranslated (hardware-dependent) scancode of the key pressed. 
            Read-only.

        .. py:attribute:: unicode

            Unicode index of the character. Takes into account the current keyboard
            layout and any modifiers pressed. This attribute is only filled in the
            :py:const:`KEYDOWN` event. Read-only.

    .. autoclass:: MouseEvent

        Generated when a mouse-related event occurs.

        .. py:attribute:: button

            The button that caused the event. Read-only.

        .. py:attribute:: cursorid

            Always :samp:`-1` for mouse events, but can be used to handle mouse and 
            tracking events in one handler. Read-only.

        .. py:attribute:: lastdownpos

            The position of the last mouse down event with the same button.
            Useful for implementing dragging. Read-only.

        .. py:attribute:: leftbuttonstate

            :keyword:`True` if the left mouse button is currently pressed. Read-only.

        .. py:attribute:: middlebuttonstate

            :keyword:`True` if the middle mouse button is currently pressed. Read-only.

        .. py:attribute:: node

            The node that the event occured in. Read-only.

        .. py:attribute:: pos

            Position in the global coordinate system. Read-only.

        .. py:attribute:: rightbuttonstate

            :keyword:`True` if the right mouse button is currently pressed. Read-only.

        .. py:attribute:: source

            Always :py:const:`MOUSE`.

        .. py:attribute:: speed

            Current speed of the mouse in pixels per millisecond as a
            :py:class:`Point2D`. Read-only.

        .. py:attribute:: x

            x position in the global coordinate system. Read-only.

        .. py:attribute:: y

            y position in the global coordinate system. Read-only.

    .. autoclass:: TouchEvent

        Generated when a touch or other tracking event occurs. Touch events happen 
        only when a multi-touch sensitive surface or other camera tracker is 
        active. 

        .. py:attribute:: area

            Size of the blob found in pixels. Read-only.

        .. py:attribute:: center

            Position as :py:class:`Point2D`, with sub-pixel accuracy. Used for 
            calibration. Read-only.

        .. py:attribute:: cursorid

            An identifier for the current touch. A single touch will generate a down,
            zero or more motion and a single up event in its lifetime, all with the same
            :py:attr:`cursorid`.

        .. py:attribute:: eccentricity

        .. py:attribute:: lastdownpos

            The initial position of the cursor. Useful for implementing dragging.

        .. py:attribute:: majoraxis

            Major axis of an ellipse that is similar to the blob. Read-only.

        .. py:attribute:: minoraxis

            Minor axis of an ellipse that is similar to the blob. Read-only.

        .. py:attribute:: node

            The node that the event occured in. Read-only.

        .. py:attribute:: orientation

            Angle of the blob in radians. For hovering hands, this is roughly the 
            direction of the hand, modulo 180 degrees. Read-only.

        .. py:attribute:: pos

            Position in the global coordinate system. Read-only.

        .. py:attribute:: source

            :py:attr:`source` can be either :py:const:`TRACK` or :py:const:`TOUCH`.
            In most cases, actual touches will generate :py:const:`TOUCH` events. When 
            used with a DI device, the internal tracker also generates :py:const:`TRACK` 
            events for hands above the surface. When used with an FTIR device, the 
            internal tracker generates :py:const:`TRACK` events for the actual touches.

        .. py:attribute:: speed

            Current speed of the touch in pixels per millisecond as a
            :py:class:`Point2D`. Read-only.

        .. py:attribute:: x

            x position in the global coordinate system. Read-only.

        .. py:attribute:: y

            y position in the global coordinate system. Read-only.

        .. py:method:: getContour() -> list

            Returns the contour of the blob as a list of points if supported by the 
            tracker being used.

        .. py:method:: getRelatedEvents() -> events

            Only for DI devices and the internal tracker: Returns a python tuple 
            containing the events 'related' to this one. For :py:const:`TOUCH` events 
            (fingers), the tuple contains one element: the corresponding 
            :py:const:`TRACK` event (hand). For :py:const:`TRACK` events,
            the tuple contains all :py:const:`TOUCH` events that belong to the same hand.

    .. autoclass:: Tracker

        A class that uses a camera to track moving objects and delivers the movements 
        as avg events. Create a tracker by using :py:meth:`Player.addTracker()`.
        The properties of this class are explained under
        https://www.libavg.de/wiki/index.php/Tracker_Setup.

        This is the internal libavg tracker. For trackers created using 
        :py:meth:`Player.enableMultitouch`, no Tracker object exists.
        
        .. py:method:: abortCalibration()

            Aborts coordinate calibration session and restores the previous
            coordinate transformer.

        .. py:method:: endCalibration()

            Ends coordinate calibration session and activates the coordinate
            transformer generated.

        .. py:method:: getDisplayROIPos()

        .. py:method:: getDisplayROISize()

        .. py:method:: getImage(imageid) -> Bitmap 

            Returns one of the intermediate images necessary for tracking.
            These images are only available if setDebugImages was called before
            with appropriate parameters. Possible :py:attr:`imageid` values are 
            :py:const:`IMG_CAMERA`, :py:const:`IMG_DISTORTED`, :py:const:`IMG_NOHISTORY`,
            :py:const:`IMG_HISTOGRAM`, :py:const:`IMG_FINGERS` or 
            :py:const:`IMG_HIGHPASS`.

        .. py:method:: getParam(element) -> value

            Returns a tracker configuration parameter.

        .. py:method:: resetHistory()

            Throws away the current history image and generates a new one from
            the next second of images.

        .. py:method:: saveConfig()

            Saves the current tracker configuration to the default config file.

        .. py:method:: setDebugImages(img, finger)

            Controls whether debug images of intermediate tracking results
            and detected finger positions are generated and exported to
            python. Generating the debug images takes a moderate amount of
            time, so it is turned off by default.

            :param img: Whether to generate intermediate result images.
            :param finger: Whether to generate the :py:const:IMG_FINGERS: result image.

        .. py:method:: setParam(element, value)

            Sets one of the tracker configuration parameters.

        .. py:method:: startCalibration(displayextents) -> TrackerCalibrator

            Starts coordinate calibration session. The returned 
            :py:class:`TrackerCalibrator` exists until :py:meth:`endCalibration`
            or :py:meth:`abortCalibration` is called.

            :param displayextents: The width and height of the display area.

    .. autoclass:: TrackerCalibrator

        Generates a mapping of display points to camera points using a set of reference
        points. Python code should display reference points that the user must
        touch to establish a mapping. Created by :py:meth:`Tracker.startCalibration`.

        .. py:method:: getDisplayPoint() -> Point2D

        .. py:method:: nextPoint() -> bool

            Advances to the next point. Returns :keyword:`False` and ends calibration if
            all points have been set.

        .. py:method:: setCamPoint(pos)

