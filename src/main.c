/*
 * risk_ce.c
 * Risk Battle Calculator for TI-84 Plus CE
 */

#include <graphx.h>
#include <keypadc.h>
#include <tice.h>
#include <stdio.h>
#include <stdbool.h>
#include <ti/getcsc.h>

#define BG       0   // black
#define TXT      192 // light gray
#define ATK_COL  224 // red
#define DEF_COL  56  // cyan
#define HLT_COL  128 // highlight gray
#define BTN_COL  160 // dark red
#define PIP_COL  255 // white

#define MAX_DICE   3
#define DICE_SZ    40
#define W          320
#define H          240

typedef enum { SCR_MAIN, SCR_RESULTS, SCR_HELP } Screen;

static void drawDice(int x, int y, int v, int c, bool sel);
static void drawMain(int atk[], int ac, int def[], int dc, int cur);
static void drawResults(int al, int dl);
static void drawHelp(void);
static void sortDesc(int *d, int n);
static void calcBattle(int *a, int ac, int *d, int dc, int *al, int *dl);

int main(void) {
    gfx_Begin();
    gfx_SetTransparentColor(BG);
    gfx_SetDrawBuffer();
    kb_SetMode(MODE_3_CONTINUOUS);

    int atk[MAX_DICE] = {1,1,1}, def[MAX_DICE] = {1,1};
    int ac = 3, dc = 2;
    int cur = 0, al = 0, dl = 0;
    Screen scr = SCR_MAIN;
    bool redraw = true, done = false;

    uint8_t prevKb[8] = {0}, currKb[8], pressed[8];

    while (!done) {
        kb_Scan();
        for (int i = 0; i < 8; i++) {
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
                int tot = ac + dc;
                cur = (cur > 0 ? cur - 1 : tot - 1);
                redraw = true;
            }
            else if (pressed[7] & kb_Right) {
                int tot = ac + dc;
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
                int *arr = (cur < ac ? atk : def);
                int idx = (cur < ac ? cur : cur - ac);
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

static void drawMain(int atk[], int ac, int def[], int dc, int cur) {
    char buf[32];
    gfx_SetTextScale(2,2);
    gfx_SetTextFGColor(TXT);
    gfx_SetTextBGColor(BG);
    gfx_PrintStringXY("RISK BATTLE", 90,20);

    // Attacker
    gfx_SetTextScale(1,1);
    gfx_SetTextFGColor(ATK_COL);
    sprintf(buf,"ATTACKER (%d)",ac);
    gfx_PrintStringXY(buf,10,70);
    for (int i=0; i<ac; i++)
        drawDice(20+i*60, 90, atk[i], ATK_COL, cur==i);

    // Defender
    gfx_SetTextFGColor(DEF_COL);
    sprintf(buf,"DEFENDER (%d)",dc);
    gfx_PrintStringXY(buf,10,150);
    for (int i=0; i<dc; i++)
        drawDice(20+i*60,170, def[i], DEF_COL, cur==(ac+i));

    gfx_SetTextScale(1,1);
    gfx_SetTextFGColor(TXT);
    gfx_SetTextBGColor(BG);
    gfx_PrintStringXY("ALPHA Help", 5, H-8);
    gfx_PrintStringXY("ENTER Calc", 240, H-8);
}


static void drawResults(int al, int dl) {
    char buf[40];
    gfx_SetTextScale(2,2);
    gfx_SetTextFGColor(TXT);
    gfx_SetTextBGColor(BG);
    gfx_PrintStringXY("RESULTS",100,40);

    gfx_SetTextScale(1,1);
    gfx_SetTextFGColor(ATK_COL);
    sprintf(buf,"Attacker loses %d",al);
    gfx_PrintStringXY(buf,80,100);

    gfx_SetTextFGColor(DEF_COL);
    sprintf(buf,"Defender loses %d",dl);
    gfx_PrintStringXY(buf,80,120);

    gfx_SetTextFGColor(TXT);
    gfx_PrintStringXY("ENTER to cont.",80,180);
}

static void drawHelp(void) {
    gfx_SetTextScale(2,2);
    gfx_SetTextFGColor(TXT);
    gfx_SetTextBGColor(BG);
    gfx_PrintStringXY("HELP",130,10);

    gfx_SetTextScale(1,1);
    gfx_PrintStringXY("1-6: set die",10,50);
    gfx_PrintStringXY("<>: select die",10,60);
    gfx_PrintStringXY("^: attacker #",10,70);
    gfx_PrintStringXY("v: defender #",10,80);
    gfx_PrintStringXY("ENTER: calc",10,90); // ENTER (G6)
    gfx_PrintStringXY("ALPHA: show help",10,100); // ALPHA (G2)
    gfx_PrintStringXY("CLEAR: exit",10,110); // CLEAR (G6)
    gfx_PrintStringXY("ENTER: back",10,200); // ENTER (G6)
}

static void drawDice(int x, int y, int v, int c, bool sel) {
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
        int cx=x+DICE_SZ/2, cy=y+DICE_SZ/2;
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

static void sortDesc(int *d, int n) {
    for (int i = 0; i < n - 1; i++) {
        for (int j = i + 1; j < n; j++) {
            if (d[j] > d[i]) {
                int t = d[i]; d[i] = d[j]; d[j] = t;
            }
        }
    }
}

static void calcBattle(int *a, int ac, int *d, int dc, int *al, int *dl) {
    sortDesc(a, ac);
    sortDesc(d, dc);
    *al = 0; *dl = 0;
    for (int i = 0; i < (ac < dc ? ac : dc); i++) { // Iterate up to the minimum number of dice on either side
        if (a[i] > d[i]) (*dl)++;
        else (*al)++;
    }
}
