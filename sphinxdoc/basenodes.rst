Base Node Classes
=================

.. automodule:: libavg.avg
    :no-members:

    .. inheritance-diagram:: Node VisibleNode
        :parts: 1

    .. autoclass:: Node([id: string])

        Base class for everything that can be put into an avg tree.

        .. py:attribute:: id

            A unique identifier that can be used to reference the node, for instance using
            :py:meth:`Player.getElementByID`. Read-only.

    .. autoclass:: VisibleNode([oncursormove, oncursorup, uncursordown, oncursorover, oncursorout, active=True, sensitive=True, opacity=1.0, parent])

        Base class for all elements in the avg tree that have a visual representation.
        All nodes except those derived from :py:class:`FXNode` are VisibleNodes.

            :param string oncursormove:

                Name of python function to call when a cursor moves.

                .. deprecated:: 1.5
                    Use :func:`setEventHandler()` instead.

            :param string oncursorup:

                Name of python function to call when an up event occurs.

                .. deprecated:: 1.5
                    Use :func:`setEventHandler()` instead.

            :param string oncursordown:

                Name of python function to call when a down event occurs.

                .. deprecated:: 1.5
                    Use :func:`setEventHandler()` instead.

            :param string oncursorover:

                Name of python function to call when a cursor enters the node.

                .. deprecated:: 1.5
                    Use :func:`setEventHandler()` instead.

            :param string oncursorout:

                Name of python function to call when a cursor leaves the node.

                .. deprecated:: 1.5
                    Use :func:`setEventHandler()` instead.

            :param DivNode parent:

                A :py:class:`DivNode` that the newly constructed Node should be appended
                to.

        .. py:attribute:: active

            If this attribute is true, the node behaves as usual. If not, it
            is neither drawn nor does it react to events.

        .. py:attribute:: opacity

            A measure of the node's transparency. 0.0 is completely
            transparent, 1.0 is completely opaque. Opacity is relative to
            the parent node's opacity.

        .. py:attribute:: sensitive

            A node only reacts to events if sensitive is true.

        .. py:method:: getAbsPos(relpos) -> Point2D

            Transforms a position in coordinates relative to the node to a
            position in window coordinates.

        .. py:method:: getElementByPos(pos) -> Node

            Returns the topmost child node that is at the position given. :py:attr:`pos`
            is in coordinates relative to the called node. The algorithm used
            is the same as the cursor hit test algorithm used for events.

        .. py:method:: getParent() -> Node

            Returns the container (:py:class:`AVGNode` or :py:class:`DivNode`) the node
            is in. For the root node, returns None.

        .. py:method:: getRelPos(abspos) -> Point2D

            Transforms a position in window coordinates to a position
            in coordinates relative to the node.

        .. py:method:: releaseEventCapture(cursorid=-1)

            Restores normal cursor event handling after a call to 
            :py:func:`setEventCapture()`. :py:attr:`cursorid` is the id of the
            cursor to release. If :py:attr:`cursorid` is not given, the mouse cursor is
            used.

        .. py:method:: setEventCapture(cursorid=-1)

            Sets up event capturing so that cursor events are sent to this node
            regardless of the cursor position. cursorid is optional; if left out,
            the mouse cursor is captured. If not, events from a specific tracker
            cursor are captured. The event propagates to the capturing node's
            parent normally. This function is useful for the
            implementation of user interface elements such as scroll bars. Only one
            node can capture a cursor at any one time. Normal operation can
            be restored by calling :py:func:`releaseEventCapture()`.
        
        .. py:method:: setEventHandler(type, source, pyfunc)

            Sets a callback function that is invoked whenever an event of the
            specified type from the specified source occurs. This function is
            similar to the event handler node attributes (e.g. oncursordown).
            It is more specific since it takes the event source as a parameter
            and allows the use of any python callable as callback function.
            
            :param type:
            
                One of the event types :py:const:`KEYUP`, :py:const:`KEYDOWN`, 
                :py:const:`CURSORMOTION`, :py:const:`CURSORUP`, :py:const:`CURSORDOWN`, 
                :py:const:`CURSOROVER`, :py:const:`CURSOROUT`, :py:const:`RESIZE` or 
                :py:const:`QUIT`.

            :param source:

                :py:const:`MOUSE` for mouse events, :py:const:`TOUCH` for multitouch touch
                events, :py:const:`TRACK` for multitouch track events or other tracking,
                :py:const:`NONE` for keyboard events. Sources can be or'ed together to 
                set a handler for several sources at once.

            :param pyfunc:

                The python callable to invoke.

        .. py:method:: unlink(kill)

            Removes a node from it's parent container. Equivalent to
            :samp:`node.getParent().removeChild(node.getParent().indexOf(node))`, 
            except that if the node has no parent, unlink does nothing. Normally, unlink
            moves the node's textures back to the CPU and preserves event handlers.
            If :samp:`kill=True`, this step is skipped. Event handlers are reset, all
            textures are deleted and the href is reset to empty in this case,
            saving some time and making sure there are no references to the node
            left on the libavg side. :py:attr:`kill` should always be set to 
            :keyword:`True` if the node will not be used after the unlink.

