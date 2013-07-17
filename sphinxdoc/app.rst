App Package
===========

libavg.app package contains modules that manage the initialization and configuration
of a libavg application.

.. note::

    The app package is experimental. Functionality and interface are still in flux and
    subject to change.

app module
==========

.. automodule:: libavg.app
    :no-members:

    .. autoclass:: App

        This class takes care of setting up a root node resolution, screen
        rotation, base parent for the user's MainDiv, debug panel, keyboard manager and
        handles settings and settings overrides via command line arguments.
        It's supposed to be used as a helper class that receives a MainDiv instance
        to its run() method.

        .. py:method:: run(mainDiv, **kargs)

            Starts the application using the provided mainDiv (an instance of
            libavg.app.MainDiv).
            
            The optional additional kargs are used to set the defaults of the Settings
            instance.

            TODO: maybe an example here

        .. py:method:: stop()

            Stops the player.

        .. py:method:: onBeforeLaunch()

            Called just before starting the main loop (Player.play()). Useful only
            for subclassing.

        .. py:method:: takeScreenshot(targetFolder='.')

            Takes a screenshot of what is currently visible on the screen. Normally
            bound to the keypress CTRL-p

        .. py:method:: dumpTextObjectCount()

            Dumps on the console the current number of initialized objects. Normally
            bound to the keypress CTRL-b.

        .. py:attribute:: mainDiv

            The instance passed as first argument of the run() method.

        .. py:attribute:: appParent

            The DivNode where the application sets up the children Divs (MainDiv, debug,
                    overlay)

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

        This abstract class, once subclassed, is the placeholder for the main user's code.
        Its member methods are to be implemented, at least onInit().

        .. py:attribute:: INIT_FUNC

            A string that represents the identifier of the method that will be called
            on the MainDiv instance upon initialization of the App. It defaults to onInit.

        .. py:attribute:: VERSION

            A version string. This is shown using the -v or --version CLI option.

        .. py:method:: onArgvParserCreated(parser)

            Called with a pristine optparse.OptionParser instance where additional (to the
            default ones) command line options can be defined. Once implemented, it's
            likely that the onArgvParsed() has to be implemented as well.

        .. py:method:: onArgvParsed(options, args, parser)

            The arguments options and args are the result of calling:
            options, args = parser.parse_args()
            where parser is a configured instance of optparse.OptionParser.
            In this function it's possible to retrieve the values of the options added
            to the parser on onArgvParserCreated().

        .. py:method:: onStartup()

            Called before libavg has been setup, mostly just after App().run() call.
            Setup early services or communication channels here.

        .. py:method:: onInit()

            Called by a libavg timer as soon as the main loop starts.
            Build the application node tree here.

        .. py:method:: onExit()

            Called when the main loop exits.
            Release resources and run cleanups here.

            .. note::

                onExit() doesn't get called if an exception is raised on the main thread.

        .. py:method:: onFrame(delta)

            Called at every frame. delta is the time that has passed since the last
            call of onFrame().


Keyboard handling
=================

.. automodule:: libavg.app.keyboardmanager
    :no-members:

    This module provides a convenient interface to keyboard events management. Keys
    that are bound thru the keyboard manager can also be shown in the debug panel via
    the keyboard bindings widgets along with their help string.

    For all the binding methods, keystring can be a python string or a unicode object.
    TODO: expand the discussion regarding keystring vs SDL
    TODO: describe the modifiers

    .. py:method:: init()

        Called by App, it's unlikely that has to be called by the user.

    .. py:method:: bindKeyDown(keystring, handler, help, modifiers=avg.KEYMOD_NONE)

        Creates a binding for a KEY_DOWN event for the given keystring and the callable
        handler.

    .. py:method:: bindKeyUp(keystring, handler, help, modifiers=avg.KEYMOD_NONE)

        Creates a binding for a KEY_UP event for the given keystring and the callable
        handler.

    .. py:method:: unbindKeyDown(keystring, modifiers=avg.KEYMOD_NONE)

        Removes a previously defined key binding for a KEY_DOWN event.

    .. py:method:: unbindKeyDown(keystring, modifiers=avg.KEYMOD_NONE)

        Removes a previously defined key binding for a KEY_UP event.

    .. py:method:: unbindAll()

        Removes all the defined key bindings at once.

    .. py:method:: push()

        Pushes all the non-modified key bindings onto a stack and clear them all.
        Useful when the application flow branches to a state where a different key
        bindings set is needed. The bindings can be then restored with pop().

    .. py:method:: pop()

        Companion to push(), restores the non-modified key bindings set previously pushed
        onto the stack via push().

    .. py:method:: getCurrentBindings()

        Returns the currently assigned bindings as a list of keyboardmanager._KeyBindings
        named tuples.

    .. py:method:: enable()

        Companion to disable(), enables the keybindings handlers.

    .. py:method:: disable()

        Companion to enable(), disables the keybindings handlers.


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

