#include <tice.h>
#include <math.h>
#include <keypadc.h>
#include <graphx.h>
#include <fileioc.h>
#include <stdint.h>

#include "font/font.h"
#include "gfx/sprites.h"

int8_t playerHeldObject = 9;
int8_t castleGatePos[3] = {0, 0, 0};
uint16_t objectPos[] = {13, 48, 128, 5, 48, 128, 21, 48, 128};

int8_t key(uint8_t, int16_t, uint8_t, int8_t);
void drawRoom(uint8_t);
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
    5, 3, 5, 5, // room 4
    5, 4, 5, 6, // room 5
    9, 5, 10, 7, // room 6
    8, 6, 8, 10, // room 7
    7, 9, 7, 11, // room 8
    10, 29, 6, 8, // room 9
    6, 7, 9, 15, // room 10
    14, 8, 15, 21, // room 11
    14, 12, 14, 13, // room 12
    13, 12, 13, 14,  // room 13
    15, 13, 11, 13, // room 14
    16, 10, 14, 24, // room 15
    11, 30, 15, 10, // room 16
    18, 19, 18, 19, // room 17
    17, 23, 17, 20, // room 18
    20, 17, 20, 17, // room 19
    19, 18, 19, 21, // room 20
    21, 20, 21, 22, // room 21
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

int main() {
    // all that setup stuff
    gfx_Begin();
    fontlib_SetFont(AdvFont,0);
    timer_Enable(1, TIMER_32K, TIMER_NOINT, TIMER_UP);
    rtc_Enable(RTC_SEC_INT);
    srand(rtc_Time());
    gfx_FillScreen(2);
    gfx_SetDrawBuffer();
    gfx_FillScreen(2);
    gfx_SetPalette(global_palette, sizeof_global_palette, 0);
    gfx_SetTransparentColor(0);

    uint8_t currentRoom = 13;
    int16_t manX = 156;
    uint8_t manY = 176;
    int8_t holdingCooldown = 0;
    bool inOrangeMaze;
    while (!(kb_Data[6] & kb_Clear)) {
        timer_Set(1, 0);
        // check for and execute any room transitions
        if (!manX) {
            currentRoom = roomTransitions[4 * currentRoom + 2];
            manX = 308;
            objectPos[3 * playerHeldObject + 1] += 308;
        } else if (manX == 312) {
            currentRoom = roomTransitions[4 * currentRoom];
            manX = 4;
            objectPos[3 * playerHeldObject + 1] -= 308;
        } else if (manY == 24) {
            currentRoom = roomTransitions[4 * currentRoom + 1];
            manY = 204;
            objectPos[3 * playerHeldObject + 2] += 180;
        } else if (manY == 208) {
            currentRoom = roomTransitions[4 * currentRoom + 3];
            manY = 28;
            objectPos[3 * playerHeldObject + 2] -= 180;
        }
        // check to see if the man is in an orange foggy maze
        if (currentRoom < 4 || (currentRoom > 23 && currentRoom < 27)) inOrangeMaze = true;
        else inOrangeMaze = false;
        // if the man is in an orange maze, then set the background to orange instead of grey
        gfx_SetColor(7 * inOrangeMaze);
        gfx_FillRectangle_NoClip(0, 24, 320, 192);
        // if the man is in a castle room, then...
        if (currentRoom == 5 || currentRoom == 13 || currentRoom == 21){
            // do a room transition when they enter the castle instead of when you hit the top of the screen
            if (manX > 145 && manX < 167 && manY == 144) {
                currentRoom--;
                manY = 204;
                objectPos[3 * playerHeldObject + 2] += 60;
            }
            // draw the castle gate at its position (whether open or closed)
            else gfx_ScaledSprite_NoClip(CastleGate, 153, 112 + castleGatePos[(currentRoom - 5)/8], 2, 2);
            // if the man comes into the castle room, teleport them to the entrance of the castle instead of the top of the screen
            if (manY == 28) {
                manY = 148;
                objectPos[3 * playerHeldObject + 2] += 120;
                objectPos[3 * playerHeldObject + 1] += 156 - manX;
                manX = 156;
            }
        }
        objectPos[3 * playerHeldObject] = currentRoom;
        // redraw the room on top of everything
        drawRoom(currentRoom);

        kb_Scan();
        // lets go of the item you're holding if [2nd] is pressed
        if (kb_Data[1] & kb_2nd) playerHeldObject = 9;
        // check for collisions then move the man, and maybe
        if (kb_Data[7] & kb_Right) {
            // the man is moved right 'cause you're pressing the right key
            // two GetPixels are performed to check if the man has bonked into a wall. If the man has bonked into a wall, then he is not moved
            if (!((gfx_GetPixel(manX + 11, manY) != 7 * inOrangeMaze) || (gfx_GetPixel(manX + 11, manY + 7) != 7 * inOrangeMaze))) {
                manX += 4;
                // the object that is being held by the player is also moved
                objectPos[3 * playerHeldObject + 1] += 4;
            }
        }
        if (kb_Data[7] & kb_Left) {
            if (!((gfx_GetPixel(manX - 4, manY) != 7 * inOrangeMaze) || (gfx_GetPixel(manX - 4, manY + 7)  != 7 * inOrangeMaze))) {
                manX -= 4;
                objectPos[3 * playerHeldObject + 1] -= 4;
            }
        }
        if (kb_Data[7] & kb_Down) {
            if (!((gfx_GetPixel(manX, manY + 11)  != 7 * inOrangeMaze) || (gfx_GetPixel(manX + 7, manY + 11)  != 7 * inOrangeMaze))) {
                manY += 4;
                objectPos[3 * playerHeldObject + 2] += 4;
            }
        }
        if (kb_Data[7] & kb_Up) {
            if (!((gfx_GetPixel(manX, manY - 4) != 7 * inOrangeMaze) || (gfx_GetPixel(manX + 7, manY - 4)  != 7 * inOrangeMaze))) {
                manY -= 4;
                objectPos[3 * playerHeldObject + 2] -= 4;
            }
        }
        // draw that man
        gfx_SetColor(roomColor[currentRoom]);
        gfx_FillRectangle(manX, manY, 8, 8);
        // if the man is in one of the orange foggy maze rooms, then do the foggy maze effect
        if (inOrangeMaze) {
            // to do the aforementioned foggy maze effect, this is the best solution we could find that didn't cause any blinking:
            // first, blit the rectangle of visibility to the screen (with some conditions so it doesn't go off the edge of the screen)
            gfx_BlitRectangle(gfx_buffer, (manX - 28) * (manX > 28 && manX < 284) + 256 * (manX >= 284), (manY - 52) * (manY > 52 && manY < 180) + 128 * (manY >= 180) + 24, 64, 64);
            // then, cover the buffer in grey
            gfx_SetColor(0);
            gfx_FillRectangle_NoClip(0, 24, 320, 192);
            // and finally, re-blit the rectangle from the screen back to the buffer
            gfx_BlitRectangle(gfx_screen, (manX - 28) * (manX > 28 && manX < 284) + 256 * (manX >= 284), (manY - 52) * (manY > 52 && manY < 180) + 128 * (manY >= 180) + 24, 64, 64);
        }

        holdingCooldown = key(currentRoom, manX, manY, holdingCooldown);
        if (holdingCooldown) holdingCooldown--;

        gfx_SetColor(2);
        gfx_FillRectangle_NoClip(0, 0, 320, 24);
        gfx_FillRectangle_NoClip(0, 216, 320, 24);
        gfx_SwapDraw();
        while (timer_Get(1) < 1092);
    }
    gfx_End();
}

