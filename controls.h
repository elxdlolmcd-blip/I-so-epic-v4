/*
 * Controles Chicken Gun (Unity InputManager) -> DualShock 2
 *  Stick izq: mover        Stick der: camara        L3: caminar lento (Change Speeds)
 *  R1/R2: disparar         L1: apuntar (zoom)       L2: granada (Secondary)
 *  X: saltar   O: agacharse (mantener)   Cuadrado: recargar   Triangulo: picotazo (Action)
 *  D-Pad izq/der: arma anterior/siguiente   D-Pad arriba: guardar/sacar arma (Equip Item Toggle)
 *  START: volver al menu    SELECT: reaparecer
 */
#ifndef CONTROLS_H
#define CONTROLS_H
#include <libpad.h>
#define BTN_FIRE        PAD_R1
#define BTN_FIRE2       PAD_R2
#define BTN_AIM         PAD_L1
#define BTN_GRENADE     PAD_L2
#define BTN_JUMP        PAD_CROSS
#define BTN_CROUCH      PAD_CIRCLE
#define BTN_RELOAD      PAD_SQUARE
#define BTN_ACTION      PAD_TRIANGLE
#define BTN_WALK        PAD_L3
#define BTN_PREV_WEAPON PAD_LEFT
#define BTN_NEXT_WEAPON PAD_RIGHT
#define BTN_HOLSTER     PAD_UP
#define BTN_MENU        PAD_START
#define BTN_RESTART     PAD_SELECT
#define STICK_DEADZONE  24
#define INVERT_Y        0
#endif
