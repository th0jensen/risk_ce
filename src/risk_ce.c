/*
 * risk_ce.c
 * Risk Battle Calculator for TI-84 Plus CE
 */

#include "../inc/uint24.h"
#include <graphx.h>
#include <keypadc.h>
#include <tice.h>
#include <stdio.h>
#include <stdbool.h>
#include <ti/getcsc.h>

#define BG       255
#define TXT      0
#define ATK_COL  224
#define DEF_COL  56
#define HLT_COL  128
#define BTN_COL  160
#define PIP_COL  0

#define MAX_DICE   3
#define DICE_SZ    40
#define W          320
#define H          240

typedef enum { SCR_MAIN, SCR_RESULTS, SCR_HELP } Screen;

static void drawDice(uint24_t x, uint24_t y, uint24_t v, uint24_t c, bool sel);
static void drawMain(uint24_t atk[], uint24_t ac, uint24_t def[], uint24_t dc, uint24_t cur);
static void drawResults(uint24_t al, uint24_t dl);
static void drawHelp(void);
static void sortDesc(uint24_t *d, uint24_t n);
static void calcBattle(uint24_t *a, uint24_t ac, uint24_t *d, uint24_t dc, uint24_t *al, uint24_t *dl);

int main(void) {
    gfx_Begin();
    gfx_SetTransparentColor(BG);
    gfx_SetDrawBuffer();
    kb_SetMode(MODE_3_CONTINUOUS);

    uint24_t atk[MAX_DICE] = {1,1,1}, def[MAX_DICE] = {1,1};
    uint24_t ac = 3, dc = 2;
    uint24_t cur = 0, al = 0, dl = 0;
    Screen scr = SCR_MAIN;
    bool redraw = true, done = false;

    uint24_t prevKb[8] = {0}, currKb[8], pressed[8];

    while (!done) {
        kb_Scan();
        for (uint24_t i = 0; i < 8; i++) {
            currKb[i] = kb_Data[i];
            pressed[i] = currKb[i] & ~prevKb[i];
            prevKb[i] = currKb[i];
        }

        if (scr == SCR_MAIN) {
            // CLEAR: Group 6, Bit 6
            if (pressed[6] & kb_Clear) {
                done = true;
            }
            // CALC: ENTER (Group 6, Bit 0)
            else if ((pressed[6] & kb_Enter)) {
                calcBattle(atk, ac, def, dc, &al, &dl);
                scr = SCR_RESULTS; redraw = true;
            }
            // HELP: ALPHA (Group 2, Bit 7)
            else if ((pressed[2] & kb_Alpha)) {
                scr = SCR_HELP; redraw = true;
            }
            // Arrow keys: Group 7
            else if (pressed[7] & kb_Left) {
                uint24_t tot = ac + dc;
                cur = (cur > 0 ? cur - 1 : tot - 1);
                redraw = true;
            }
            else if (pressed[7] & kb_Right) {
                uint24_t tot = ac + dc;
                cur = (cur < tot - 1 ? cur + 1 : 0);
                redraw = true;
            }
            else if (pressed[7] & kb_Up) {
                ac = ac % 3 + 1;
                if (cur >= ac + dc) cur = 0;
                redraw = true;
            }
            else if (pressed[7] & kb_Down) {
                dc = dc % 2 + 1;
                if (cur >= ac + dc) cur = 0;
                redraw = true;
            }
            // Number keys 1–6:
            // 1, 4 in Group 3
            // 2, 5 in Group 4
            // 3, 6 in Group 5
            // 1, 2, 3 are Bit 1
            // 4, 5, 6 are Bit 2
            else {
                uint24_t *arr = (cur < ac ? atk : def);
                uint24_t idx = (cur < ac ? cur : cur - ac);
                bool num_pressed = false;

                if (pressed[3] & kb_1) { arr[idx] = 1; num_pressed = true; }
                else if (pressed[4] & kb_2) { arr[idx] = 2; num_pressed = true; }
                else if (pressed[5] & kb_3) { arr[idx] = 3; num_pressed = true; }
                else if (pressed[3] & kb_4) { arr[idx] = 4; num_pressed = true; }
                else if (pressed[4] & kb_5) { arr[idx] = 5; num_pressed = true; }
                else if (pressed[5] & kb_6) { arr[idx] = 6; num_pressed = true; }

                if (num_pressed) {
                    redraw = true;
                }
            }
        }
        else if (scr == SCR_RESULTS) {
            // CLEAR: Group 6, Bit 6
            if (pressed[6] & kb_Clear) done = true;
            // ENTER: Group 6, Bit 0
            else if (pressed[6] & kb_Enter) { scr = SCR_MAIN; redraw = true; }
             // HELP: ALPHA (Group 2, Bit 7)
            else if ((pressed[2] & kb_Alpha)) {
                scr = SCR_HELP; redraw = true;
            }
        }
        else {  // SCR_HELP
            // CLEAR: Group 6, Bit 6
            if (pressed[6] & kb_Clear) done = true;
            // ENTER: Group 6, Bit 0
            else if (pressed[6] & kb_Enter) { scr = SCR_MAIN; redraw = true; }
        }

        if (redraw) {
            gfx_FillScreen(BG);
            switch (scr) {
                case SCR_MAIN:    drawMain(atk, ac, def, dc, cur); break;
                case SCR_RESULTS: drawResults(al, dl);             break;
                case SCR_HELP:    drawHelp();                      break;
             }
            gfx_SwapDraw();
            redraw = false;
        }
    }

    gfx_End();
    return 0;
}

