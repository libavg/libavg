User Interface Classes
======================

The namespace libavg.ui contains python modules that expose higher-level user interface
functionality

.. automodule:: libavg.ui
    :no-members:

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

            List of key definitions. Keys can be either character keys::

                [(<keycode>, <shift keycode>, <altgr keycode>), <pos>, <size>]

            or command keys::

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

        .. py:method:: reset():

            Resets any sticky keys (shift, altgr) to their default state.

        .. py:method:: setKeyHandler(self, downHandler, [upHandler]):

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
    
    
    .. autoclass:: DragRecognizer(node, [eventSource=avg.TOUCH | avg.MOUSE, startHandler=None, moveHandler=None, upHandler=None, stopHandler=None, friction=-1])

        A :py:class:`DragRecognizer` attaches itself to a node's cursor events and 
        delivers higher-level callbacks that can be used to implement dragging or 
        drag-like functionality.

        :py:class:`DragRecognizer` supports inertia after the node is released.
        
        :param avg.Node node: The node to attach to.

        :param eventSource: 
            
            One of the standard event sources (:py:const:`TRACK`, :py:const:`TOUCH` 
            etc.).

        :param float friction:

            If set, this parameter enables inertia processing. It describes how 
            quickly the drag comes to a stop after the cursor is released.

        The callbacks are:

            .. py:method:: startHandler(event):

                Called when a drag begins. 
                
                :param event: The corresponding cursor down event. 

            .. py:method:: moveHandler(event, offset):

                Called when the drag should cause a position change. This usually happens
                in response to a :py:const:`CURSORMOTION` event, but may also happen
                because of inertia.

                :param event: 
                
                    The corresponding cursor motion event. If there was no event, 
                    this parameter is :py:const:`None`.

                :param avg.Point2D offset: 
                
                    The current offset from the start of the drag in global coordinates.

            .. py:method:: upHandler(event, offset):

                Called when the cursor is released. If inertia is enabled, there may be 
                move events after the up event.

                :param event: 
                
                    The corresponding :py:const:`CURSORUP` event.

                :param avg.Point2D offset: 
                
                    The current offset from the start of the drag in global coordinates.

            .. py:method:: stopHandler():

                Called when movement stops. This is either directly after the up event
                or when inertia has run its course.

        .. py:method:: abortInertia():

            Causes inertia processing to end immediately.


    .. autoclass:: Recognizer(node, eventSource)

        Base class for gesture recognizers that attach to a node's cursor events and 
        emit higher-level events.

        :param Node node: Node to attach to.

        :param eventSource: 
        
            One of the standard event sources (:py:const:`TRACK`, :py:const:`TOUCH` 
            etc.).

        .. py:method:: enable(isEnabled)

            Enables or disables the :py:class:`Recognizer`.
