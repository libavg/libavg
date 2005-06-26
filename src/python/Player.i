//
// $Id$
//

%module avg
%{
#include "../player/Player.h"
#include "../player/Node.h"
#include "../player/AVGNode.h"
%}

%include "std_string.i"

namespace avg {

%typemap(python,in) PyObject *PyFunc {
  if (!PyCallable_Check($source)) {
      PyErr_SetString(PyExc_TypeError, "Need a callable object!");
      return NULL;
  }
  $target = $source;
}

class Node;
class AVGNode;

class Player //: IEventSink
{
    public:
        Player ();
        virtual ~Player ();

        /**
         * Loads the avg file specified in fileName. Returns false if the file 
         * could not be opened.
         */
        void loadFile (const std::string& fileName);
        
        /**
         * Opens a playback window or screen and starts playback. framerate is 
         * the number of frames per second that should be displayed. 
         */
        void play (double framerate);
        
        /**
         * Stops playback and resets the video mode if nessesary.
         */
        void stop ();

        Node * createNodeFromXml (const std::string& sXML);

        /**
         * Sets code that should be executed every time milliseconds.
         * The smallest timeframe that can be set is once per frame. If less
         * than that is specified, the code will be executed exactly once per 
         * frame. The function returns an id that can be used to call 
         * clearInterval() to stop the code from being called.
         */
//        int setInterval(int time, TimeoutFunc code);
        /**
         * Sets code that should be executed after time milliseconds.
         * The function returns an id that can be used to call clearInterval() 
         * to stop the code from being called.
         */
        int setTimeout(int time, PyObject * pyfunc);
        /**
         * Stops a timeout or an interval from being called. Returns true if 
         * there was an interval with the given id, false if not.
         */
        bool clearInterval(int id);

        /**
         * Gets an interface to the current event. Only valid inside event 
         * handlers (onmouseup, onmousedown, etc.)
         */
//        Event& getCurEvent();

        /**
         * Saves the contents of the current screen in a png file. Returns 
         * true on success, false if the screen couldn't be saved.
         */
        bool screenshot(const std::string& sFilename);
        /**
         * Shows or hides the mouse cursor. (Currently, this only works for 
         * OpenGL. Showing the DirectFB mouse cursor seems to expose some 
         * issue with DirectFB.)
         */
        void showCursor(bool bShow);

        /**
         * Returns an element in the avg tree. The id corresponds to the id 
         * attribute of the node. 
         */
        Node * getElementByID (const std::string& id);
        /**
         * Returns the outermost element in the avg tree. 
         */
        AVGNode * getRootNode ();
        double getFramerate ();
};

}
