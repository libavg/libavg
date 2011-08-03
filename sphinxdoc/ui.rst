User Interface Classes
======================

The namespace libavg.ui contains python modules that expose higher-level user interface
functionality

.. automodule:: libavg.ui
    :no-members:

    .. autoclass:: DragRecognizer(node, [eventSource=avg.TOUCH | avg.MOUSE, startHandler=None, moveHandler=None, upHandler=None, stopHandler=None, initialEvent=None, friction=-1])

        A :py:class:`DragRecognizer` attaches itself to a node's cursor events and 
        delivers higher-level callbacks that can be used to implement dragging or 
        drag-like functionality.

        :py:class:`DragRecognizer` supports inertia after the node is released.
        
        :param avg.Node node: The node to attach to.

        :param eventSource: 
            
            One of the standard event sources (:py:const:`TRACK`, :py:const:`TOUCH` 
            etc.).

        :param initialEvent:

            A cursordown event to pass to the recognizer immediately.

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
    
    
    .. autoclass:: Mat3x3([row0=(1,0,0), row1=(0,1,0), row2=(0,0,1)])

        Matrix class for homogenous 2d transforms. This is a temporary class needed by
        :py:class:`TransformRecognizer`. It will be removed again as soon as a generic 
        matrix class is available.

        .. py:method:: applyMat(m) -> Mat3x3:

            Applies the matrix to another matrix and returns the result.

        .. py:method:: applyVec(v) -> Vec3:

            Applies the matrix to a 3-component vector and returns the result.

        .. py:method:: det() -> float:

            Returns the determinant of the matrix.

        .. py:method:: inverse() -> Mat3x3:

            Returns the inverse of the matrix.

        .. py:method:: scalarMult(s) -> Mat3x3:

            Returns the matrix multiplied by a scalar.

        .. py:method:: setNodeTransform(node):

            Sets a node's :py:attr:`pos`, :py:attr:`angle` and :py:attr:`size` 
            attributes to correspond to the matrix. Assumes :samp:`pivot=0`.

        .. py:classmethod:: fromNode(node) -> Mat3x3:

            Returns the transform matrix of a node computed from its :py:attr:`pos`,
            :py:attr:`angle` and :py:attr:`size` attributes. Assumes :samp:`pivot=0`.

        .. py:classmethod:: rotate(a) -> Mat3x3:

            Factory method that creates a rotation matrix from an angle.

        .. py:classmethod:: scale(s) -> Mat3x3:

            Factory method that creates a nonuniform scale matrix from a 2-component 
            vector.

        .. py:classmethod:: translate(t) -> Mat3x3:

            Factory method that creates a translation matrix from a translation vector.


    .. autoclass:: Recognizer(node, eventSource, maxContacts, initialEvent)

        Base class for gesture recognizers that attach to a node's cursor events and 
        emit higher-level events.

        :param Node node: Node to attach to.

        :param eventSource: 
        
            One of the standard event sources (:py:const:`TRACK`, :py:const:`TOUCH` 
            etc.).

        :param maxContacts:

            The maximum number of contacts that the recognizer should handle. 
            :py:const:`None` if there is no maximum.

        :param initialEvent:

            A cursordown event to pass to the recognizer immediately.

        .. py:method:: enable(isEnabled)

            Enables or disables the :py:class:`Recognizer`.

    .. autoclass:: TapRecognizer(node, [eventSource=avg.TOUCH | avg.MOUSE, startHandler=None, tapHandler=None, failHandler=None, initialEvent=None])

        A :py:class:`TapRecognizer` attaches to a node's cursor events and sends 
        higher-level events when a tap (a short touch without a large change of the 
        cursor position) occurs at this position.

        :param avg.Node node: The node to attach to.

        :param eventSource: 
            
            One of the standard event sources (:py:const:`TRACK`, :py:const:`TOUCH` 
            etc.).

        :param initialEvent:

            A cursordown event to pass to the recognizer immediately.
        
        The callbacks are:

            .. py:method:: startHandler():

                Called when a possible tap begins. 
                
            .. py:method:: tapHandler():

                Called when a tap is recognized.

            .. py:method:: failHandler(event):

                Called if the touch moves too far from the initial position to be a tap.

    .. autoclass:: TransformRecognizer(node, [eventSource=avg.TOUCH | avg.MOUSE, startHandler=None, moveHandler=None, upHandler=None, stopHandler=None, initialEvent=None, friction=-1])

        A :py:class:`TransformRecognizer` is used to support drag/zoom/rotate 
        functionality. From any number of touches on a node, it calculates an aggregate
        transform that can be used to change the position, size and angle of a node. A
        usage example can be found under :samp:`src/samples/mttransform.py`.

        :param avg.Node node: The node to attach to.

        :param eventSource: 
            
            One of the standard event sources (:py:const:`TRACK`, :py:const:`TOUCH` 
            etc.).

        :param initialEvent:

            A cursordown event to pass to the recognizer immediately.
       
        :param friction:

            Currently not implemented.

        The callbacks are:

            .. py:method:: startHandler():

                Called when a transform begins. 
                
            .. py:method:: moveHandler(transform):

                Called whenever the transform changes.

                :param Mat3x3 transform:
                
                    The current transformation as a homogenous matrix.

            .. py:method:: upHandler(transform):

                Called when the last touch is released.

                :param Mat3x3 transform:
                
                    The current transformation as a homogenous matrix.

            .. py:method:: stopHandler(transform):

                Not implemented.
