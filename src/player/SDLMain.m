#import <SDL/SDL.h>
#import "SDLMain.h"

#import <Cocoa/Cocoa.h>

void CustomSDLMain()
{
     NSAutoreleasePool  *pool = [[NSAutoreleasePool alloc] init];
     [ NSApplication sharedApplication ];
     [ NSApp setMainMenu:[[NSMenu alloc] init] ];
}

