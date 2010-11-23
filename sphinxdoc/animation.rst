Animation
=========

.. automodule:: libavg.avg
    :no-members:

    .. autoclass:: Anim

        Base class for all animations.

        .. py:method:: setStartCallback(pyfunc)

            Sets a python callable to be invoked when the animation starts. 
            Corresponds to the constructor parameter startCallback.

        .. py:method:: setStopCallback(pyfunc)

            Sets a python callable to invoke when the animation has 
            finished running. Corresponds to the constructor parameter stopCallback.

        .. py:method:: abort

            Stops the animation.

        .. py:method:: start

            Starts the animation. 

            :param bKeepAttr: 
            
                If this parameter is set to :keyword:`True`, the animation doesn't 
                set the attribute value when starting. Instead, it calculates a virtual 
                start time from the current attribute value and proceeds from there.

        .. py:method:: isRunning

                Returns :keyword:`True` if the animation is currently executing.

    .. autoclass:: AnimState

        One state of a StateAnim.

        .. py:method:: __init__(name, anim, nextName="")

            :param name: The name of the state. Used in StateAnim.set/getState().

            :param anim: The child animation to run when this state is active.

            :param nextName: The name of the state to enter when this state is done.

    .. autoclass:: AttrAnim

    .. autoclass:: ContinuousAnim

        Class that animates an attribute of a libavg node continuously and 
        linearly. The animation will not stop until the :py:meth:`abort()` method is 
        called. 

        A possible use case is the continuous rotation of an object.

        .. py:method:: __init__(node, attrName, startValue, speed, [useInt=False, startCallback=None, stopCallback=None])

            :param node: The libavg node object to animate.

            :param attrName: 
            
                The name of the attribute to change.

            :param startValue: Initial value of the attribute.

            :param speed: Attribute change per second.

            :param useInt: 
            
                If :keyword:`True`, the attribute is always set to an integer value.

            :param startCallback: Python callable to invoke when the animation starts.

            :param stopCallback: 
            
                Python callable to invoke when the animation has 
                finished running, either because abort was called or because 
                another animation for the same attribute was started.

    .. autoclass:: EaseInOutAnim

        Class that animates an attribute of a libavg node. The animation proceeds
        in three phases: ease-in, linear and ease-out. Start and end speed are
        zero. Ease-in and ease-out phases have the shape of one quadrant of the
        sine curve.

        .. py:method:: __init__(node, attrName, duration, startValue, endValue, easeInDuration, easeOutDuration, [useInt=False, startCallback=None, stopCallback=None])

            :param node: The libavg node object to animate.

            :param attrName:

                The name of the attribute to change.

            :param duration: The length of the animation in milliseconds.

            :param startValue: Initial value of the attribute.

            :param endValue: Value of the attribute after duration has elapsed.

            :param easeInDuration: The duration of the ease-in phase in milliseconds.

            :param easeOutDuration: 
            
                The duration of the ease-out phase in milliseconds.

            :param useInt: 
            
                If :keyword:`True`, the attribute is always set to an integer value.

            :param startCallback: Python callable to invoke when the animation starts.

            :param stopCallback: 
            
                Python callable to invoke when the animation has 
                finished running, either because it's run the allotted time, because
                abort was called or because another animation for the same
                attribute was started.

    .. autoclass:: LinearAnim

        Class that animates an attribute of a libavg node by interpolating
        linearly between start and end values.

        .. py:method:: __init__(node, attrName, duration, startValue, endValue, [useInt=False, startCallback=None, stopCallback=None])

            :param node: The libavg node object to animate.

            :param attrName: 
            
                The name of the attribute to change.

            :param duration: The length of the animation in milliseconds.

            :param startValue: Initial value of the attribute.

            :param endValue: Value of the attribute after duration has elapsed.

            :param useInt: 

                If :keyword:`True`, the attribute is always set to an integer value.

            :param startCallback: Python callable to invoke when the animation starts.

            :param stopCallback:
            
                Python callable to invoke when the animation has 
                finished running, either because it's run the allotted time, because
                abort was called or because another animation for the same
                attribute was started.
        
    .. autoclass:: ParallelAnim

            Animation that executes several child animations at the same time. The 
            duration of the ParallelAnim is the maximum of the child's durations or 
            maxAge, whatever is shorter.

        .. py:method:: __init__(anims, [startCallback, stopCallback, maxAge])

            :param anims: A list of child animations.

            :param startCallback: Python callable to invoke when the animation starts.

            :param stopCallback: 
                
                Python callable to invoke when the animation has 
                finished running, either because it's run the allotted time or because
                abort was called.

            :param maxAge: The maximum duration of the animation in milliseconds.

        .. py:method:: start(keepAttr)

            Starts the animation by calling :py:meth:`start` for each of the child 
            animations.

            :param bKeepAttr: 
            
                This parameter is passed to the child animations.

    .. autoclass:: SimpleAnim

        Base class for animations that change libavg node attributes by
        interpolating over a set amount of time. If :py:meth:`Anim.abort()` isn't needed,
        there is no need to hold on to the animation object after calling 
        :py:meth:`Anim.start` - it will exist exactly as long as the animation lasts and
        then disappear.

        The animation framework makes sure that only one animation per attribute
        of a node runs at any given time. If a second one is started, the first
        one is aborted.

    .. autoclass:: StateAnim

        Animation that executes one of several child animations depending on it's 
        current state. The state can be None, in which case no animation is 
        executed. None is the initial state. Note that changing the state of an 
        animation during a start or stop callback of a child animation is not 
        possible. An attempt to do so is silently ignored.

        .. py:method:: __init__(states)

            :param states: A list of AnimState objects.

        .. py:method:: setState
        
        .. py:method:: getState

        .. py:method:: setDebug(debug)

            Setting this to :keyword:`True` causes all state changes to be printed on 
            the console.

    .. autoclass:: WaitAnim

        Animation that simply does nothing for a specified duration. Useful 
        in the context of StateAnims.

        .. py:method:: __init__([duration=-1, startCallback, stopCallback])

            :param duration: The length of the animation in milliseconds.
            
            :param startCallback: Python callable to invoke when the animation starts.

            :param stopCallback: 
            
                Python callable to invoke when the animation has 
                finished running, either because it's run the allotted time or because
                abort was called.

        .. py:method:: start

    .. autofunction:: fadeIn(node, duration, [max=1.0, stopCallback])

        Fades the opacity of a node.

        :param node: The node to fade.
        :param duration: Length of the fade in milliseconds.
        :param max: The opacity of the node at the end of the fade.
        :param stopCallback: Function to call when the fade is over.

    .. autofunction:: fadeOut(node, duration, [stopCallback])

        Fades the opacity of a node to zero.

        :param node: The node to fade.
        :param duration: Length of the fade in milliseconds.
        :param stopCallback: Function to call when the fade is over.
    
    .. autofunction:: getNumRunningAnims() -> int

         Returns the total number of running attribute-based animations (this
         includes LinearAnim, EaseInOutAnim and ContinuousAnim). Useful for
         debugging memory leaks.