void drawRoom(uint8_t roomNumber) {
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
    gfx_SetColor(roomColor[roomNumber]);
    for (uint16_t i = roomLoadPos[roomNumber]; i < roomLoadDuration[roomNumber] + roomLoadPos[roomNumber]; i += 2) {
        gfx_FillRectangle_NoClip(8 * (roomDrawer % 20), 24 + 32 * floor(roomDrawer / 20) - 16 * (roomDrawer > 19), 8 * roomData[i], 16 + 16 * (roomDrawer > 19 && roomDrawer < 120));
        if (roomNumber == 1 || roomNumber == 2) gfx_FillRectangle_NoClip(8 * (roomDrawer % 20) + 160, 24 + 32 * floor(roomDrawer / 20) - 16 * (roomDrawer > 19), 8 * roomData[i], 16 + 16 * (roomDrawer > 19 && roomDrawer < 120));
        else gfx_FillRectangle(320 - (8 * (roomDrawer % 20)) - 8 * roomData[i], 24 + 32 * floor(roomDrawer / 20) - 16 * (roomDrawer > 19), 8 * roomData[i], 16 + 16 * (roomDrawer > 19 && roomDrawer < 120));
        roomDrawer += roomData[i] + roomData[i + 1];
    }
    gfx_SetColor(2);
    // for those rooms that have a random black bar down the side, draw that black bar down the side
    if (roomNumber == 11 || roomNumber == 22) gfx_FillRectangle_NoClip(16, 24, 4, 192);
    if (roomNumber == 15 || roomNumber == 28) gfx_FillRectangle_NoClip(300, 24, 4, 192);
}

int8_t key(uint8_t currentRoom, int16_t manX, uint8_t manY, int8_t holdingCooldown) {
    for (int8_t i = 0; i < 3; i++) {
        if (currentRoom == objectPos[3 * i]) {
            if (!i) gfx_ScaledTransparentSprite_NoClip(GoldKey, objectPos[1], objectPos[2], 2, 2);
            else if (i == 1) gfx_ScaledTransparentSprite_NoClip(BlackKey, objectPos[4], objectPos[5], 2, 2);
            else if (i == 2) gfx_ScaledTransparentSprite_NoClip(WhiteKey, objectPos[7], objectPos[8], 2, 2);
            if (!holdingCooldown && gfx_CheckRectangleHotspot(manX, manY, 8, 8, objectPos[3 * i + 1], objectPos[3 * i + 2], 16, 6)) {
                playerHeldObject = i;
                holdingCooldown = 15;
                if (kb_Data[7] & kb_Left) objectPos[3 * i + 1] -= 6;
                if (kb_Data[7] & kb_Right) objectPos[3 * i + 1] += 4;
                if (kb_Data[7] & kb_Up) objectPos[3 * i + 2] -= 6;
                if (kb_Data[7] & kb_Down) objectPos[3 * i + 2] += 6;
            }
        }
    }
    return holdingCooldown;
}
