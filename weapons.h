/* GENERADO desde los prefabs de Chicken Gun (dano, dps, precision, precio).
 * Cargador y recarga NO estan en los prefabs: se estiman segun la cadencia. */
#ifndef WEAPONS_H
#define WEAPONS_H

enum { WK_GUN = 0, WK_MELEE = 1, WK_EXPLOSIVE = 2 };
typedef struct { const char *id; int dmg; float cd; int mag; float reload; float tol; int kind; int price; int free_item; } Weapon;

#define NWEAP 21
static const Weapon WEAPONS[NWEAP] = {
    {"1SN", 120, 1.00f, 5, 1.30f, 0.17f, 0, 190, 0},
    {"3S", 18, 0.24f, 20, 1.60f, 0.53f, 0, 25, 1},
    {"AK47", 16, 0.26f, 20, 1.60f, 0.51f, 0, 50, 0},
    {"AUG", 30, 0.33f, 12, 1.44f, 0.51f, 0, 185, 0},
    {"Axe", 65, 1.10f, 0, 1.20f, 0.13f, 1, 50, 0},
    {"BigRevolver", 34, 0.40f, 12, 1.44f, 0.42f, 0, 110, 0},
    {"BlasterT", 23, 0.26f, 20, 1.60f, 0.37f, 0, 220, 0},
    {"Bow1", 51, 0.67f, 8, 1.36f, 0.51f, 0, 80, 0},
    {"CopSG", 19, 0.24f, 20, 1.60f, 0.53f, 0, 70, 0},
    {"Crossbow", 34, 0.43f, 12, 1.44f, 0.51f, 0, 90, 0},
    {"DDEagle", 20, 0.25f, 20, 1.60f, 0.42f, 0, 140, 0},
    {"DesertEagle", 25, 0.33f, 12, 1.44f, 0.42f, 0, 25, 0},
    {"G35", 14, 0.26f, 20, 1.60f, 0.37f, 0, 30, 1},
    {"GrenadeGun", 100, 0.71f, 4, 2.20f, 0.34f, 2, 100, 0},
    {"Knife", 75, 1.00f, 0, 1.20f, 0.13f, 1, 48, 0},
    {"Knife2", 120, 1.00f, 0, 1.20f, 0.13f, 1, 100, 0},
    {"LMchGun", 23, 0.27f, 20, 1.60f, 0.51f, 0, 190, 0},
    {"Mac10", 20, 0.31f, 12, 1.44f, 0.42f, 0, 90, 0},
    {"PirMiniG", 25, 0.22f, 20, 1.60f, 0.46f, 0, 250, 0},
    {"RocketLauncher", 100, 0.71f, 4, 2.20f, 0.34f, 2, 100, 0},
    {"SSG3000", 90, 1.00f, 5, 1.30f, 0.17f, 0, 0, 1},
};

#endif
