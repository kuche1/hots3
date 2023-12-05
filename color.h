
#ifndef COLOR_H
#define COLOR_H

// https://gist.github.com/fnky/458719343aabd01cfb17a3a4f7296797

// foreground colors
#define COL_GREEN_BRIGHT   {'\033', '[', '9', '2', 'm'}
#define COL_GREEN_DARK     {'\033', '[', '3', '2', 'm'}
#define COL_YELLOW_BRIGHT  {'\033', '[', '9', '3', 'm'}
#define COL_YELLOW_DARK    {'\033', '[', '3', '3', 'm'}
#define COL_MAGENTA_BRIGHT {'\033', '[', '9', '5', 'm'}
#define COL_MAGENTA_DARK   {'\033', '[', '3', '5', 'm'}
#define COL_RED_BRIGHT     {'\033', '[', '9', '1', 'm'}
#define COL_RED_DARK       {'\033', '[', '3', '1', 'm'}

// background colors
#define COL_BG_RED_DARK     {'\033', '[', '4', '1',      'm'}
#define COL_BG_BLUE_DARK    {'\033', '[', '4', '4',      'm'}
#define COL_BG_WHITE_DARK   {'\033', '[', '4', '6',      'm'}
#define COL_BG_WHITE_BRIGHT {'\033', '[', '1', '0', '7', 'm'}
#define COL_BG_MAGENTA_DARK {'\033', '[', '4', '5',      'm'}
#define COL_BG_BLACK_DARK   {'\033', '[', '4', '0',      'm'}

// effects
#define EFFECT_BOLD               {'\033', '[',      '1', 'm'}
#define EFFECT_NO_BOLD            {'\033', '[', '2', '2', 'm'} // note that this is used for disabling both bold and dim/faint
#define EFFECT_ITALIC             {'\033', '[',      '3', 'm'}
#define EFFECT_NO_ITALIC          {'\033', '[', '2', '3', 'm'}
#define EFFECT_UNDERLINE          {'\033', '[',      '4', 'm'}
#define EFFECT_NO_UNDERLINE       {'\033', '[', '2', '4', 'm'}
#define EFFECT_BLINK              {'\033', '[',      '5', 'm'}
#define EFFECT_NO_BLINK           {'\033', '[', '2', '5', 'm'}
#define EFFECT_INVERSE_REVERSE    {'\033', '[',      '7', 'm'}
#define EFFECT_NO_INVERSE_REVERSE {'\033', '[', '2', '7', 'm'}
#define EFFECT_STRIKETHROUGH      {'\033', '[',      '9', 'm'}
#define EFFECT_NO_STRIKETHROUGH   {'\033', '[', '2', '9', 'm'}

// this is not the most appropriate place of all but it will be fine for now
#define RESET_WITH_SPACE {'\033', '[', '0', 'm', ' '}

#endif
