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
        Global user interface items such as a debug panel and keyboard manager are also 
        set up. Additionally, :py:class:`App` handles configuration settings and command
        line argument support.
        
        :py:class:`App` usually does not need to be subclassed. Instead, subclass 
        :py:class:`MainDiv` and pass an instance of the derived class to :py:meth:`run()`.
        
        .. py:attribute:: debugPanel

            An instance of debugpanel.DebugPanel.

        .. py:attribute:: mainDiv

            The instance passed as first argument of the :py:meth:`run()` method.

        .. py:attribute:: overlayPanel

            DivNode that stands always on top of the MainDiv.

        .. py:attribute:: settings

            An instance of settings.Settings.

        .. py:method:: dumpTextObjectCount()

            Dumps on the console the current number of initialized objects. Normally
            bound to the keypress :kbd:`CTRL-b`.

        .. py:method:: onBeforeLaunch()

            Called just before starting the main loop (:samp:`Player.play()`). Useful only
            for subclassing. The display hasn't been initialized at this point.

        .. py:method:: run(mainDiv, **kargs)

            Starts the application using the provided :py:attr:`mainDiv` (an instance of
            :py:class:`MainDiv`).
            
            The optional additional kargs are used to set default settings - see 
            :py:class:`settings.Settings`.

        .. py:method:: takeScreenshot(targetFolder='.')

            Takes a screenshot of what is currently visible on the screen. Normally
            bound to the keypress :kbd:`CTRL-p`. Screenshots are saved to the disk under
            the name 'App-nnn.png', where nnn is a sequence number.

    
    .. autoclass:: MainDiv

        This abstract class must be subclassed to write a libavg application. It is the
        main application entry point and should be the parent node for all 
        application-created nodes. All the public methods are empty and don't do anything
        if not overridden.

        .. py:attribute:: VERSION

            A version string. This is shown using the :option:`-v` or :option:`--version`
            command-line option.

        .. py:method:: onArgvParsed(options, args, parser)

            This method is called after command-line arguments have been parsed and should
            be used to retrieve any application-specific options. The arguments 
            :py:attr:`options` and :py:attr:`args` are the result of calling
            :samp:`options, args = parser.parse_args()`, where parser is an instance of 
            :py:class:`optparse.OptionParser` configured by calling 
            :py:meth:`onArgvParserCreated`.
            
        .. py:method:: onArgvParserCreated(parser)

            Called with an empty :py:class:`optparse.OptionParser` instance. Allows the
            application to add additional command line options. :py:class:`App` adds it's 
            own parameters as well. If this is overridden, :py:meth:`onArgvParsed()` 
            should probably be overridden as well.

        .. py:method:: onExit()

            Called after the main loop exits.
            Release resources and run cleanups here.

            .. note::

                onExit() doesn't get called if an exception is raised on the main thread.

        .. py:method:: onFrame()

            Called every frame.

        .. py:method:: onInit()

            Called by a libavg timer as soon as the main loop starts.
            Build the application node tree here.

        .. py:method:: onStartup()

            Called before libavg has been setup, just after the App().run() call.


keyboardmanager Module
----------------------

.. automodule:: libavg.app.keyboardmanager
    :no-members:

    This module makes it possible to attach event handlers to individual keypresses. 
    Keys that are bound through the keyboard manager are also be shown in the debug panel
    via the keyboard bindings widget along with their help string.
    :py:class:`libavg.app.App` defines a range of keyboard bindings by default. 
    Conceptually, all keys modified by :kbd:`CTRL` are reserved by 
    :py:class:`libavg.app.App`.

    For all the binding methods, keystring can be a python string or a unicode object.
    The modifiers are described under :py:attr:`libavg.avg.KeyEvent.modifiers`.

    .. py:function:: bindKeyDown(keystring, handler, help, modifiers=avg.KEYMOD_NONE)

        Sets up a key handler so that :py:attr:`handler` is called whenever 
        :py:attr:`keystring` is pressed.

    .. py:function:: bindKeyUp(keystring, handler, help, modifiers=avg.KEYMOD_NONE)

        Sets up a key handler so that :py:attr:`handler` is called whenever 
        :py:attr:`keystring` is released.

    .. py:function:: disable()

        Companion to :py:meth:`enable()`, disables all handlers.

    .. py:function:: enable()

        Companion to :py:meth:`disable()`, enables all handlers.

    .. py:function:: getCurrentBindings()

        Returns the currently assigned bindings as a list of named tuples.

    .. py:function:: init()

        Called by :py:class:`App`. Should not be called by user programs.

    .. py:function:: pop()

        Companion to :py:meth:`push()`, restores the non-modified key bindings
        previously pushed onto the stack via :py:meth:`push()`.

    .. py:function:: push()

        Pushes all the non-modified key bindings onto a stack and clears them all.
        Useful when the application flow branches to a state where a different key
        bindings set is needed. The bindings can be then restored with :py:meth:`pop()`.

    .. py:function:: unbindAll()

        Removes all the defined key bindings at once.

    .. py:function:: unbindKeyDown(keystring, modifiers=avg.KEYMOD_NONE)

        Removes a previously defined key binding for a KEY_DOWN event.

    .. py:function:: unbindKeyDown(keystring, modifiers=avg.KEYMOD_NONE)

        Removes a previously defined key binding for a KEY_UP event.


.. settings Module
  ---------------

  .. automodule:: libavg.app.settings
    :no-members:

    .. autoclass:: Settings(defaults=[])
        
        The :py:class:`Settings` class holds a collection of application options as 
        :py:class:`Option` instances. It is usually instantiated by :py:class:`App`. The
        available options and defaults are either passed on construction or configured 
        using :py:meth:`Settings.addOption`. Usually, the configuration happens in 
        :py:class:`App` (for general options) and :py:meth:`MainDiv.onArgvParserCreated` 
        (for application-specific options). :py:class:`App` takes converts parameters to 
        :samp:`App.run()` and command-line arguments to the configured options.

        .. py:method:: addOption(option)

            Adds an option.

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


flashmessage Module
-------------------

.. automodule:: libavg.app.flashmessage
    :no-members:

    .. autoclass:: FlashMessage(text, timeout=DEFAULT_TIMEOUT, parent=None, isError=False, acknowledge=False)

        A :py:class:`FlashMessage` is a user notification shown as a text line. The
        message can have an optional timeout or stay visible until clicked on by the user.
        It inserts itself into node tree at the top. Multiple :py:class:`FlashMessage` 
        instances are shown in the order they get created.

        :param timeout: The time in milliseconds the message should persist on screen
                before it gets removed. Only valid if :py:attr:`acknowledge` is 
                :py:const:`False`.
        
        :param parent: When specified, the parent node the message should be appending
                itself to.

        :param isError: A boolean flag to mark the message as error. Error-flagged
                messages are shown in a different color and are routed to the
                logger as well.

        :param acknowledge: A flag to indicate whether the message should remove itself
                automatically (after timeout has elapsed) or needs to be acknowledged
                by the user (by clicking / touching on it).



