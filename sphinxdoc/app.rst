app module
==========

.. automodule:: libavg.app

    .. note::

        The app package is experimental. Functionality and interface are still in flux and
        subject to change.

    .. autoclass:: App

        This class handles global application affairs. Among these are setting up a root
        node along with window size (and possibly screen resolution) and screen
        rotation, and creating an environment for the user's :py:class:`MainDiv`. 
        Global user interface items such as a debug panel and keyboard manager are also set 
        up. Additionally, :py:class:`App` handles configuration settings and command line
        argument support.
        
        :py:class:`App` usually does not need to be subclassed. Instead, subclass 
        :py:class:`MainDiv` and pass an instance of the derived class to :py:meth:`run()`.
        
        .. py:method:: run(mainDiv, **kargs)

            Starts the application using the provided :py:attr:`mainDiv` (an instance of
            :py:class:`MainDiv`).
            
            The optional additional kargs are used to set default settings - see 
            :py:class:`settings.Settings`.

        .. py:method:: stop()

            Stops the player.

        .. py:method:: onBeforeLaunch()

            Called just before starting the main loop (:samp:`Player.play()`). Useful only
            for subclassing.

        .. py:method:: takeScreenshot(targetFolder='.')

            Takes a screenshot of what is currently visible on the screen. Normally
            bound to the keypress :kbd:`CTRL-p`.

        .. py:method:: dumpTextObjectCount()

            Dumps on the console the current number of initialized objects. Normally
            bound to the keypress :kbd:`CTRL-b`.

        .. py:attribute:: mainDiv

            The instance passed as first argument of the :py:meth:`run()` method.

        .. py:attribute:: appParent

            The :py:class:`DivNode` that is :py:attr:`mainDiv`'s parent. It also holds
            several internal nodes such as the overlay panels.

        .. py:attribute:: debugPanel

            An instance of debugpanel.DebugPanel.

        .. py:attribute:: overlayPanel

            DivNode that stands always on top of the MainDiv.

        .. py:attribute:: settings

            An instance of settings.Settings.

        .. py:attribute:: resolution

            Current target screen resolution.

        .. py:attribute:: windowSize

            Current target window size.

    .. autoclass:: MainDiv

        This abstract class must be subclassed to write a libavg application. It is the main
        application entry point and should be the parent node for all application-created
        nodes. All the public methods are empty and don't do anything if not overridden.

        .. py:attribute:: INIT_FUNC

            A string that represents the identifier of the method that will be called
            on the MainDiv instance upon initialization of the App. It defaults to onInit.

        .. py:attribute:: VERSION

            A version string. This is shown using the :option:`-v` or :option:`--version`
            command-line option.

        .. py:method:: onArgvParserCreated(parser)

            Called with an empty :py:class:`optparse.OptionParser` instance. Allows the
            application to add additional command line options. :py:class:`App` adds it's 
            own parameters as well. If this is overridden, :py:meth:`onArgvParsed()` 
            should probably be overridden as well.

        .. py:method:: onArgvParsed(options, args, parser)

            This method is called after command-line arguments have been parsed and should
            be used to retrieve any application-specific options. The arguments 
            :py:attr:`options` and :py:attr:`args` are the result of calling
            :samp:`options, args = parser.parse_args()`, where parser is an instance of 
            :py:class:`optparse.OptionParser` configured by calling 
            :py:meth:`onArgvParserCreated`.
            
        .. py:method:: onStartup()

            Called before libavg has been setup, just after the App().run() call.

        .. py:method:: onInit()

            Called by a libavg timer as soon as the main loop starts.
            Build the application node tree here.

        .. py:method:: onExit()

            Called after the main loop exits.
            Release resources and run cleanups here.

            .. note::

                onExit() doesn't get called if an exception is raised on the main thread.

        .. py:method:: onFrame(delta)

            Called at every frame. :py:attr:`delta` is the time that has passed since the 
            last call of :py:meth:`onFrame()` in milliseconds.


Keyboard handling
=================

