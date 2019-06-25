/*********************************************************************************************************************/
/*                                                  /===-_---~~~~~~~~~------____                                     */
/*                                                 |===-~___                _,-'                                     */
/*                  -==\\                         `//~\\   ~~~~`---.___.-~~                                          */
/*              ______-==|                         | |  \\           _-~`                                            */
/*        __--~~~  ,-/-==\\                        | |   `\        ,'                                                */
/*     _-~       /'    |  \\                      / /      \      /                                                  */
/*   .'        /       |   \\                   /' /        \   /'                                                   */
/*  /  ____  /         |    \`\.__/-~~ ~ \ _ _/'  /          \/'                                                     */
/* /-'~    ~~~~~---__  |     ~-/~         ( )   /'        _--~`                                                      */
/*                   \_|      /        _)   ;  ),   __--~~                                                           */
/*                     '~~--_/      _-~/-  / \   '-~ \                                                               */
/*                    {\__--_/}    / \\_>- )<__\      \                                                              */
/*                    /'   (_/  _-~  | |__>--<__|      |                                                             */
/*                   |0  0 _/) )-~     | |__>--<__|     |                                                            */
/*                   / /~ ,_/       / /__>---<__/      |                                                             */
/*                  o o _//        /-~_>---<__-~      /                                                              */
/*                  (^(~          /~_>---<__-      _-~                                                               */
/*                 ,/|           /__>--<__/     _-~                                                                  */
/*              ,//('(          |__>--<__|     /                  .----_                                             */
/*             ( ( '))          |__>--<__|    |                 /' _---_~\                                           */
/*          `-)) )) (           |__>--<__|    |               /'  /     ~\`\                                         */
/*         ,/,'//( (             \__>--<__\    \            /'  //        ||                                         */
/*       ,( ( ((, ))              ~-__>--<_~-_  ~--____---~' _/'/        /'                                          */
/*     `~/  )` ) ,/|                 ~-_~>--<_/-__       __-~ _/                                                     */
/*   ._-~//( )/ )) `                    ~~-'_/_/ /~~~~~~~__--~                                                       */
/*    ;'( ')/ ,)(                              ~~~~~~~~~~                                                            */
/*   ' ') '( (/                                                                                                      */
/*     '   '  `                                                                                                      */
/*********************************************************************************************************************/
#ifndef _PTIME_H_
#define _PTIME_H_

#include <stdint.h>

/*
 * Call once before any other functions to initialize internal structures
 * (this calls for instance QueryPerformanceFrequency on Windows)
 */
void time_setup();

/*
 * Get current point in time in unspecified 'ticks'. The value that
 * is returned has no relation to the 'wall-clock' time and is
 * not in a specific time unit, it is only useful to compute
 * time differences.
 */
uint64_t time_now();

/*
 * Computes the time difference between new and old. This will always
 * return a positive, non-zero value.
 */
uint64_t time_diff(uint64_t new_ticks, uint64_t old_ticks);

/*
 * Takes the current time, and returns the elapsed time since start
 * (this is a shortcut for `time_diff(time_now(), start)`)
 */
uint64_t time_since(uint64_t start_ticks);

/*
 * This is useful for measuring frame time and other recurring
 * events. It takes the current time, returns the time difference
 * to the value in last_time, and stores the current time in
 * last_time for the next call. If the value in last_time is 0,
 * the return value will be zero (this usually happens on the
 * very first call).
 */
uint64_t time_laptime(uint64_t* last_time);

/*
 * Converts a tick value into seconds, milliseconds, microseconds
 * or nanoseconds. Note that not all platforms will have nanosecond
 * or even microsecond precision.
 */
double time_sec(uint64_t ticks);
double time_msec(uint64_t ticks);
double time_usec(uint64_t ticks);
double time_nsec(uint64_t ticks);

#endif /* ! _PTIME_H_ */