static uint24_t centered_x(const char * label) { return (W - gfx_GetStringWidth(label)) / 2;  }

static void drawTitle(const char * label) {
    gfx_SetTextScale(2,2);
    gfx_SetTextFGColor(TXT);
    gfx_SetTextBGColor(BG);
    gfx_PrintStringXY(label, centered_x(label), 20);
}

static void drawMain(uint24_t atk[], uint24_t ac, uint24_t def[], uint24_t dc, uint24_t cur) {
    char buf[32];
    drawTitle("RISK BATTLE");

    // Attacker
    gfx_SetTextScale(1,1);
    gfx_SetTextFGColor(ATK_COL);
    sprintf(buf,"ATTACKER (%d)",ac);
    gfx_PrintStringXY(buf,10,70);
    for (uint24_t i=0; i<ac; i++)
        drawDice(20+i*60, 90, atk[i], ATK_COL, cur==i);

    // Defender
    gfx_SetTextFGColor(DEF_COL);
    sprintf(buf,"DEFENDER (%d)",dc);
    gfx_PrintStringXY(buf,10,150);
    for (uint24_t i=0; i<dc; i++)
        drawDice(20+i*60,170, def[i], DEF_COL, cur==(ac+i));

    gfx_SetTextScale(1,1);
    gfx_SetTextFGColor(TXT);
    gfx_SetTextBGColor(BG);
    gfx_PrintStringXY("ALPHA Help", 5, H-8);
    gfx_PrintStringXY("ENTER Calc", 240, H-8);
}


static void drawResults(uint24_t al, uint24_t dl) {
    char buf[40];
    drawTitle("RESULT");

    gfx_SetTextScale(1,1);
    gfx_SetTextFGColor(ATK_COL);
    sprintf(buf,"Attacker loses %d",al);
    gfx_PrintStringXY(buf,centered_x(buf),70);

    gfx_SetTextFGColor(DEF_COL);
    sprintf(buf,"Defender loses %d",dl);
    gfx_PrintStringXY(buf,centered_x(buf),100);

    gfx_SetTextFGColor(TXT);
    const char * label = "ENTER to return";
    gfx_PrintStringXY(label,centered_x(label),150);
}

static void drawHelp(void) {
    drawTitle("HELP");

    gfx_SetTextScale(1,1);
    char* labels[] = {
        "1-6: set die",
        "<>: select die",
        "^: attacker #",
        "v: defender #",
        "ENTER: calc",
        "ALPHA: show help",
        "CLEAR: exit"
    };

    uint24_t height = 70;
    for (uint24_t i = 0; i < sizeof(labels) / sizeof(labels[0]); i++) {
        gfx_PrintStringXY(labels[i],10,height);
        height += 10;
    }

    gfx_PrintStringXY("ENTER Back", 5, H-8);
}

static void drawDice(uint24_t x, uint24_t y, uint24_t v, uint24_t c, bool sel) {
    if (x<0||x+DICE_SZ>W||y<0||y+DICE_SZ>H) return;
    if (sel) {
        gfx_SetColor(HLT_COL);
        gfx_FillRectangle(x-4,y-4,DICE_SZ+8,DICE_SZ+8);
    }
    gfx_SetColor(c);
    gfx_FillRectangle(x,y,DICE_SZ,DICE_SZ);
    gfx_SetColor(PIP_COL);
    gfx_Rectangle(x,y,DICE_SZ,DICE_SZ);

    if (v>0) {
        uint24_t cx=x+DICE_SZ/2, cy=y+DICE_SZ/2;
        if (v%2) gfx_FillCircle(cx,cy,4); // Center pip for 1, 3, 5
        if (v>=2) { // Top-left, bottom-right for 2, 3, 4, 5, 6
            gfx_FillCircle(x+10,y+10,4);
            gfx_FillCircle(x+DICE_SZ-10,y+DICE_SZ-10,4);
        }
        if (v>=4) { // Top-right, bottom-left for 4, 5, 6
            gfx_FillCircle(x+DICE_SZ-10,y+10,4);
            gfx_FillCircle(x+10,y+DICE_SZ-10,4);
        }
        if (v==6) { // Middle-left, middle-right for 6
            gfx_FillCircle(x+10,cy,4);
            gfx_FillCircle(x+DICE_SZ-10,cy,4);
        }
    }
}

static void sortDesc(uint24_t *d, uint24_t n) {
    for (uint24_t i = 0; i < n - 1; i++) {
        for (uint24_t j = i + 1; j < n; j++) {
            if (d[j] > d[i]) {
                int t = d[i]; d[i] = d[j]; d[j] = t;
            }
        }
    }
}

static void calcBattle(uint24_t *a, uint24_t ac, uint24_t *d, uint24_t dc, uint24_t *al, uint24_t *dl) {
    sortDesc(a, ac);
    sortDesc(d, dc);
    *al = 0; *dl = 0;
    for (uint24_t i = 0; i < (ac < dc ? ac : dc); i++) { // Iterate up to the minimum number of dice on either side
        if (a[i] > d[i]) (*dl)++;
        else (*al)++;
    }
}
