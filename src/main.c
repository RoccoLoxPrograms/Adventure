#include <tice.h>
#include <math.h>
#include <keypadc.h>
#include <graphx.h>
#include <fileioc.h>
#include <stdint.h>

#include "font/font.h"
#include "gfx/sprites.h"

int8_t playerHeldObject = 13;
int16_t objectPos[] = {
    5, 56, 128, // 0: Black key
    13, 56, 128, // 1: Goldeny key
    21, 56, 128, // 2: Whitish key
    23, 220, 160, // 3: Magneto
    12, 100, 140, // 4: Sword (or arrow or whatever you see it as)
    26, 128, 104, // 5: Magick bridge
    1, 96, 170, // 6: Chalice
    2, 160, 200, // 7: Grey dot
    13, 64, 48, // 8: the bat (dun dun DUN)
    15, 160, 120, // 9: Grundle (the green dragon)
    11, 110, 120, // 10: Yurgle (the yeller dragon)
    1, 60, 120 // 11: Rhindle (the red dragon)
};

void magnet(int8_t);
void drawRoom(int8_t, uint8_t);
/* There are 30 rooms in Adventure. Their "room numbers" are as such:
0: Black Castle Maze, top-left
1: Black Castle Maze, top-right
2: Black Castle Maze, bottom-left
3: Black Castle Maze, bottom-right
4: red room above the Black Castle
5: the Black Castle
6: Blue Maze, right under the Black Castle
7: Blue Maze, right under the one right under the Black Castle
8: Blue Maze, entrance
9: Blue Maze, the other one that is to the very left on the picture
10: Blue Maze, right under the one that is right under the one right under the Black Castle
11: dark-green room that is one left and one down from the Golden Castle
12: room above the Golden Castle
13: the Golden Castle
14: light-green room below the Golden Castle
15: yellow room to the right of the light-green room
16: ¡EASTER EGG ROOM!
17: Red Maze, top-left
18: Red Maze, top-right
19: Red Maze, bottom-left
20: Red Maze, bottom-right
21: the White Castle
22: light-green room below the White Castle
23: dark-green room 2 down from the White Castle
24: Orange Maze, entrance
25: Orange Maze, middle
26: Orange Maze, bottom
27: purple room one right and one up from the exit of the Orange Maze
28: light-blue room right from the exit of the Orange maze
29: red room below the light-blue room
30: the room above the easter-egg room

Each map has a resolution of 7 x 20 pixels (which then
gets flipped across the screen for a total of 7 x 40), while
the entire screen has a resolution 160 x 192. The current
method for storing and drawing a room's data is this:
{11, 1, 1, 1, 1, 1, 1, 3}
The first "11" signifies to draw 11 map pixels. The next "1"
signifies to skip over one map pixel before drawing another
"1" map pixel. It skips one, draws one, skips one, draws 1,
skips one, draws one, and finally skips 3. This draws the top
battlements of the castle.
*/
const uint8_t roomColor[31] = {0, 0, 0, 0, 8, 2, 10, 10, 10, 10, 10, 5, 6, 6, 4, 3, 9, 8, 8, 8, 8, 1, 12, 13, 0, 0, 0, 9, 11, 8, 9};
const int8_t roomTransitions[124] = {
    1, 2, 3, 2, // room 0's room transitions
    2, 29, 0, 3, // room 1's room transitions, and so on and so forth
    3, 0, 1, 0, // room 2
    0, 1, 2, 4, // room 3
    4, 3, 4, 5, // room 4
    5, 11, 5, 6, // room 5
    9, 5, 10, 7, // room 6
    8, 6, 8, 10, // room 7
    7, 9, 7, 11, // room 8
    10, 29, 6, 8, // room 9
    6, 7, 9, 15, // room 10
    14, 8, 15, 5, // room 11
    12, 12, 12, 13, // room 12
    13, 14, 13, 14, // room 13
    15, 13, 11, 13, // room 14
    16, 10, 14, 24, // room 15
    11, 30, 15, 10, // room 16
    18, 19, 18, 19, // room 17
    17, 23, 17, 20, // room 18
    20, 17, 20, 17, // room 19
    19, 18, 19, 21, // room 20
    21, 23, 21, 22, // room 21
    26, 21, 28, 23, // room 22
    22, 22, 27, 21, // room 23
    25, 15, 25, 25, // room 24
    24, 24, 24, 26, // room 25
    28, 25, 22, 27, // room 26
    21, 29, 21, 28, // room 27
    22, 27, 26, 29, // room 28
    23, 28, 23, 27, // room 29
    30, 30, 30, 16 // room 30
};
const int8_t objectSize[] = {20, 10, 20, 10, 20, 10, 20, 20, 20, 14, 20, 52, 20, 22, 6, 6, 20, 26, 20, 40, 20, 40, 20, 40};

