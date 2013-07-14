App Package
===========

libavg.app package contains modules that manage the initialization and configuration
of a libavg application.

.. note::

    The app package is experimental. Functionality and interface are still in flux and
    subject to change.

.. automodule:: libavg.app
    :no-members:

    .. autoclass:: App

        This class takes care of setting up a root node resolution, screen
        rotation, base parent for the user's MainDiv, debug panel, keyboard manager and
        handles settings and settings overrides via command line arguments.
        It's supposed to be used as a helper class that receives a MainDiv instance
        to its run() method.

        .. py:method:: run(mainDiv, **kargs)

            Starts the application upon mainDiv (an instance of libavg.app.MainDiv).
            The optional additional kargs are used to set the defaults of the Settings
            instance.

        .. py:method:: run(mainDiv, **kargs)

            Stops the player.

        .. py:method:: onBeforeLaunch()

            Called just before starting the main loop (Player.play()). Useful only
            for subclassing.

        .. py:method:: takeScreenshot()

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

        .. py:method:: run(mainDiv, **kargs)

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

        .. py:method:: onFrame(delta)

            Called at every frame. delta is the time that has passed since the last
            call of onFrame().

.. automodule:: libavg.keyboardmanager
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
