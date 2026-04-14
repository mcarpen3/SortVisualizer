#include <stdio.h>
#include <raylib.h>
#include <raymath.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>
#include <stdint.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>

#define WW 800
#define WH 480 
#define MAG 50 
#define BG 0x800090ff
#define FG 0x008090ff
#define HL 0x800000ff
#define DN 0x008000ff
#define RECT_W (WW / MAG)
#define IDX_TXT_H 10
#define TITLE_TXT_H 40 
#define GUI_TXT_SZ 30

typedef enum {
    UNDEF,
    BUBBLE,
    INSERT,
} SortType;
static const char *options = "Select Method;Bubble;Insertion";
static void GetSelection(SortType *st);

static Color GetColor32(uint32_t iclr);
static void GetRandHeightRects(Rectangle rects[], int length);
static int SortRectsStep(SortType sr, Rectangle rects[], int *idxa, int *idxb);
static int BubbleSortStep(Rectangle rects[], int *idxa, int *idxb);
static int InsertionSortStep(Rectangle rects[], int *idxa, int *idxb);
static Vector2 GetTitle(char *buf, SortType);
static Vector2 GetResultText(char *buf, long end, uint32_t steps);
static Color GetRectColor(int sort, int idx, int sort_idx_a, int sort_idx_b);
static void SwapRects(Rectangle *a, Rectangle *b);

int main(void)
{
    srandom(time(NULL));
    Rectangle rects[MAG] = {0};
    GetRandHeightRects(rects, MAG);
    SetTargetFPS(25);
    InitWindow(WW, WH, "VisualSort");
    char title[16];
    char result[64];
    char buf[16];
    int idxTxtYPos = 0, swap = 0, sort_idx_a, sort_idx_b;
    uint32_t steps = 0;
    Vector2 titlePos, resultPos;
    Color rectColor; 
    SortType stprev = UNDEF;
    SortType st = UNDEF;
    titlePos = GetTitle(title, st);
    GuiSetStyle(DEFAULT, TEXT_SIZE, GUI_TXT_SZ);
    GuiSetStyle(DEFAULT, TEXT_COLOR_NORMAL, 0x000000ff);


    // timer
    time_t begin;
    long end;
    bool done;
    //

    while(!WindowShouldClose())
    {
        BeginDrawing();
        ClearBackground(GetColor32(BG));
        DrawText(title, titlePos.x, titlePos.y, TITLE_TXT_H, WHITE);

        if (st != UNDEF)
        {
            for (int i = 0; i < MAG; ++i)
            {
                sprintf(buf, "%d", (int)rects[i].height);
                rectColor = GetRectColor(swap, i, sort_idx_a, sort_idx_b);
                DrawRectangleRec(rects[i], rectColor);
                Vector2 topLeft = (Vector2) { rects[i].x, rects[i].y };
                Vector2 btmLeft = (Vector2) { rects[i].x, rects[i].y + rects[i].height };
                Vector2 topRight = (Vector2) { rects[i].x + rects[i].width, rects[i].y };
                DrawLineV(topLeft, topRight, BLACK);
                DrawLineV(topLeft, btmLeft, BLACK);
                idxTxtYPos = rects[i].height < IDX_TXT_H ? WH - IDX_TXT_H : rects[i].y;
                DrawText(buf, rects[i].x, idxTxtYPos, IDX_TXT_H, WHITE);
            }
            if (done)
            {
                DrawText(result, resultPos.x, resultPos.y, TITLE_TXT_H, WHITE);
                if (GuiButton((Rectangle){
                        resultPos.x, 
                        resultPos.y + GUI_TXT_SZ + 4, 
                        MeasureText("RESTART", GUI_TXT_SZ) + 4, 
                        GUI_TXT_SZ + 6 
                    }, 
                    "RESTART"))
                {
                    st = UNDEF;
                }
            }

        }
        else
        {
            stprev = st;
            GetSelection(&st);
        }
                    
        EndDrawing();

        if (stprev != st) 
        {
            if (st != UNDEF)
            {
                // reset with chosen sorting method
                GetRandHeightRects(rects, MAG);
                done = false;
                begin = time(NULL);
                swap = 0;
            }
            titlePos = GetTitle(title, st);
        }

        if (st != UNDEF)
        {
            if (swap != 2)
            {
                swap = SortRectsStep(st, rects, &sort_idx_a, &sort_idx_b);
                steps++;
            }
            else if (!done)
            {
                end = (time(NULL) - begin);
                resultPos = GetResultText(result, end, steps);
                done = true;
            }
        }
        stprev = st;
    }
    CloseWindow();
    return 0;
}

