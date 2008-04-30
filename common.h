#ifndef COMMON_H_
#define COMMON_H_

// customizable options
// OSC Port 
#define _OSCPORT "7770"

// start/stop editor with engine
#define _BUNDLE

// disable this when you experience sluggish midi reaction
#define _MIDIBLOCK 1


// debug output
//#define _DEBUG

// prefetch support with intrinsics
//#define _PREFETCH

// vectorize calculations
//#define _VECTOR

// 'old' binary fileformat, switched off per default
//#define _BINFILE

// not customizable options ----------------------------------
// amount of parameters per patch
#define _PARACOUNT 139

// how many additional settings per sound in multi to store
#define _MULTISETTINGS 6

// the version number as string
#define _VERSION "1.2"

#endif