.. automodule:: libavg.app.keyboardmanager
    :no-members:

    This module makes it possible to attach event handlers to individual keypresses. 
    Keys that are bound through the keyboard manager are also be shown in the debug panel
    via the keyboard bindings widget along with their help string. :py:class:`App` defines
    a range of keyboard bindings by default. Conceptually, all keys modified by :kbd:`CTRL`
    are reserved by :py:class:`App`.

    For all the binding methods, keystring can be a python string or a unicode object.
    TODO: expand the discussion regarding keystring vs SDL
    TODO: describe the modifiers

    .. py:method:: init()

        Called by :py:class:`App`. Should not be called by user programs.

    .. py:method:: bindKeyDown(keystring, handler, help, modifiers=avg.KEYMOD_NONE)

        Sets up a key handler so that :py:attr:`handler` is called whenever 
        :py:attr:`keystring` is pressed.

    .. py:method:: bindKeyUp(keystring, handler, help, modifiers=avg.KEYMOD_NONE)

        Sets up a key handler so that :py:attr:`handler` is called whenever 
        :py:attr:`keystring` is released.

    .. py:method:: unbindKeyDown(keystring, modifiers=avg.KEYMOD_NONE)

        Removes a previously defined key binding for a KEY_DOWN event.

    .. py:method:: unbindKeyDown(keystring, modifiers=avg.KEYMOD_NONE)

        Removes a previously defined key binding for a KEY_UP event.

    .. py:method:: unbindAll()

        Removes all the defined key bindings at once.

    .. py:method:: push()

        Pushes all the non-modified key bindings onto a stack and clears them all.
        Useful when the application flow branches to a state where a different key
        bindings set is needed. The bindings can be then restored with :py:meth:`pop()`.

    .. py:method:: pop()

        Companion to :py:meth:`push()`, restores the non-modified key bindings set
        previously pushed onto the stack via :py:meth:`push()`.

    .. py:method:: getCurrentBindings()

        Returns the currently assigned bindings as a list of keyboardmanager._KeyBindings
        named tuples.

    .. py:method:: enable()

        Companion to :py:meth:`disable()`, enables all handlers.

    .. py:method:: disable()

        Companion to :py:meth:`enable()`, disables all handlers.


Settings
========

.. automodule:: libavg.app.settings
    :no-members:

    The Setting class, normally instantiated and set up by App, is a collection
    of options. A group of core options, necessary for the App to properly set up
    the running conditions are added with their defaults.

    Options can be overridden on App().run() (as kargs) and by the command line
    interface.

    .. autoclass:: Settings(defaults=[])

        .. py:method:: get(key, convertFunc=lambda v: v)

            XXX

        .. py:method:: getJson(key)

            XXX

        .. py:method:: getPoint2D(key)

            XXX

        .. py:method:: getInt(key)

            XXX

        .. py:method:: getFloat(key)

            XXX

        .. py:method:: getBoolean(key)

            XXX

    .. autoclass:: Option(key, value, help=None)

        XXX


Debug panel
===========

.. automodule:: libavg.app.debugpanel
    :no-members:

    ..autoclass:: DebugPanel

        XXX


Flash messages
==============

.. automodule:: libavg.app.flashmessage
    :no-members:

    .. autoclass:: FlashMessage(text, timeout=DEFAULT_TIMEOUT, parent=None, isError=False, acknowledge=False)

        A FlashMessage is a brief, temporary or persistent notification to the user, under
        the form of a text line shown on top of the node hierarchy.

        The message is normally shown for a certain amount of time, then it disappears. It
        can also be defined as persistent: in such case the user has to acknowledge it by
        clicking on it to make it disappear.

        A FlashMessage can be used also to notify errors, shown with a different color and
        automatically pushing the message to the underlying logger.

        Multiple FlashMessages are assured to be shown in the order they get created.

        :param timeout: The time in milliseconds the message should persist on screen
                before it gets removed.
        
        :param parent: When specified, the parent node the message should be appending
                itself to.

        :param isError: A boolean flag to mark the message as error. Error-flagged
                messages are shown in a different color and are routed to the
                logger as well.

        :param acknowledge: A boolean flag to indicate whether the message has to be
                removing automatically (after timeout has elapsed) or acknowledged
                by the user (by clicking / touching on it).


Touch visualization
===================

.. automodule:: libavg.app.touchvisualization
    :no-members:

    .. autoclass:: TouchVisualizationOverlay(isDebug, visClass)

        XXX