static void GetRandHeightRects(Rectangle rects[], int length)
{
    for (int i = 0; i < length; ++i)
    {
        int randHeight = random() % WH;
        rects[i].x = i * RECT_W;
        rects[i].y = WH - randHeight;
        rects[i].width = RECT_W;
        rects[i].height = randHeight;
    }
}

static Color GetColor32(uint32_t iclr)
{
    return (Color) {
        iclr >> 24 & 0xff, 
        iclr >> 16 & 0xff,
        iclr >> 8 & 0xff,
        iclr & 0xff
    };
}

static int SortRectsStep(SortType sr, Rectangle rects[], int *idxa, int *idxb)
{
    switch(sr)
    {
        case INSERT:
            return InsertionSortStep(rects, idxa, idxb);
            break;

        case BUBBLE:
        default:
            return BubbleSortStep(rects, idxa, idxb);
            break;
    }
}

static int BubbleSortStep(Rectangle rects[], int *idxa, int *idxb)
{
    int rc = 0;
    static int a = 0, sorted = MAG - 1;
    if (rects[a].height > rects[a + 1].height)
    {
        SwapRects(&rects[a], &rects[a + 1]);
        rc = 1;
    }
    *idxa = a;
    *idxb = a+1;
    if (a == sorted - 1)
    {
        a = 0;
        sorted--;
    }
    else
    {
        a++;
    }
    if (sorted == 1)
    {
        a = 0; sorted = MAG - 1;
        return 2;
    }
    return rc;
}

static int InsertionSortStep(Rectangle rects[], int *idxa, int *idxb)
{
    static int rc = -1, a = 1, b = 0;
    
    if (a == MAG - 1 && (b < 0 || rc == 0)) 
    {
        a = 1; b = 0; rc = -1;
        return 2;
    }
    else if (b < 0 || rc == 0) 
    {
        a++;
        b = a - 1;
    }


    if (rects[b+1].height < rects[b].height)
    {
        SwapRects(&rects[b+1], &rects[b]);
        *idxa = b;
        *idxb = b+1;
        rc = 1;
    } 
    else
    {
        rc = 0;
    }

    b--;

    return rc;
}

static Vector2 GetTitle(char *buf, SortType st)
{
    switch(st)
    {
        case BUBBLE:
            sprintf(buf, "%s", "BUBBLE");
            break;
        case INSERT:
            sprintf(buf, "%s", "INSERTION");
            break;
        default:
            sprintf(buf, "%s", "SORT VISUALIZER");
            break;
    }

    return (Vector2) { 
        .x = WW / 2 - MeasureText(buf, TITLE_TXT_H) / 2,
        .y = 2 
    };
}

static void GetSelection(SortType *st)
{
    static int active = 0;
    static bool edit = false;
    bool tmpedit = false;
    Rectangle bounds = (Rectangle){WW/3, TITLE_TXT_H + 4,WW/3,WH/8}; 
    tmpedit = GuiDropdownBox(bounds, options, &active, edit);
    if (tmpedit) edit = true; 
    if (active > 0)
    {
        *st = active;
        active = 0;
        edit = false;
    }
}

static Color GetRectColor(int sort, int idx, int sort_idx_a, int sort_idx_b)
{   
    if (sort == 1 && (idx == sort_idx_a || idx == sort_idx_b)) 
    {
        return GetColor32(HL);
    }
    else if (sort == 2)
    {
        return GetColor32(DN);
    }
    else
    {
        return GetColor32(FG);
    }
}
static void SwapRects(Rectangle *a, Rectangle *b)
{
    float ytmp = a->y;
    float htmp = a->height;
    a->y = b->y;
    a->height = b->height;
    b->y = ytmp;
    b->height = htmp;
}

static Vector2 GetResultText(char *buf, long end, uint32_t steps)
{
    sprintf(buf, "time %lds, %u steps", end, steps);
    return (Vector2) { 
        WW / 2 - MeasureText(buf, TITLE_TXT_H) / 2,
        WH / 2 - TITLE_TXT_H / 2
    };
}