int main(void) {
    // all that setup stuff
    gfx_Begin();
    fontlib_SetFont(AdvFont,0);
    fontlib_SetTransparency(true);
    timer_Enable(1, TIMER_32K, TIMER_NOINT, TIMER_UP);
    rtc_Enable(RTC_SEC_INT);
    srand(rtc_Time());
    gfx_FillScreen(2);
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTransparentColor(0);
    // mm mm mm delicious variables
    int8_t currentRoom = 13, castleGateOpener = 0, holdingCooldown = 0, castleGatePos[3] = {24, 24, 24}, dragonStates[3] = {0, 0, 0}, dragonDirections[6] = {0, 0, 0, 0, 0, 0}, batDirection[2] = {randInt(-1, 1), randInt(-1, 1)}, batHeldObject = 14;
    int16_t manX = 156;
    uint8_t manY = 176, chaliceColor = 1;
    bool inOrangeMaze, win = false, dead = false, batAnimation, batHungry = true;
    // display the epic title screen that was created by TIny_Hacker
    gfx_SetColor(0);
    gfx_FillRectangle_NoClip(0, 24, 320, 192);
    drawRoom(14, 9);
    gfx_ScaledTransparentSprite_NoClip(AdventureTitle, 2, 40, 2, 2);
    fontlib_SetCursorPosition(37, 201);
    fontlib_DrawString("(Re)made by RoccoLox Programs");
    fontlib_SetForegroundColor(2);
    fontlib_SetCursorPosition(101, 128);
    fontlib_DrawString("Press any key...");
    gfx_SetDrawBuffer();
    gfx_FillScreen(2);
    // wait for a button to be pressed before moving on to the actual game
    do {
        kb_Scan();
    } while (kb_AnyKey());
    while (!kb_AnyKey()) kb_Scan();
    while (!(kb_Data[1] & kb_Del || win || dead)) {
        timer_Set(1, 0);
        kb_Scan();
        // check for and execute any room transitions
        if (!manX && kb_Data[7] & kb_Left) {
            currentRoom = roomTransitions[4 * currentRoom + 2];
            manX = 312;
            objectPos[3 * playerHeldObject + 1] += 312;
        } else if (manX == 312 && kb_Data[7] & kb_Right) {
            currentRoom = roomTransitions[4 * currentRoom];
            manX = 0;
            objectPos[3 * playerHeldObject + 1] -= 312;
        } else if (manY == 24 && kb_Data[7] & kb_Up) {
            currentRoom = roomTransitions[4 * currentRoom + 1];
            manY = 208;
            objectPos[3 * playerHeldObject + 2] += 184;
        } else if (manY == 208 && kb_Data[7] & kb_Down) {
            currentRoom = roomTransitions[4 * currentRoom + 3];
            manY = 24;
            objectPos[3 * playerHeldObject + 2] -= 184;
        }
        objectPos[3 * playerHeldObject] = currentRoom;
        // if mode is pressed, then resurrect the dragons, reset the player back to the golden castle, and make the player let go of whatever item they were carrying
        if (kb_Data[1] & kb_Mode) {
            dragonStates[0] = 0;
            dragonStates[1] = 0;
            dragonStates[2] = 0;
            manX = 156;
            manY = 176;
            currentRoom = 13;
            playerHeldObject = 13;
        }
        // lets go of the item being held if [2nd] is pressed
        if (kb_Data[1] & kb_2nd) playerHeldObject = 13;
        // checks to see if the player is in a foggy orange maze
        if (currentRoom < 4 || (currentRoom > 23 && currentRoom < 27)) inOrangeMaze = true;
        else inOrangeMaze = false;
        // checks all the objects to see if the player should pick one up or if the object needs to perform a room transition
        for (int8_t i = 0; i < 12; i++) {
            uint8_t j = 3 * i;
            // check if the bat picks up an item
            if (objectPos[24] == objectPos[j] && batHungry && i != 7 && i != 8) {
                if (gfx_CheckRectangleHotspot(objectPos[25], objectPos[26], 16, 22, objectPos[j + 1] - 2, objectPos[j + 2] - 2, objectSize[2 * i], objectSize[2 * i + 1])) {
                    if (i == playerHeldObject) playerHeldObject = 13;
                    batHeldObject = i;
                    batHungry = false;
                }
            }
            // check if the player picks up an item
            if (!holdingCooldown && currentRoom == objectPos[j]) {
                if (gfx_CheckRectangleHotspot(manX, manY, 8, 8, objectPos[j + 1] - 2, objectPos[j + 2] - 2, objectSize[2 * i], objectSize[2 * i + 1]) || (i == 5 && gfx_CheckRectangleHotspot(manX, manY, 8, 8, objectPos[16] + 46, objectPos[17] - 2, 20, 52))) {
                    if (i < 9) {
                        // if the man picked up a different item...
                        if (playerHeldObject != i && batHeldObject != i) {
                            // the holding cooldown is set to 8 frames
                            holdingCooldown = 8;
                            // the man picks up the corresponding object that it touched
                            playerHeldObject = i;
                        } else holdingCooldown = 0;
                        // that object is shifted to be outside the man. it goes to the right by default but in the other directions if the key is being pressed
                        if (kb_Data[7] & kb_Right) objectPos[j + 1] += 4;
                        if (kb_Data[7] & kb_Left) objectPos[j + 1] -= 4;
                        if (kb_Data[7] & kb_Up) objectPos[j + 2] -= 4;
                        if (kb_Data[7] & kb_Down) objectPos[j + 2] += 4;
                        // otherwise, if the object is a dragon, then
                    } else if (!dragonStates[i - 9] && dragonStates[i - 9] < 126) {
                        // set it to bite for X amount of frames
                        dragonStates[i - 9] = 30;
                        // move the dragon to be biting the player
                        objectPos[j + 1] = manX - 2;
                        objectPos[j + 2] = manY - 6;
                    }
                }
            }
            // if the object is a dragon
            if (i > 8) {
                // checks to see if the player is bitten by the dragon
                if (dragonStates[i - 9] && dragonStates[i - 9] < 126) {
                    dragonStates[i - 9]--;
                    if (dragonStates[i - 9] == 1) {
                        if (gfx_CheckRectangleHotspot(manX, manY, 8, 8, objectPos[j + 1], objectPos[j + 2] + 4, 8, 16)) dragonStates[i - 9] = 126;
                        else dragonStates[i - 9] = 0;
                    }
                // If the man gets eaten, they and their item are teleported to the dragon's stomach
                } else if (dragonStates[i - 9] == 126) {
                    objectPos[3 * playerHeldObject + 1] += objectPos[j + 1] + 4 - manX;
                    objectPos[3 * playerHeldObject + 2] += objectPos[j + 2] + 20 - manY;
                    manX = objectPos[j + 1] + 4;
                    manY = objectPos[j + 2] + 20;
                    currentRoom = objectPos[j];
                    objectPos[3 * playerHeldObject] = currentRoom;
                }
                // checks to see if the dragon got killed by the sword
                if (!dragonStates[i - 9] && objectPos[12] == objectPos[j] && gfx_CheckRectangleHotspot(objectPos[13] + 2, objectPos[14] + 4, 12, 2, objectPos[j + 1], objectPos[j + 2], 16, 40)) dragonStates[i - 9] = 127;
            }
            // does the room transitions for all the objects
            if (i != playerHeldObject) {
                if (objectPos[j + 1] + (objectSize[2 * i] / 2 - 2) < 4) {
                    objectPos[j] = roomTransitions[4 *objectPos[j] + 2];
                    objectPos[j + 1] += 312;
                }
                if (objectPos[j + 1] + (objectSize[2 * i] / 2 - 2) > 316) {
                    objectPos[j] = roomTransitions[4 * objectPos[j]];
                    objectPos[j + 1] -= 312;
                }
                if (objectPos[j + 2] + (objectSize[2 * i + 1] / 2 - 2) < 28) {
                    objectPos[j] = roomTransitions[4 * objectPos[j] + 1];
                    objectPos[j + 2] += 184;
                }
                if (objectPos[j + 2] + (objectSize[2 * i + 1] / 2 - 2) > 212) {
                    // if the object leaves a room into the castle, transport it to the castle gate
                    if ((objectPos[j] == 4 || objectPos[j] == 12 || objectPos[j] == 20)) {
                        if (castleGatePos[(currentRoom - 4) / 8] != 24) objectPos[j]++;
                        else objectPos[j + 2] -= 124;
                        objectPos[j + 2] -= 60;
                    } else {
                        objectPos[j] = roomTransitions[4 * objectPos[j] + 3];
                        objectPos[j + 2] -= 184;
                    }
                }
            }
        }
        // if the man is in an orange maze, then set the background to orange instead of grey
        gfx_SetColor(7 * inOrangeMaze);
        gfx_FillRectangle_NoClip(0, 24, 320, 192);
        // if the man is in a castle room, then...
        if (currentRoom == 5 || currentRoom == 13 || currentRoom == 21){
            // if the man comes into the castle room, teleport them to the entrance of the castle instead of the top of the screen
            if (manY == 24 && manX >= 128 && manX <= 184) {
                objectPos[3 * playerHeldObject + 1] += 156 - manX;
                objectPos[3 * playerHeldObject + 2] += 148 - manY;
                manY = 148;
                manX = 156;
            }
            // if the key touches the gate that it's supposed to unlock, then it sets the castleGateOpener variable
            for (int8_t i = 0; i < 3; i++) {
                if (objectPos[3 * i] == currentRoom && currentRoom == 8 * i + 5 && !castleGateOpener && gfx_CheckRectangleHotspot(objectPos[1 + 3 * i], objectPos[2 + 3 * i], 16, 6, 153, 140 + castleGatePos[i], 14, 4)) {
                    castleGateOpener = 2 * (!castleGatePos[i]) - 1;
                }
            }
            // do a room transition when they enter the castle instead of when you hit the top of the screen
            if (castleGatePos[(currentRoom - 5) / 8] != 24 && manX > 145 && manX < 167 && manY == 144) {
                currentRoom--;
                manY = 204;
                castleGatePos[(currentRoom - 3) / 8] = 0;
                castleGateOpener = 0;
                objectPos[3 * playerHeldObject + 2] += 60;
                objectPos[3 * playerHeldObject] = currentRoom;
                if (playerHeldObject == 6 && currentRoom == 12) win = true;
            }
            // draw the castle gate at its position (whether open or closed)
            else gfx_ScaledSprite_NoClip(CastleGate, 153, 112 + castleGatePos[(currentRoom - 5) / 8], 2, 2);
        }
        // opens/closes the castle gates if the castleGateOpener was set
        castleGatePos[(currentRoom - 3)  / 8] += castleGateOpener;
        if (castleGatePos[(currentRoom - 3) / 8] >= 24 || castleGatePos[(currentRoom - 3) / 8] <= 0) castleGateOpener = 0;
        // redraw the room on top of everything
        drawRoom(currentRoom, roomColor[currentRoom]);
        // if the man is in the easter egg room, then display the easter egg room's text
        if (currentRoom == 16) {
            // Created Warren.... by....i..Robbinett
            const uint8_t easteRegg[36] = {67, 114, 101, 97, 116, 101, 100, 32, 87, 97, 114, 114, 101, 110, 46, 46, 46, 32, 98, 121, 46, 46, 46, 46, 105, 46, 46, 82, 111, 98, 105, 110, 101, 116, 116};
            for (int8_t i = 0; i < 35; i++) {
                fontlib_SetForegroundColor(chaliceColor);
                fontlib_SetCursorPosition(151 + 12 * (i > 16), 24 + 11 * i - 197 * (i > 16));
                fontlib_DrawGlyph(easteRegg[i]);
            }
        }
        // these if statments dictate what the green dragon will gravitate towards, such as the player or the black key or the magic bridge or the magnet
        if (!dragonStates[0]) {
            // every 9 or so seconds, the dragon will choose a different different direction randomly, unless..
            if (!chaliceColor) {
                do {
                    dragonDirections[0] = randInt(-1, 1);
                    dragonDirections[1] = randInt(-1, 1);
                } while (!(dragonDirections[0] || dragonDirections[1]));
            }
            // if the player is in the same room, chase the player
            if (objectPos[27] == currentRoom) {
                dragonDirections[0] = (objectPos[28] < manX - 4) - (objectPos[28] > manX - 4);
                dragonDirections[1] = (objectPos[29] < manY - 4) - (objectPos[29] > manY - 4);
            // if the chalice is in the same room, chase the chalice
            } else if (objectPos[27] == objectPos[18]) {
                dragonDirections[0] = (objectPos[28] < objectPos[19]) - (objectPos[28] > objectPos[19]);
                dragonDirections[1] = (objectPos[29] < objectPos[20]) - (objectPos[29] > objectPos[20]);
            // if the magic bridge is in the same room, chase the magic bridge
            } else if (objectPos[27] == objectPos[15]) {
                dragonDirections[0] = (objectPos[28] < objectPos[16]) - (objectPos[28] > objectPos[16]);
                dragonDirections[1] = (objectPos[29] < objectPos[17]) - (objectPos[29] > objectPos[17]);
            // if the magnet is in the same room, chase the magnet
            } else if (objectPos[27] == objectPos[9]) {
                dragonDirections[0] = (objectPos[28] < objectPos[10]) - (objectPos[28] > objectPos[10]);
                dragonDirections[1] = (objectPos[29] < objectPos[11]) - (objectPos[29] > objectPos[11]);
            // if the black key is in the same room, chase the black key
            } else if (objectPos[27] == objectPos[0]) {
                dragonDirections[0] = (objectPos[28] < objectPos[1]) - (objectPos[28] > objectPos[1]);
                dragonDirections[1] = (objectPos[29] < objectPos[2]) - (objectPos[29] > objectPos[2]);
            }
            objectPos[28] += 2 * dragonDirections[0];
            objectPos[29] += 2 * dragonDirections[1];
        }
        // draw the proper sprite depending on what state the dragon is in
        if (objectPos[27] == currentRoom) {
            if (!dragonStates[0] || dragonStates[0] == 126) gfx_ScaledTransparentSprite_NoClip(GrindleChase, objectPos[28], objectPos[29], 2, 2);
            else if (dragonStates[0] == 127) gfx_ScaledTransparentSprite_NoClip(GrindleDead, objectPos[28], objectPos[29], 2, 2);
            else gfx_ScaledTransparentSprite_NoClip(GrindleEat, objectPos[28], objectPos[29], 2, 2);
        }
        // the yellow dragon's behavior now!
        if (!dragonStates[1]) {
            if (!chaliceColor) {
                do {
                    dragonDirections[2] = randInt(-1, 1);
                    dragonDirections[3] = randInt(-1, 1);
                } while (!(dragonDirections[2] || dragonDirections[3]));
            }
            // if the yellow key is in the same room, then run away from the yellow key (like a coward)
            if (objectPos[30] == objectPos[3]) {
                dragonDirections[2] = (objectPos[31] > objectPos[4]) - (objectPos[31] < objectPos[4]);
                dragonDirections[3] = (objectPos[32] > objectPos[5]) - (objectPos[32] < objectPos[5]);
            // if the man is in the same room, chase the man
            } else if (objectPos[30] == currentRoom) {
                dragonDirections[2] = (objectPos[31] < manX - 4) - (objectPos[31] > manX - 4);
                dragonDirections[3] = (objectPos[32] < manY - 4) - (objectPos[32] > manY - 4);
            // if the chalice is in the same room, chase the chalice
            } else if (objectPos[30] == objectPos[18]) {
                dragonDirections[2] = (objectPos[31] < objectPos[19]) - (objectPos[31] > objectPos[19]);
                dragonDirections[3] = (objectPos[32] < objectPos[20]) - (objectPos[32] > objectPos[20]);
            }
            objectPos[31] += 2 * dragonDirections[2];
            objectPos[32] += 2 * dragonDirections[3];
        }
        if (objectPos[30] == currentRoom) {
            if (!dragonStates[1] || dragonStates[1] == 126) gfx_ScaledTransparentSprite_NoClip(YurgleChase, objectPos[31], objectPos[32], 2, 2);
            else if (dragonStates[1] == 127) gfx_ScaledTransparentSprite_NoClip(YurgleDead, objectPos[31], objectPos[32], 2, 2);
            else gfx_ScaledTransparentSprite_NoClip(YurgleEat, objectPos[31], objectPos[32], 2, 2);
        }
        // the red dragon's behavior now!
        if (!dragonStates[2]) {
            if (!chaliceColor) {
                do {
                    dragonDirections[4] = randInt(-1, 1);
                    dragonDirections[5] = randInt(-1, 1);
                } while (!(dragonDirections[4] || dragonDirections[5]));
            }
            // if the man is in the same room, chase the man
            if (objectPos[33] == currentRoom) {
                dragonDirections[4] = (objectPos[34] < manX - 4) - (objectPos[34] > manX - 4);
                dragonDirections[5] = (objectPos[35] < manY - 4) - (objectPos[35] > manY - 4);
            // if the chalice is in the same room, chase the chalice
            } else if (objectPos[33] == objectPos[18]) {
                dragonDirections[4] = (objectPos[34] < objectPos[19]) - (objectPos[34] > objectPos[19]);
                dragonDirections[5] = (objectPos[35] < objectPos[20]) - (objectPos[35] > objectPos[20]);
            // if the white key is in the same room, chase the white key
            } else if (objectPos[33] == objectPos[6]) {
                dragonDirections[4] = (objectPos[34] < objectPos[7]) - (objectPos[34] > objectPos[7]);
                dragonDirections[5] = (objectPos[35] < objectPos[8]) - (objectPos[35] > objectPos[8]);
            }
            objectPos[34] += 3 * dragonDirections[4];
            objectPos[35] += 3 * dragonDirections[5];
        }
        if (objectPos[33] == currentRoom) {
            if (!dragonStates[2] || dragonStates[2] == 126) gfx_ScaledTransparentSprite_NoClip(RhindleChase, objectPos[34], objectPos[35], 2, 2);
            else if (dragonStates[2] == 127) gfx_ScaledTransparentSprite_NoClip(RhindleDead, objectPos[34], objectPos[35], 2, 2);
            else gfx_ScaledTransparentSprite_NoClip(RhindleEat, objectPos[34], objectPos[35], 2, 2);
        }
        // the bat behavior
        if (batHungry) {
            if (objectPos[24] == objectPos[3 * playerHeldObject] && playerHeldObject != 8 && playerHeldObject != 13) {
                batDirection[0] = (objectPos[25] < objectPos[3 * playerHeldObject + 1]) - (objectPos[25] > objectPos[3 * playerHeldObject + 1]);
                batDirection[1] = (objectPos[26] < objectPos[3 * playerHeldObject + 2]) - (objectPos[26] > objectPos[3 * playerHeldObject + 2]);
            } else {
                for (int8_t i = 0; i < 12; i++) {
                    while (i == 7 || i == 8 || i == batHeldObject || i == playerHeldObject) i++;
                    if (objectPos[24] == objectPos[3 * i]) {
                        batDirection[0] = (objectPos[25] < objectPos[3 * i + 1]) - (objectPos[25] > objectPos[3 * i + 1]);
                        batDirection[1] = (objectPos[26] < objectPos[3 * i + 2]) - (objectPos[26] > objectPos[3 * i + 2]);
                        break;
                    }
                }
            }
        } else {
            if (!chaliceColor) {
                do {
                    batDirection[0] = randInt(-1, 1);
                    batDirection[1] = randInt(-1, 1);
                } while (!(batDirection[0] || batDirection[1]));
                batHungry = true;
            }
        }
        if (playerHeldObject != 8) {
            // the bat moves in the direction that it is going in
            objectPos[25] += 4 * batDirection[0];
            objectPos[26] += 4 * batDirection[1];
        }
        objectPos[3 * batHeldObject] = objectPos[24];
        objectPos[3 * batHeldObject + 1] = objectPos[25] + 18;
        objectPos[3 * batHeldObject + 2] = objectPos[26] + 6;
        // check for collisions then move the man and the item being held
        if (kb_Data[7] & kb_Right) {
            // the man is moved right 'cause you're pressing the right key
            // two GetPixels are performed to check if the man has bonked into a wall. If the man has bonked into a wall, then he is not moved
            if (!((gfx_GetPixel(manX + 9, manY) != 7 * inOrangeMaze) || (gfx_GetPixel(manX + 9, manY + 7) != 7 * inOrangeMaze))) {
                manX += 4;
                // the object that is being held by the player is also moved
                objectPos[3 * playerHeldObject + 1] += 4;
            }
        }
        if (kb_Data[7] & kb_Left) {
            if (!((gfx_GetPixel(manX - 2, manY) != 7 * inOrangeMaze) || (gfx_GetPixel(manX - 2, manY + 7)  != 7 * inOrangeMaze))) {
                manX -= 4;
                objectPos[3 * playerHeldObject + 1] -= 4;
            }
        }
        if (kb_Data[7] & kb_Down) {
            // if the player is inside of the magic bridge, then they can always move down (or up) no matter what walls may try to stop them
            if ((playerHeldObject != 5 && objectPos[15] == currentRoom && manX > objectPos[16] + 16 && manX < objectPos[16] + 40 && manY < objectPos[17] + 40 && manY + 10 > objectPos[17]) || !((gfx_GetPixel(manX, manY + 11)  != 7 * inOrangeMaze) || (gfx_GetPixel(manX + 7, manY + 11)  != 7 * inOrangeMaze))) {
                manY += 4;
                objectPos[3 * playerHeldObject + 2] += 4;
            }
        }
        if (kb_Data[7] & kb_Up) {
            if ((playerHeldObject != 5 && objectPos[15] == currentRoom && manX > objectPos[16] + 16 && manX < objectPos[16] + 40 && manY < objectPos[17] + 50 && manY - 2 > objectPos[17]) || !((gfx_GetPixel(manX, manY - 4) != 7 * inOrangeMaze) || (gfx_GetPixel(manX + 7, manY - 4)  != 7 * inOrangeMaze))) {
                manY -= 4;
                objectPos[3 * playerHeldObject + 2] -= 4;
            }
        }
        // draw that man
        gfx_SetColor(roomColor[currentRoom]);
        gfx_FillRectangle(manX, manY, 8, 8);
        if (inOrangeMaze) {
            /*
            gfx_BlitRectangle(gfx_buffer, (manX - 28) * (manX > 28 && manX < 284) + 256 * (manX >= 284), (manY - 52) * (manY > 52 && manY < 180) + 128 * (manY >= 180) + 24, 64, 64);
            gfx_SetColor(0);
            gfx_FillRectangle_NoClip(0, 24, 320, 192);
            gfx_BlitRectangle(gfx_screen, (manX - 28) * (manX > 28 && manX < 284) + 256 * (manX >= 284), (manY - 52) * (manY > 52 && manY < 180) + 128 * (manY >= 180) + 24, 64, 64);
            */
            /*
            gfx_SetColor(0);
            gfx_FillRectangle_NoClip(0, 24, 320, (manY - 52) * (manY > 52 && manY < 180) + 128 * (manY >= 180));
            gfx_FillRectangle_NoClip(0, (manY - 52) * (manY > 52 && manY < 180) + 128 * (manY >= 180), (manX - 28) * (manX > 28 && manX < 284) + 256 * (manX >= 284), 192);
            gfx_FillRectangle_NoClip(0, (manY - 52) * (manY > 52 && manY < 180) + 128 * (manY >= 180) + 64, 320, 216 - ((manY - 52) * (manY > 52 && manY < 180) + 128 * (manY >= 180) + 64));
            gfx_FillRectangle_NoClip((manX - 28) * (manX > 28 && manX < 284) + 256 * (manX >= 284) + 64, (manY - 52) * (manY > 52 && manY < 180) + 128 * (manY >= 180), 320 - ((manX - 28) * (manX > 28 && manX < 284) + 256 * (manX >= 284) + 64), 192);
            */
            /*
            gfx_GetSprite(FogBackground, (manX - 28) * (manX > 28 && manX < 284) + 256 * (manX >= 284), (manY - 52) * (manY > 52 && manY < 180) + 128 * (manY >= 180) + 24);
            gfx_SetColor(0);
            gfx_FillRectangle_NoClip(0, 24, 320, 192);
            gfx_Sprite_NoClip(FogBackground, (manX - 28) * (manX > 28 && manX < 284) + 256 * (manX >= 284), (manY - 52) * (manY > 52 && manY < 180) + 128 * (manY >= 180) + 24);
            */
        }
        // time to draw the bat!
        if (objectPos[24] == currentRoom) {
            // draws the different animations of the bat
            if (batAnimation) gfx_ScaledTransparentSprite_NoClip(Knubberrub1, objectPos[25], objectPos[26], 2, 2);
            else gfx_ScaledTransparentSprite_NoClip(Knubberrub2, objectPos[25], objectPos[26], 2, 2);
            // changes the bat's animation every 8 frames
            if (!(chaliceColor % 8)) batAnimation = !batAnimation;
        }
        // decrement the holding cooldown each frame
        if (holdingCooldown) holdingCooldown--;
        // make the magnet attract other objects
        magnet(currentRoom);
        // draw the keys
        if (currentRoom == objectPos[0]) gfx_ScaledTransparentSprite_NoClip(BlackKey, objectPos[1], objectPos[2], 2, 2);
        if (currentRoom == objectPos[3]) gfx_ScaledTransparentSprite_NoClip(GoldKey, objectPos[4], objectPos[5], 2, 2);
        if (currentRoom == objectPos[6]) gfx_ScaledTransparentSprite_NoClip(WhiteKey, objectPos[7], objectPos[8], 2, 2);
        // draw the sword
        if (currentRoom == objectPos[12]) gfx_ScaledTransparentSprite_NoClip(Sword, objectPos[13], objectPos[14], 2, 2);
        // don't ask any questions about why the magic bridge is different. It's just special that way
        if (currentRoom == objectPos[15]) gfx_TransparentSprite(MagicBridge, objectPos[16], objectPos[17]);
        // draw the chalice with its flashing colors
        if (currentRoom == objectPos[18]) {
            fontlib_SetForegroundColor(chaliceColor);
            fontlib_SetCursorPosition(objectPos[19], objectPos[20]);
            fontlib_DrawGlyph(0);
        }
        chaliceColor++;
        // draw the grey dot
        if (currentRoom == objectPos[21]) {
            gfx_SetColor(0);
            gfx_FillRectangle_NoClip(objectPos[22], objectPos[23], 2, 2);
        }
        // because there is not a version of gfx_ScaledTransparentSprite with clipping, we have to draw black bars across the top and bottom of the screen to stop stuff from drawing up there. Weeeeeeeeeeeeeeee...
        gfx_SetColor(2);
        gfx_FillRectangle_NoClip(0, 0, 320, 24);
        gfx_FillRectangle_NoClip(0, 216, 320, 24);
        // these are for debugging. Don't worry about 'em
        // gfx_SetTextXY(2, 2);
        // gfx_PrintInt(, 1);
        gfx_SwapDraw();
        while (timer_Get(1) < 1092);
    }
    // if the player wins, then do the flashing colors of the screen
    if (win) {
        gfx_SwapDraw();
        for (chaliceColor = 102; chaliceColor != 7; chaliceColor++) {
            timer_Set(1, 0);
            kb_Scan();
            drawRoom(12, chaliceColor);
            gfx_SetColor(chaliceColor);
            gfx_FillRectangle(manX, manY, 8, 8);
            fontlib_SetForegroundColor(chaliceColor);
            fontlib_SetCursorPosition(objectPos[19], objectPos[20]);
            fontlib_DrawGlyph(0);
            gfx_SetColor(2);
            gfx_FillRectangle_NoClip(0, 216, 320, 24);
            gfx_BlitBuffer();
            while (timer_Get(1) < 1092);
        }
        while (!(kb_Data[1] & kb_Del)){
            timer_Set(1, 0);
            kb_Scan();
            chaliceColor++;
            fontlib_SetForegroundColor(chaliceColor);
            fontlib_SetCursorPosition(objectPos[19], objectPos[20]);
            fontlib_DrawGlyph(0);
            gfx_BlitBuffer();
            while (timer_Get(1) < 1092);
        }
    }
    gfx_End();
}

