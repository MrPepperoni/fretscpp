#ifndef __HDR_verbosity_h
#define __HDR_verbosity_h

#include "message.h"

const int LEV_QUIET       = 0;
const int LEV_WARN        = 1;
const int LEV_INFO        = 2;
const int LEV_DBG         = 3;
const int LEV_VDBG        = 4;

#ifdef DISABLEMESSAGES
const int MSG_BIAS        = 9;
#else
const int MSG_BIAS        = 0;
#endif

const int MSG_VPXDEC      = LEV_QUIET  ;
const int MSG_VIDEO       = LEV_QUIET  ;
const int MSG_AVIFILE     = LEV_QUIET  ;

const int MSG_AUDIO       = LEV_QUIET  ;
const int MSG_SFX         = LEV_QUIET  ;
const int MSG_SONG        = LEV_QUIET  ;
const int MSG_AUDIOFILE   = LEV_QUIET  ;

const int MSG_SONGDB      = LEV_QUIET  ;
const int MSG_TAPPABLE    = LEV_QUIET  ;
const int MSG_THEME       = LEV_QUIET  ;
const int MSG_FONT        = LEV_QUIET  ;
const int MSG_READMID     = LEV_QUIET  ;
const int MSG_MIDI        = LEV_QUIET  ;
const int MSG_MIDIDUMP    = LEV_QUIET  ;
const int MSG_SPRITES     = LEV_QUIET  ;
const int MSG_SETSCENE    = LEV_QUIET  ;
const int MSG_SCNGUITAR   = LEV_QUIET  ;
const int MSG_SCNSONGOPT  = LEV_QUIET  ;
const int MSG_GL          = LEV_QUIET  ;
const int MSG_JOYSTICK    = LEV_QUIET  ;
const int MSG_SONGINI     = LEV_QUIET  ;
const int MSG_SCORES      = LEV_QUIET  ;
const int MSG_CONFIG      = LEV_QUIET  ;
const int MSG_STAGE       = LEV_QUIET  ;
const int MSG_STAGEFX     = LEV_QUIET  ;
const int MSG_PNGREAD     = LEV_QUIET  ;
const int MSG_TEXMAN      = LEV_QUIET  ;
const int MSG_NUMEXPR     = LEV_QUIET  ;

#define INFO(tgt,...)    __COND_DBG_OUT(MSG_##tgt >= MSG_BIAS+LEV_INFO,__VA_ARGS__)
#define WARN(tgt,...)    __COND_DBG_OUT_COLOR(MSG_##tgt >= MSG_BIAS+LEV_WARN,__VA_ARGS__)
#define DBG(tgt,...)     __COND_DBG_OUT(MSG_##tgt >= MSG_BIAS+LEV_DBG, __VA_ARGS__)
#define VDBG(tgt,...)    __COND_DBG_OUT(MSG_##tgt >= MSG_BIAS+LEV_VDBG, __VA_ARGS__)


#endif
