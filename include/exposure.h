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
#ifndef _EXPOSURE_H_
#define _EXPOSURE_H_

/**
 * A series of utilities to compute exposure, exposure value at ISO 100 (EV100),
 * luminance and illuminance using a physically-based camera model.
 */

/**
 * Returns the exposure value (EV at ISO 100) of the specified exposure parameters.
 */
float ev100(float aperture, float shutter_speed, float sensitivity);

/**
 * Returns the exposure value (EV at ISO 100) for the given average luminance (in @f$ \frac{cd}{m^2} @f$).
 */
float ev100_from_luminance(float luminance);

/**
* Returns the exposure value (EV at ISO 100) for the given illuminance (in lux).
*/
float ev100_from_illuminance(float illuminance);

/**
 * Returns the photometric exposure for the specified exposure parameters.
 * This function is equivalent to calling `exposure(ev100(aperture, shutter_speed, sensitivity))`
 * but is slightly faster and offers higher precision.
 */
float exposure(float aperture, float shutter_speed, float sensitivity);

/**
 * Returns the photometric exposure for the given EV100.
 */
float exposure_from_ev100(float ev100);

/**
 * Returns the incident luminance in @f$ \frac{cd}{m^2} @f$ for the specified exposure parameters of
 * a camera acting as a spot meter.
 * This function is equivalent to calling `luminance(ev100(aperture, shutter_speed, sensitivity))`
 * but is slightly faster and offers higher precision.
 */
float luminance(float aperture, float shutter_speed, float sensitivity);

/**
 * Converts the specified EV100 to luminance in @f$ \frac{cd}{m^2} @f$.
 * EV100 is not a measure of luminance, but an EV100 can be used to denote a
 * luminance for which a camera would use said EV100 to obtain the nominally
 * correct exposure
 */
float luminance_from_ev100(float ev100);

/**
 * Returns the illuminance in lux for the specified exposure parameters of
 * a camera acting as an incident light meter.
 * This function is equivalent to calling `illuminance(ev100(aperture, shutter_speed, sensitivity))`
 * but is slightly faster and offers higher precision.
 */
float illuminance(float aperture, float shutter_speed, float sensitivity);

/**
 * Converts the specified EV100 to illuminance in lux.
 * EV100 is not a measure of illuminance, but an EV100 can be used to denote an
 * illuminance for which a camera would use said EV100 to obtain the nominally
 * correct exposure.
 */
float illuminance_from_ev100(float ev100);

#endif /* ! _EXPOSURE_H_ */