void drawRoom(int8_t roomNumber, uint8_t roomColor) {
    const int8_t roomData[508] = {
        8, 4, 8, 12, 2, 6, 14, 26, 2, 4, 14, 6, 2, 12, 8, 4, 8, // room 0; position 0
        20, 18, 2, 0, 16, 2, 2, 14, 2, 4, 4, 4, 12, 8, 4, 6, 2, 0, 2, 2, 2, 2, 4, 2, 2, 2, 2, // room 1; position 17
        8, 4, 8, 0, 2, 18, 2, 4, 14, 6, 2, 12, 8, 4, 8, 0, 2, 10, 2, 6, 8, 4, 8, // room 2; position 44
        2, 2, 2, 2, 4, 2, 2, 2, 2, 4, 2, 8, 2, 2, 2, 0, 16, 24, 16, 24, 16, // room 3; position 67
        16, 4, 2, 18, 2, 18, 2, 18, 2, 18, 2, 18, 16, // room 4; position 88
        11, 1, 1, 1, 1, 1, 1, 3, 2, 8, 7, 3, 2, 8, 10, 0, 2, 10, 8, 0, 2, 10, 6, 2, 2, 18, 16, // rooms 5, 13, 21; position 101
        16, 12, 2, 4, 2, 4, 4, 4, 2, 4, 4, 2, 4, 4, 2, 10, 18, 8, 2, 8, 2, 2, 4, 2, 2, 2, 8, // room 6; position 128
        4, 2, 2, 2, 8, 8, 2, 6, 4, 2, 12, 2, 4, 12, 2, 2, 4, 2, 4, 2, 2, 2, 2, 2, 4, 8, 2, 2, 2, 2, 2, 4, 8, 2, 2, 2, 2, // room 7; position 155
        8, 2, 2, 2, 2, 2, 2, 6, 2, 2, 2, 2, 2, 4, 4, 2, 2, 2, 2, 2, 6, 6, 2, 2, 2, 8, 8, 2, 10, 20, 16, // room 8; position 192
        20, 20, 10, 2, 8, 0, 4, 14, 2, 0, 4, 2, 10, 2, 2, 6, 2, 6, 2, 2, 2, 0, 8, 2, 2, 2, 2, 2, 2, // room 9; position 223
        8, 2, 2, 2, 2, 10, 2, 6, 2, 4, 4, 2, 10, 4, 4, 16, 8, 18, 2, 12, 20, // room 10; position 252
        16, 104, 20, // rooms 11, 14, 16; position 273
        20, 0, 2, 18, 2, 18, 2, 18, 2, 18, 2, 18, 16, //rooms 12, 27; position 276
        20, 100, 16, // room 15; position 289
        20, 20, 16, 18, 2, 4, 12, 2, 2, 4, 4, 6, 2, 2, 2, 2, 2, 0, 4, 2, 2, 2, 6, 2, 2, // room 17; position 292
        20, 18, 2, 0, 16, 2, 2, 14, 2, 2, 2, 0, 4, 2, 2, 2, 10, 0, 4, 2, 2, 2, 2, 8, 4, 2, 6, 2, 2, // room 18; position 317
        4, 2, 2, 2, 6, 2, 2, 0, 4, 2, 2, 12, 4, 2, 2, 2, 10, 6, 2, 2, 2, 8, 12, 28, 16, // room 19; position 346
        4, 2, 6, 2, 2, 4, 4, 10, 2, 4, 16, 10, 2, 12, 4, 2, 2, 18, 2, 12, 16, // room 20; position 371
        16, 104, 16, // rooms 22, 28; position 393
        16, 4, 2, 18, 2, 18, 2, 18, 2, 18, 2, 18, 20, // rooms 23, 29; position 396
        16, 10, 2, 12, 4, 2, 2, 4, 8, 6, 2, 10, 2, 0, 8, 2, 2, 6, 2, 10, 2, 6, 2, 0, 12, 2, 2, 2, 2, // room 24, position 409
        12, 2, 2, 2, 2, 14, 2, 2, 2, 0, 4, 6, 6, 2, 2, 10, 2, 8, 8, 2, 2, 2, 6, 6, 2, 2, 2, 2, 2, 4, 4, 2, 2, 2, 2, 2, 2, 2, 2, // room 25; position 438
        4, 2, 2, 2, 2, 2, 2, 2, 2, 6, 2, 6, 2, 2, 2, 6, 10, 2, 2, 18, 2, 6, 8, 4, 2, 6, 2, 10, 2, 0, 20 // room 26, position 477
    };
    const int16_t roomLoadPos[31] = {0, 17, 44, 67, 88, 101, 128, 155, 192, 223, 252, 273, 276, 101, 273, 289, 273, 292, 317, 346, 371, 101, 392, 395, 408, 437, 476, 276, 392, 395, 276};
    const int8_t roomLoadDuration[31] = {18, 28, 24, 22, 14, 28, 28, 38, 32, 30, 22, 4, 14, 28, 4, 4, 4, 26, 30, 26, 22, 28, 4, 14, 30, 40, 32, 14, 4, 14, 14};

    uint8_t roomDrawer = 0;
    // draws the room based on the data in roomData
    gfx_SetColor(roomColor);
    for (uint16_t i = roomLoadPos[roomNumber]; i < roomLoadDuration[roomNumber] + roomLoadPos[roomNumber]; i += 2) {
        gfx_FillRectangle_NoClip(8 * (roomDrawer % 20), 24 + 32 * floor(roomDrawer / 20) - 16 * (roomDrawer > 19), 8 * roomData[i], 16 + 16 * (roomDrawer > 19 && roomDrawer < 120));
        if (roomNumber == 1 || roomNumber == 2) gfx_FillRectangle_NoClip(8 * (roomDrawer % 20) + 160, 24 + 32 * floor(roomDrawer / 20) - 16 * (roomDrawer > 19), 8 * roomData[i], 16 + 16 * (roomDrawer > 19 && roomDrawer < 120));
        else gfx_FillRectangle(320 - (8 * (roomDrawer % 20)) - 8 * roomData[i], 24 + 32 * floor(roomDrawer / 20) - 16 * (roomDrawer > 19), 8 * roomData[i], 16 + 16 * (roomDrawer > 19 && roomDrawer < 120));
        roomDrawer += roomData[i] + roomData[i + 1];
    }
    gfx_SetColor(2);
    // for those rooms that have a random black bar down the side, draw that black bar down the side
    if (roomNumber == 11 || roomNumber == 22) gfx_FillRectangle_NoClip(16, 24, 4, 192);
    if ((roomNumber == 15 && playerHeldObject != 7) || roomNumber == 28) gfx_FillRectangle_NoClip(300, 24, 4, 192);
}

void magnet(int8_t currentRoom) {
    if (currentRoom == objectPos[9]) gfx_ScaledTransparentSprite_NoClip(Magnet, objectPos[10], objectPos[11], 2, 2);
    for (int8_t i = 0; i < 7; i++) {
        i += (i == 3 || i == playerHeldObject);
        i += (i == 3 || i == playerHeldObject);
        if (objectPos[3 * i] == objectPos[9]) {
            objectPos[3 * i + 1] += (objectPos[3 * i + 1] < objectPos[10]) - (objectPos[3 * i + 1] > objectPos[10]);
            objectPos[3 * i + 2] += (objectPos[3 * i + 2] < objectPos[11] + 18) - (objectPos[3 * i + 2] > objectPos[11] + 18);
            break;
        }
    }
}
