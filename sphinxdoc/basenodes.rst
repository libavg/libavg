Base Node Classes
=================

This section describes the base classes for all node classes that libavg provides. 

.. automodule:: libavg.avg
    :no-members:

    .. inheritance-diagram:: Node
        :parts: 1

    To be rendered, a :py:class:`Node` must be part of a scene graph. Scene graphs are 
    trees of :py:class:`Node` objects associated with a :py:class:`Canvas`. A
    :py:class:`CanvasNode` is at the root of each scene graph. Scene graphs are pure tree
    structures, so each :py:class:`Node` only has one parent node. Nodes that are not 
    linked to a canvas are not rendered. Any media that these nodes need are loaded from 
    disk, however.

    libavg :py:class:`Node` classes make heavy use of inheritance. Concepts like 
    :py:attr:`id`, :py:attr:`position` and :py:attr:`opacity` are defined in base classes
    and can be used in any of the subclasses. 
    
    .. note::

        To reduce redundancy in the reference,
        inherited methods and attributes are not mentioned in the derived class 
        documentation - follow the link to the base class to access them. This also 
        applies to constructor parameters: When constructing an object of a derived class,
        constructor parameters of the base classes are also accepted.
        
    There are several ways of constructing a
    node. The reference documentation follows the python constructor syntax. The
    parameters remain the same in all syntactic variations, however. The options for 
    construction are as follows:

        **Use the standard python constructor**:
            Nodes can be created using a standard python constructor. As an example::

                node = ImageNode(id="background", href="sunset.png", pos=(0,0), 
                        parent=rootNode)

            Parameters to a node constructor are always named parameters. Nodes never have
            positional constructor parameters.

        **Use** :py:meth:`Player.createNode`:
            There are two ways to create a node using createNode::

                node = player.createNode("image", 
                        {"id":"background", "href":"sunset.png", "pos":(0,0), 
                         "parent":rootNode})

            and::

                node = player.createNode(
                        """<image id="background" href="sunset.png" 
                                      pos="(0,0)"/>""")

            Using the second option, complete trees of nodes can be constructed in one
            statement.

        **Load it from an avg file**:
            Complete scene graphs for onscreen display can be loaded from disk using 
            :py:meth:`Player.loadFile`::

                root = player.loadFile("scene.avg")

        **Create a complete scene graph using inline xml**:
            :py:meth:`Player.loadString` allows using an avg-formatted xml string to 
            create a scene graph of nodes::

                root = player.loadString("""
                        <avg size="(800,600)">
                            <image id="background" href="sunset.png" 
                                      pos="(0,0)"/>
                        </avg>
                    """)

        The methods :py:meth:`Player.loadFile` and :py:meth:`Player.loadString` create 
        onscreen scene graphs. :py:meth:`Player.loadCanvasFile` and 
        :py:meth:`Player.loadCanvasString` are the equivalent methods for offscreen 
        canvases.

    .. autoclass:: Node([id: string="", parent: DivNode=None, active=True, sensitive=True, opacity=1.0, style=None])

        Base class for everything that can be put into an avg tree. This is an abstract
        class.

        **Messages:**

            All cursor and hover messages are emitted only if the cursor is above the
            :py:class:`Node` and
            :py:attr:`active` as well as :py:attr:`sensitive` are True. The message
            parameters are of type :py:class:`CursorEvent`. The CURSOR messages are 
            emitted for mouse and touch events. The HOVER events are emitted for touch 
            devices which can sense hands approaching the surface before the actual touch.
            TANGIBLE events are emitted when a touch by a physical object is
            detected and have parameters of class :py:class:`TangibleEvent`. Tangibles are
            usually marker-based and have unique ids.

            To get these messages, call :py:meth:`Publisher.subscribe`.

            .. py:method:: CURSOR_DOWN(cursorevent)
            
                Emitted whenever a mouse button is pressed or a new touch is registered.

            .. py:method:: CURSOR_MOTION(cursorevent)
            
                Emitted whenever a mouse or a touch moves.

            .. py:method:: CURSOR_UP(cursorevent)
            
                Emitted whenever a mouse button is released or a touch leaves the surface.

            .. py:method:: CURSOR_OVER(cursorevent)
            
                Emitted whenever a mouse or a touch enters the :py:class:`Node`'s area.

            .. py:method:: CURSOR_OUT(cursorevent)
            
                Emitted whenever a mouse or a touch leaves the :py:class:`Node`'s area.

            .. py:method:: HOVER_DOWN(cursorevent)
            
                Emitted whenever a new hover cursor is registered.

            .. py:method:: HOVER_MOTION(cursorevent)
            
                Emitted whenever a hover cursor moves.

            .. py:method:: HOVER_UP(cursorevent)
            
                Emitted whenever a hover cursor disappears.

            .. py:method:: HOVER_OVER(cursorevent)
            
                Emitted whenever a hover cursor enters the :py:class:`Node`'s area.

            .. py:method:: HOVER_OUT(cursorevent)
            
                Emitted whenever a hover cursor leaves the :py:class:`Node`'s area.

            .. py:method:: TANGIBLE_DOWN(tangibleevent)
            
                Emitted whenever a new tangible cursor is registered.

            .. py:method:: TANGIBLE_MOTION(tangibleevent)
            
                Emitted whenever a tangible cursor moves.

            .. py:method:: TANGIBLE_UP(tangibleevent)
            
                Emitted whenever a tangible cursor disappears.

            .. py:method:: TANGIBLE_OVER(tangibleevent)
            
                Emitted whenever a tangible cursor enters the :py:class:`Node`'s area.

            .. py:method:: TANGIBLE_OUT(tangibleevent)
            
                Emitted whenever a tangible cursor leaves the :py:class:`Node`'s area.

            .. py:method:: SIZE_CHANGED(newSize)
            
                Emitted whenever the size of the node changes. This includes any python
                calls that change the size. In addition, image loading (for 
                :py:class:`ImageNode`), opening of video files (for 
                :py:class:`VideoNode`) and changes in the text displayed (in the case of
                :py:class:`WordsNode`) can trigger :py:meth:`SIZE_CHANGED` messages. Note
                that changing the size of a node inside a :py:meth:`SIZE_CHANGED` handler
                will lead to an additional recursive invocation of 
                :py:meth:`SIZE_CHANGED`.
                
                :py:class:`RectNode` and all classes derived from :py:class:`AreaNode`
                support this message.


        .. py:attribute:: id

            A unique identifier that can be used to reference the node, for instance using
            :py:meth:`Player.getElementByID`. Read-only.

        .. py:attribute:: parent

            A :py:class:`DivNode` that the node will become a child of. When used as a
            constructor parameter, this is equivalent to calling 
            :py:meth:`DivNode.appendChild` directly after construction. Read-only.

        .. py:attribute:: active

            If this attribute is true, the node behaves as usual. If not, it
            is neither drawn nor does it react to events.

        .. py:attribute:: opacity

            A measure of the node's transparency. 0.0 is completely
            transparent, 1.0 is completely opaque. Opacity is relative to
            the parent node's opacity.

        .. py:attribute:: sensitive

            A node only reacts to events if sensitive is true.

        .. py:method:: connectEventHandler(type, source, pyobj, pyfunc)

            .. deprecated:: 1.8
                Use the message interface instead.

            Sets a callback function that is invoked whenever an event of the
            specified type from the specified source occurs. Unlike 
            :py:meth:`setEventHandler`, this method allows several handlers for one 
            type/source-combination. To remove a handler installed using 
            :py:meth:`connectEventHandler`, call :py:meth:`disconnectEventHandler`.

            :param type:
            
                One of the event types :py:const:`KEYUP`, :py:const:`KEYDOWN`, 
                :py:const:`CURSORMOTION`, :py:const:`CURSORUP`, :py:const:`CURSORDOWN`, 
                :py:const:`CURSOROVER` or :py:const:`CURSOROUT`.

            :param source:

                :py:const:`MOUSE` for mouse events, :py:const:`TOUCH` for multitouch touch
                events, :py:const:`TRACK` for multitouch track events or other tracking,
                :py:const:`NONE` for keyboard events. Sources can be or'ed together to 
                set a handler for several sources at once.

            :param pyobj:

                The python object that hosts the callback. This parameter is only needed
                so that :py:meth:`disconnectEventHandler` can be called to remove all
                handlers hosted by one object in one call.

            :param pyfunc:

                The python callable to invoke. This callable must take the event to 
                process as a parameter. In contrast to callbacks set up using 
                :py:meth:`setEventHandler`, it should not return anything. If 
                :py:meth:`connectEventHandler` is used, all events bubble up the tree.
                pyfunc may not be :py:const:`None`.

        .. py:method:: disconnectEventHandler(pyobj, [pyfunc])

            .. deprecated:: 1.8
                Use the message interface instead.

            Removes one or more event handlers from the node's table of event handlers.
            If several event handlers conform to the parameters given, all are removed.
            It is an error if no matching event handler exists.

            :param pyobj:

                The python object that hosts the event handler.

            :param pyfunc:

                The python callable that should not be called anymore. If pyfunc is
                absent, all callbacks hosted by :py:attr:`pyobj` are removed.

        .. py:method:: getAbsPos(relpos) -> Point2D

            Transforms a position in coordinates relative to the node to a
            position in window coordinates.

        .. py:method:: getElementByPos(pos) -> Node

            Returns the topmost child node that is at the position given. :py:attr:`pos`
            is in coordinates relative to the called node. The algorithm used
            is the same as the cursor hit test algorithm used for events.

        .. py:method:: getParent() -> Node

            .. deprecated:: 1.8
                Use :attr:`parent` instead.

            Returns the container (:py:class:`AVGNode` or :py:class:`DivNode`) the node
            is in. For the root node (or if the node is not connected), returns 
            :py:const:`None`.

        .. py:method:: getRelPos(abspos) -> Point2D

            Transforms a position in window coordinates to a position
            in coordinates relative to the node.

        .. py:method:: registerInstance(self, parent)

            Needs to be called when deriving from a Node class in python in the derived
            classes :py:meth:`__init__` method.

        .. py:method:: releaseEventCapture([cursorid])

            Restores normal cursor event handling after a call to 
            :py:meth:`setEventCapture()`. :py:attr:`cursorid` is the id of the
            cursor to release. If :py:attr:`cursorid` is not given, the mouse cursor is
            used.

        .. py:method:: setEventCapture([cursorid])

            Sets up event capturing so that cursor events are sent to this node
            regardless of the cursor position. cursorid is optional; if left out,
            the mouse cursor is captured. If not, events from a specific tracker
            cursor are captured. The event propagates to the capturing node's
            parent normally. This function is useful for the
            implementation of user interface elements such as scroll bars. Only one
            node can capture a cursor at any one time. Normal operation can
            be restored by calling :py:meth:`releaseEventCapture()`.
        
        .. py:method:: setEventHandler(type, source, pyfunc)

            .. deprecated:: 1.7
                Use the message interface instead.

            Sets a callback function that is invoked whenever an event of the
            specified type from the specified source occurs. This method removes all 
            other event handlers from this type/source-combination. 

            :param type:
            
                One of the event types :py:const:`KEYUP`, :py:const:`KEYDOWN`, 
                :py:const:`CURSORMOTION`, :py:const:`CURSORUP`, :py:const:`CURSORDOWN`, 
                :py:const:`CURSOROVER` or :py:const:`CURSOROUT`.

            :param source:

                :py:const:`MOUSE` for mouse events, :py:const:`TOUCH` for multitouch
                touch events, :py:const:`TRACK` for multitouch track events or other 
                tracking, :py:const:`NONE` for keyboard events. Sources can be or'ed 
                together to set a handler for several sources at once.

            :param pyfunc:

                The python callable to invoke. This callable must take the event to 
                process as a parameter. If pyfunc returns :py:const:`None` or 
                :py:const:`False`, the event bubbles up the node tree. If it is 
                :py:const:`True`, bubbling is suppressed.

                If pyfunc is :py:const:`None`, the previous handler is removed.

        .. py:method:: unlink([kill=False])

            Removes a node from its parent container and optionally deletes all resources
            the node holds. In the default case, :py:meth:`unlink` is equivalent to
            :samp:`node.getParent().removeChild(node.getParent().indexOf(node))`, 
            except that if the node has no parent, unlink does nothing. Also in the 
            default case, textures are moved back to the CPU and event handlers are 
            preserved.

            If :samp:`kill=True`, textures are not moved back. Event handlers for events
            routed to this node are reset, all textures are deleted and the href is reset
            to empty in this case, saving some time and making sure there are no 
            references to the node left on the libavg side. :py:attr:`kill` should always
            be set to :py:const:`True` if the node will not be used after the unlink.
    

    .. autoclass:: Publisher()

        libavg supports event handling and callbacks through a publish/subscribe 
        interface. :py:class:`Publisher` is the base class for all classes that
        send messages. Derived classes can send messages of arbitrary types. The base
        class takes care of managing a list of subscribers for each message type and
        sending the message to each subscriber.

        Many libavg classes, including :py:class:`Node`, :py:class:`Player`, 
        :py:class:`Contact` and the :py:class:`Recognizer` classes derive from publisher.
        In addition, it is possible to derive from :py:class:`Publisher` in client code
        by calling the methods in the protected interface.

        .. py:method:: subscribe(messageID, callable) -> int

            Registers a subscriber for the given :py:attr:`messageID`. The 
            :py:attr:`callable` for all subscribers is invoked whenever the publisher 
            sends out the message with this ID.
            
            :py:meth:`subscribe` returns a :py:attr:`subscriberID` that can be used to
            unsubscribe if this becomes necessary. The subscription is also terminated if 
            either the publisher or the subscriber is deleted. The :py:class:`Publisher` 
            class works with weak references to subscribers when possible, so this should 
            happen automatically in most cases. The exception is when :py:attr:`callable` 
            is an anonymous function (a lambda expression). In this case, the
            :py:class:`Publisher` needs to hold a reference to the callable to keep it 
            from being deleted immediately and :py:meth:`unsubscribe` needs to be called 
            manually.

            If the :py:attr:`callable` parameter is :py:const:`None`, the call is ignored
            and -1 is returned.

        .. py:method:: unsubscribe(messageID, subscriberID)
                       unsubscribe(messageID, callable)

            Removes a subscriber from the list of subscribers for :py:attr:`messageID`. 
            The subscriber is either determined by the :py:attr:`subscriberID` returned
            from :py:meth:`subscribe` or by the :py:attr:`callable` attached to the
            subscription.

        **Protected Interface:**

            To be called from derived classes.

            .. py:method:: publish(messageID)

                Registers a :py:attr:`messageID` so that interested parties can subscribe
                to this message.

            .. py:method:: notifySubscribers(messageID, argsList)

                Invokes all callables registered for this :py:attr:`messageID` using the
                list of args passed. Subscribers are called synchronously; the order of
                invokation is undefined.

