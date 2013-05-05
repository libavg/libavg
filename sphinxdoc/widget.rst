Widget Classes
==============

The libavg.widget module contains high-level user interface elements such as buttons and 
list boxes. Widgets are fully skinnable and multitouch-enabled.

.. note::

    The widget module is experimental. Functionality and interface are still in flux and
    subject to change.

.. automodule:: libavg.widget
    :no-members:

    .. inheritance-diagram:: HStretchNode VStretchNode HVStretchNode SwitchNode Button TextButton BmpButton ToggleButton CheckBox BmpToggleButton Keyboard Slider ScrollBar ScrollBarTrack ScrollBarThumb SliderThumb ProgressBar ScrollArea ScrollPane TimeSlider MediaControl Skin 
        :parts: 1


    .. autoclass:: BmpButton(upSrc, downSrc, [disabledSrc=None])

        A :py:class:`Button` that is created from image files. Internally, it creates two or
        three :py:class:`ImageNode`s and uses them as constructor parameters for
        :py:class:`Button`.

    .. autoclass:: Button(upNode, downNode, [disabledNode=None, activeAreaNode=None, fatFingerEnlarge=False, enabled=True, clickHandler=None])

        A button that shows different user-supplied nodes depending on its
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

            If this parameter is set to :py:const:`True`, the button generates its own 
            internal :py:attr:`activeAreaNode` that is at least 20x20mm large. 
            :py:attr:`fatFingerEnlarge` is incompatible with a custom 
            :py:attr:`activeAreaNode`.

        **Messages:**

            To get these messages, call :py:meth:`Publisher.subscribe`.

            .. py:method:: Button.PRESSED()

                Called when a tap on the button is initiated.

            .. py:method:: Button.RELEASED()

                Called when a tap on the button ends. Emitted for both successful and
                aborted taps.

            .. py:method:: Button.CLICKED()

                Called when the button is clicked.

        .. py:attribute:: enabled

            :py:const:`True` if the button accepts input. If the button is disabled,
            it shows the :py:attr:`disabledNode`.

    
    .. autoclass:: HStretchNode(endsExtent, [src=None, minExtent=-1]) 

        A node that stretches its graphics to fill the size given horizontally. It is used
        as base component for scrollbars and buttons. The base
        bitmap is split into three parts: left, center and right. The left and right parts
        are :py:attr:`endsExtent` wide and generated from the corresponding parts of the 
        source bitmap. The center part is generated from a one pixel wide slice of the 
        source bitmap and stretched to fill the space left between the left and right
        parts.

        :param int endsExtent:

            Width of the left and right bitmaps in pixels.

        :param src:

            Either the name of a bitmap file or a :py:class:`Bitmap` object. Used to
            generate the graphics used.

        :param int minExtent:

            Minimum horizontal size. The default of -1 uses :py:const:`2*endsExtent+1` as
            minimum.


    .. autoclass:: HVStretchNode(endsExtent, [src=None, minExtent=-1]) 

        A node that stretches its graphics to fill the size given horizontally and
        vertically. It is used as base component for scrollareas. Similar to 
        :py:class:`HStretchNode`, the base bitmap is split and partial bitmaps are
        extracted. Four corner bitmaps of size :py:attr:`endsExtent` stay the same size,
        four one-pixel-slice bitmaps fill the sides, and a single one-pixel bitmap is used
        to fill the center area.

        :param IntPoint endsExtent:

            Size of the corner bitmaps in pixels.

        :param src:

            Either the name of a bitmap file or a :py:class:`Bitmap` object. Used to
            generate the graphics used.

        :param IntPoint minExtent:

            Minimum size. The default of :py:const:`-1` uses :py:const:`2*endsExtent+1` as
            minimum.


    .. autoclass:: Keyboard(bgSrc, downSrc, keyDefs, shiftKeyCode, [altGrKeyCode=None, stickyShift=False, feedbackSrc=None])

        Implements an onscreen keyboard that turns mouse clicks or touches into key 
        presses. The keyboard is completely configurable. Keyboard graphics are determined
        by the two image files in :py:attr:`bgSrc` and :py:attr:`downSrc`. Keys can be
        defined as rectangles anywhere on these images. Works for both single-touch and 
        multitouch devices. Generates events when keys are pressed or released. 
        An additional enlarged image of the key being pressed can be rendered above a 
        pending touch as well by using :py:attr:`feedbackSrc`.

        Needs offscreen rendering support on the machine to generate individual key images
        from the image files supplied.

        :param string bgSrc: 
        
            Filename of an image that contains the keyboard with unpressed keys.

        :param string downSrc:
        
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

            One of the command keycodes. When a key with this code is pressed,
            pressing other keys causes them to return the shifted keycode.

        :param altGrKeyCode:

            One of the command keycodes. When a key with this code is pressed,
            pressing other keys causes them to return the altgr keycode.

        :param bool stickyShift:

            For single-touch devices, the shift key must stay in the pressed state
            until the next normal key is pressed to have any effect. This is the 
            behaviour if :py:attr:`stickyShift` is :py:const:`True`. If it is 
            :py:const:`False` (the default), a 
            multitouch device is assumed and shift works like on a physical keyboard.

        :param string feedbackSrc:

            Filename of an image that contains an enlarged version of bgSrc for use as
            feedback during key pressed. If this parameter not set the feedback funktion
            is turned off.

        **Messages:**

            :py:class:`Keyboard` emits messages on every key press and release:

            .. py:method:: DOWN(keycode)

            Emitted whenever a key (command or char) is pressed.

            .. py:method:: UP(keycode)

            Emitted whenever a key (command or char) is released.

            .. py:method:: CHAR(char)


            Emitted whenever a character is generated. This is generally when a char key
            is released and takes into account shift/altgr status.

        .. py:method:: reset()

            Resets any sticky keys (shift, altgr) to their default state.

        .. py:classmethod:: makeRowKeyDefs(startPos, keySize, spacing, feedbackStr, keyStr, shiftKeyStr, [altGrKeyStr])

            Creates key definitions for a row of uniform keys. Useful for creating the 
            keyDefs parameter of the Keyboard constructor. All the keys get no repeat 
            functionality.

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
    
   
    .. autoclass:: MediaControl([duration=1000, time=0])

        A composite control that incorporates a :py:class:`TimeSlider`, a play/pause 
        button and text widgets that display the time. By itself, the 
        :py:class:`MediaControl` is independent of a media node. The controlling
        application is responsible for keeping track of media node and 
        :py:class:`MediaControl` state and syncing the two.

        **Messages:**
        
            .. py:method:: PLAY_CLICKED()

            Emitted when the play/pause toggle is switched to play.

            .. py:method:: PAUSE_CLICKED()

            Emitted when the play/pause toggle is switched to pause.

            .. py:method:: SEEK_PRESSED()

            Emitted when the user starts dragging the seek thumb.

            .. py:method:: SEEK_MOTION(curTime)

            Emitted when the user moves the seek thumb.

            .. py:method:: SEEK_RELEASED()

            Emitted when the user releases the seek thumb.

        .. py:attribute:: duration

        Duration of the medium in milliseconds. 

        .. py:attribute:: time

        Current media time in milliseconds.

        .. py:method:: play()

        Switches to play mode by toggling the button.

        .. py:method:: pause()

        Switches to pause mode by toggling the button.


    .. autoclass:: Orientation()

        .. py:data:: HORIZONTAL

        .. py:data:: VERTICAL


    .. autoclass:: SwitchNode([nodeMap=None, visibleid=None])

        A :py:class:`DivNode` that keeps a map of child nodes and shows only one of 
        the map members at any time. 

        :param map nodeMap:

            A map :py:const:`id->node` that contains the nodes to switch between. 
        
        .. py:method:: setNodeMap(nodeMap)

            Can be used to set the :py:attr:`nodeMap` after construction if no node map
            was set before.

        .. py:attribute:: visibleid

            The id of the visible child node.


    .. autoclass:: TextButton(text, [skinObj=skin.Skin.default])

        A :py:class:`Button` that is created using the given :py:class:`Skin` and a text.

        .. py:attribute:: text 

            The string displayed on the button.


    .. autoclass:: TimeSlider()

        Works like a :py:class:`ProgressBar` with an additional slider thumb.

    .. autoclass:: ToggleButton(uncheckedUpNode, uncheckedDownNode, checkedUpNode, checkedDownNode, [uncheckedDisabledNode=None, checkedDisabledNode=None, activeAreaNode=None, fatFingerEnlarge=False, enabled=True, checked=False])

        A button that can be used to toggle between checked and unchecked states. 
        Classical GUI checkboxes are an example of this kind of button.
        
        A :py:class:`ToggleButton` has a total of six visual states. In addition to the
        distinction between checked and unchecked, a button can be enabled or disabled.
        Buttons also change their appearance as soon as they are touched, leading to two
        further states. For each visual state, a node is passed as constructor parameter.
        The constructor attaches the nodes to the :py:class:`ToggleButton`.

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

            If this parameter is set to :py:const:`True`, the button generates its own 
            internal :py:attr:`activeAreaNode` that is at least 20x20mm large. 
            :py:attr:`fatFingerEnlarge` is incompatible with a custom 
            :py:attr:`activeAreaNode`.

        :param bool checked:

            If this parameter is set to :py:const:`True`, the button starts in the checked
            state.

        :param bool enabled:

            If this parameter is set to :py:const:`True`, the button starts in the 
            disabled state.

        **Messages:**

            To get these messages, call :py:meth:`Publisher.subscribe`.

            .. py:method:: Button.PRESSED()

                Called when a tap on the button is initiated.

            .. py:method:: Button.RELEASED()

                Called when a tap on the button ends. Emitted for both successful and
                aborted taps.

            .. py:method:: Button.TOGGLED()

                Called when the button changes from unchecked to checked or vice-versa.


        .. py:attribute:: checked

            The state of the toggle.

        .. py:attribute:: enabled

            Determines whether the button accepts input.


    .. autoclass:: VStretchNode(endsExtent, [src=None, minExtent=-1]) 

        A node that stretches its graphics to fill the size given vertically. It is used
        as base component for scrollbars. The base
        bitmap is split into three parts: top, center and bottom. The top and bottom parts
        are :py:attr:`endsExtent` wide and generated from the corresponding parts of the 
        source bitmap. The center part is generated from a one pixel high slice of the 
        source bitmap and stretched to fill the space left between the top and bottom
        parts.

        :param int endsExtent:

            Width of the top and bottom bitmaps in pixels.

        :param src:

            Either the name of a bitmap file or a :py:class:`Bitmap` object. Used to
            generate the graphics used.

        :param int minExtent:

            Minimum vertical size. The default of :py:const:`-1` uses 
            :py:const:`2*endsExtent+1` as minimum.

