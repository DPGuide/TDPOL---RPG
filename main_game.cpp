// <--- SECTION: SYSTEM_HEADERS --->
#include <windows.h>
#include <mmsystem.h>
#include <string>
#include <map>
#include <fstream>
#include <vector>
// <--- END SECTION --->

#pragma comment(lib,"winmm.lib")

// <--- SECTION: SCHNEIDER_BRIDGE --->
typedef BOOL(WINAPI* T_pB)(HDC,_43,_43,_43,_43,HDC,_43,_43,_43,_43,UINT);
// <--- END SECTION --->

T_pB pB = NULL;

struct Hero {
    _44 active;
    _90 name[32];
    _43 x, y, rID, cID;
    _43 sS, sD, sI, sW, sC;
    _43 hp, hpMax;
};

struct Enemy {
    _43 type, hp, hpMax, dmg;
};

struct Sprite {
    HBITMAP hBmp;
    _43 w, h;
};

enum GameState { S_INTRO, S_MENU, S_CHAR, S_CREATE, S_MAP, S_SETTINGS, S_CONTROLS, S_BATTLE };
GameState state = S_INTRO;
Hero heroes[10];
Enemy curEnemy; // Der aktuelle Gegner
_43 curHeroIdx = 0, crRace = 0, crClass = 0, crStep = 0, menuIdx = 0, setIdx = 0, batIdx = 0;
_44 setMus = _128, setSnd = _128;
_87 battleLog = "A WILD ENEMY APPEARS!"; // Kampf-Nachricht
_87 nameInput = "HERO";
std::map<_43, Sprite> sprites;
_87 RACES[5] = { "HUMAN", "ELF", "DARK ELF", "DWARF", "ORC" };
_87 CLASSES[5] = { "WARRIOR", "MAGE", "ROGUE", "CLERIC", "BARD" };
_87 BATTLE_CMD[4] = { "ATTACK", "MAGIC", "HEAL", "RUN" };

// <--- SECTION: FUNC_Save --->
_50 Save() {
    std::ofstream o("save_v1475.dat", std::ios::binary);
    _15(o.is_open()) {
        o.write((_90*)heroes, sizeof(heroes));
        o.close();
    }
}
// <--- END SECTION --->


// <--- SECTION: FUNC_Load --->
_50 Load() {
    std::ifstream i("save_v1475.dat", std::ios::binary);
    _15(i.is_open()) {
        i.read((_90*)heroes, sizeof(heroes));
        i.close();
    }
}
// <--- END SECTION --->


// <--- SECTION: FUNC_LoadSprite --->
_50 LoadSprite(_43 id, _87 path) {
    HBITMAP h = (HBITMAP)LoadImageA(NULL, path.c_str(), IMAGE_BITMAP, 0, 0, LR_LOADFROMFILE);
    _15(h) {
        BITMAP b;
        GetObject(h, sizeof(BITMAP), &b);
        sprites[id] = { h, b.bmWidth, b.bmHeight };
    }
}
// <--- END SECTION --->


// <--- SECTION: FUNC_DrawSpriteTrans --->
_50 DrawSpriteTrans(HDC hdc, _43 id, _43 x, _43 y) {
    _15(sprites.count(id)) {
        Sprite& s = sprites[id];
        HDC mdc = CreateCompatibleDC(hdc);
        SelectObject(mdc, s.hBmp);
        _15(pB) pB(hdc, x, y, s.w, s.h, mdc, 0, 0, s.w, s.h, RGB(0,0,0));
        DeleteDC(mdc);
    }
}
// <--- END SECTION --->


// <--- SECTION: FUNC_DrawBox --->
_50 DrawBox(HDC hdc, _43 x, _43 y, _43 w, _43 h, _87 txt, COLORREF c, _44 fill) {
    HBRUSH b = CreateSolidBrush(fill ? RGB(100, 90, 80) : RGB(25, 25, 35));
    HPEN p = CreatePen(PS_SOLID, 2, c);
    SelectObject(hdc, b);
    SelectObject(hdc, p);
    Rectangle(hdc, x, y, x + w, y + h);
    SetTextColor(hdc, c);
    SetBkMode(hdc, 1);
    TextOutA(hdc, x + 10, y + 12, txt.c_str(), (_43)txt.length());
    DeleteObject(b);
    DeleteObject(p);
}
// <--- END SECTION --->


// <--- SECTION: FUNC_initGame --->
_50 initGame(HWND h) {
    HMODULE hM = LoadLibraryA("msimg32.dll");
    _15(hM) pB = (T_pB)GetProcAddress(hM, "TransparentBlt");
    
    mciSendStringA("close all", NULL, 0, 0);
    mciSendStringA("open \"intro_01.wmv\" type MPEGVideo alias vid1 style popup", NULL, 0, 0);
    mciSendStringA("play vid1 notify", NULL, 0, (HWND)h);
    PlaySoundA("intro_music.wav", NULL, SND_ASYNC | SND_LOOP);
    PlaySoundA("intro_sound.wav", NULL, SND_ASYNC);
    mciSendStringA("open \"intro_sound.wav\" type waveaudio alias snd1", NULL, 0, 0);
    
    Load();
    LoadSprite(1, "pl_human.bmp");
    LoadSprite(100, "map_1.bmp");
    LoadSprite(101, "splash_title.bmp");
    LoadSprite(20, "camp_human.bmp");
    LoadSprite(21, "camp_elf.bmp");
    LoadSprite(22, "camp_darkelf.bmp");
    LoadSprite(23, "camp_dwarf.bmp");
    LoadSprite(24, "camp_orc.bmp");
    
    SetTimer(h, 1, 16, NULL);
}
// <--- END SECTION --->


// <--- SECTION: FUNC_StartBattle --->
_50 StartBattle() {
    state = S_BATTLE;
    curEnemy.type = 24; // ORC
    curEnemy.hpMax = 50;
    curEnemy.hp = 50;
    curEnemy.dmg = 5;
    battleLog = "A WILD ORC APPEARS!";
}
// <--- END SECTION --->


// <--- SECTION: FUNC_HandleKeys --->
_50 HandleKeys(HWND h, WPARAM w) {
    _15(w EQ VK_ESCAPE) {
        _15(state EQ S_CONTROLS OR state EQ S_BATTLE) state = S_SETTINGS;
        else _15(state EQ S_SETTINGS) state = S_MENU;
        else _15(state > S_MENU) state = S_MENU;
        else {
            mciSendStringA("close all", 0, 0, 0);
            PostQuitMessage(0);
        }
    }
    
    _15(state EQ S_INTRO AND (w EQ VK_SPACE OR w EQ VK_RETURN)) {
        mciSendStringA("close vid1", 0, 0, 0);
        ShowWindow(h, SW_MAXIMIZE);
        state = S_MENU;
    }
    else _15(state EQ S_MENU) {
        _15(w EQ VK_UP) menuIdx = (menuIdx + 3) % 4;
        _15(w EQ VK_DOWN) menuIdx = (menuIdx + 1) % 4;
        _15(w EQ VK_RETURN) {
            _15(menuIdx < 2) state = S_CHAR;
            else _15(menuIdx EQ 2) state = S_SETTINGS;
            else PostQuitMessage(0);
        }
    }
    else _15(state EQ S_SETTINGS) {
        _15(w EQ VK_UP) setIdx = (setIdx + 3) % 4;
        _15(w EQ VK_DOWN) setIdx = (setIdx + 1) % 4;
        _15(w EQ VK_RETURN) {
            _15(setIdx EQ 0) {
                setMus = !setMus;
                _15(setMus) PlaySoundA("intro_music.wav", NULL, SND_ASYNC | SND_LOOP);
                else PlaySoundA(NULL, 0, 0);
            }
            _15(setIdx EQ 1) setSnd = !setSnd;
            _15(setIdx EQ 2) state = S_CONTROLS;
            _15(setIdx EQ 3) state = S_MENU;
        }
    }
    else _15(state EQ S_CHAR) {
        _15(w EQ VK_UP) curHeroIdx = (curHeroIdx + 9) % 10;
        _15(w EQ VK_DOWN) curHeroIdx = (curHeroIdx + 1) % 10;
        _15(w EQ VK_RETURN) {
            _15(heroes[curHeroIdx].active) state = S_MAP;
            else {
                state = S_CREATE;
                crStep = 0;
                nameInput = "";
            }
        }
    }
    else _15(state EQ S_CREATE) {
        _15(crStep EQ 0) {
            _15(w EQ VK_UP) crRace = (crRace + 4) % 5;
            _15(w EQ VK_DOWN) crRace = (crRace + 1) % 5;
            _15(w EQ VK_RETURN) crStep = 1;
        }
        else _15(crStep EQ 1) {
            _15(w EQ VK_UP) crClass = (crClass + 4) % 5;
            _15(w EQ VK_DOWN) crClass = (crClass + 1) % 5;
            _15(w EQ VK_RETURN) crStep = 2;
            _15(w EQ VK_BACK) crStep = 0;
        }
        else _15(crStep EQ 2) {
            _15(w EQ VK_BACK AND nameInput.length() > 0) nameInput.pop_back();
            else _15(w >= 32 AND w <= 126 AND nameInput.length() < 15) nameInput += (_90)w;
            _15(w EQ VK_RETURN AND nameInput.length() > 0) {
                Hero& he = heroes[curHeroIdx];
                he.active = _128;
                he.x = 7;
                he.y = 7;
                he.rID = crRace;
                he.cID = crClass;
                strcpy(he.name, nameInput.c_str());
                he.sS = 10+crRace+(crClass EQ 0?5:0);
                he.sD = 10+(crClass EQ 2?5:0);
                he.sI = 10+(crClass EQ 1?5:0);
                he.sW = 10+(crClass EQ 3?5:0);
                he.sC = 10+(crRace EQ 4?3:0);
                he.hp = he.sC * 10;
                he.hpMax = he.hp;
                Save();
                state = S_MAP;
            }
        }
    }
    else _15(state EQ S_BATTLE) {
        _15(w EQ VK_LEFT) batIdx = (batIdx + 3) % 4;
        _15(w EQ VK_RIGHT) batIdx = (batIdx + 1) % 4;
        _15(w EQ VK_RETURN) {
            Hero& h = heroes[curHeroIdx];
            _15(batIdx EQ 0) { // ATTACK
                _43 dmg = (h.sS * 3) / 2;
                curEnemy.hp -= dmg;
                battleLog = "HIT FOR " + std::to_string(dmg) + " DMG!";
                _15(curEnemy.hp <= 0) {
                    battleLog = "VICTORY!";
                    state = S_MAP;
                }
            }
            _15(batIdx EQ 2) { // HEAL
                h.hp += h.sW * 2;
                _15(h.hp > h.hpMax) h.hp = h.hpMax;
                battleLog = "HEALED!";
            }
            _15(batIdx EQ 3) { // RUN
                _15(h.sD > 10) {
                    battleLog = "ESCAPED!";
                    state = S_MAP;
                }
                else battleLog = "FAILED TO RUN!";
            }
        }
    }
    else _15(state EQ S_MAP) {
        Hero& p = heroes[curHeroIdx];
        _15(w EQ VK_LEFT) p.x -= 1;
        _15(w EQ VK_RIGHT) p.x += 1;
        _15(w EQ VK_UP) p.y -= 1;
        _15(w EQ VK_DOWN) p.y += 1;
        // BATTLE TRIGGER (TEST KEY 'B')
        _15(w EQ 'B') StartBattle();
    }
}
// <--- END SECTION --->


// <--- SECTION: FUNC_GameProc --->
LRESULT CALLBACK GameProc(HWND h, UINT m, WPARAM w, LPARAM l) {
    _15(m EQ WM_DESTROY) {
        mciSendStringA("close all", 0, 0, 0);
        PostQuitMessage(0);
        _96 0;
    }
    _15(m EQ WM_KEYDOWN) HandleKeys(h, w);
    _15(m EQ WM_TIMER) InvalidateRect(h, NULL, _86);
    _15(m EQ MM_MCINOTIFY AND state EQ S_INTRO AND w EQ MCI_NOTIFY_SUCCESSFUL) {
        mciSendStringA("close vid1", 0, 0, 0);
        ShowWindow(h, SW_MAXIMIZE);
        state = S_MENU;
    }
    _15(m EQ WM_PAINT) {
        PAINTSTRUCT ps;
        HDC hdc = BeginPaint(h, &ps);
        _15(state EQ S_INTRO) {
            EndPaint(h, &ps);
            _96 0;
        }
        RECT r;
        GetClientRect(h, &r);
        _15(r.right < 10) {
            EndPaint(h, &ps);
            _96 0;
        }
        HDC mdc = CreateCompatibleDC(hdc);
        HBITMAP mb = CreateCompatibleBitmap(hdc, r.right, r.bottom);
        SelectObject(mdc, mb);
        
        HBRUSH bg = CreateSolidBrush(RGB(10, 15, 30));
        FillRect(mdc, &r, bg);
        DeleteObject(bg);
        
        HFONT f = CreateFontA(24,0,0,0,700,0,0,0,0,0,0,0,0,"Arial");
        SelectObject(mdc, f);
        SetTextColor(mdc, RGB(255, 215, 0));
        SetBkMode(mdc, 1);
        
        _15(state EQ S_MENU) {
            _15(sprites.count(101)) {
                HDC sdc = CreateCompatibleDC(mdc);
                SelectObject(sdc, sprites[101].hBmp);
                StretchBlt(mdc,0,0,r.right,r.bottom,sdc,0,0,sprites[101].w,sprites[101].h,SRCCOPY);
                DeleteDC(sdc);
            }
            _87 b[4] = {"PLAY", "NEW", "SETTINGS", "EXIT"};
            _17(_43 i=0; i<4; i++) DrawBox(mdc, r.right/2-100, r.bottom/2-100+(i*65), 200, 50, b[i], RGB(255,215,0), menuIdx EQ i);
        }
        else _15(state EQ S_SETTINGS) {
            _87 s[4] = {"MUSIC: " + _87(setMus?"ON":"OFF"), "SOUND: " + _87(setSnd?"ON":"OFF"), "CONTROLS", "BACK"};
            _17(_43 i=0; i<4; i++) DrawBox(mdc, 100, 150+(i*60), 300, 50, s[i], RGB(255,215,0), setIdx EQ i);
        }
        else _15(state EQ S_CONTROLS) {
            TextOutA(mdc, 100, 100, "ARROWS = MOVE", 13);
            TextOutA(mdc, 100, 150, "ENTER = SELECT", 14);
            TextOutA(mdc, 100, 200, "ESC = BACK", 10);
        }
        else _15(state EQ S_CHAR) {
            TextOutA(mdc, 50, 50, "SELECT HERO", 11);
            _17(_43 i=0; i<10; i++) DrawBox(mdc, 50, 100+(i*55), 350, 45, heroes[i].active ? heroes[i].name : "[ EMPTY SLOT ]", RGB(255,215,0), curHeroIdx EQ i);
        }
        else _15(state EQ S_CREATE) {
            _17(_43 i=0; i<5; i++) DrawBox(mdc, 50, 100+(i*55), 180, 45, RACES[i], RGB(255,215,0), (crRace EQ i AND crStep EQ 0) OR (crStep > 0 AND crRace EQ i));
            _17(_43 i=0; i<5; i++) DrawBox(mdc, 240, 100+(i*55), 180, 45, CLASSES[i], RGB(255,215,0), (crClass EQ i AND crStep EQ 1) OR (crStep > 1 AND crClass EQ i));
            _15(sprites.count(20+crRace)) {
                Sprite& s = sprites[20+crRace];
                DrawSpriteTrans(mdc, 20+crRace, (r.right-s.w)/2, (r.bottom-s.h)/2);
            }
            _43 sS = 10+crRace+(crClass EQ 0?5:0);
            _87 sT = "STR: " + std::to_string(sS);
            TextOutA(mdc, r.right-250, 150, sT.c_str(), (_43)sT.length());
            
            _43 sD = 10+(crClass EQ 2?5:0);
            _87 sD_t = "DEX: " + std::to_string(sD);
            TextOutA(mdc, r.right-250, 200, sD_t.c_str(), (_43)sD_t.length());
            
            _43 sI = 10+(crClass EQ 1?5:0);
            _87 sI_t = "INT: " + std::to_string(sI);
            TextOutA(mdc, r.right-250, 250, sI_t.c_str(), (_43)sI_t.length());
            
            _43 sW = 10+(crClass EQ 3?5:0);
            _87 sW_t = "WIS: " + std::to_string(sW);
            TextOutA(mdc, r.right-250, 300, sW_t.c_str(), (_43)sW_t.length());
            
            _43 sC = 10+(crRace EQ 4?3:0);
            _87 sC_t = "CON: " + std::to_string(sC);
            TextOutA(mdc, r.right-250, 350, sC_t.c_str(), (_43)sC_t.length());
            
            DrawBox(mdc, r.right/2-150, 40, 300, 50, "NAME: " + nameInput + (crStep EQ 2 ? "_":""), RGB(255,215,0), crStep EQ 2);
        }
        else _15(state EQ S_BATTLE) {
            TextOutA(mdc, r.right/2 - 100, 50, battleLog.c_str(), (_43)battleLog.length());
            _17(_43 i=0; i<4; i++) DrawBox(mdc, r.right/2 - 320 + (i*160), r.bottom - 100, 150, 50, BATTLE_CMD[i], RGB(255,215,0), batIdx EQ i);
            DrawSpriteTrans(mdc, curEnemy.type, (r.right-100)/2, r.bottom/2 - 100);
            
            _87 eHP = "ENEMY HP: " + std::to_string(curEnemy.hp);
            TextOutA(mdc, r.right/2 - 50, r.bottom/2 + 50, eHP.c_str(), (_43)eHP.length());
            
            _87 hHP = "HERO HP: " + std::to_string(heroes[curHeroIdx].hp);
            TextOutA(mdc, 50, r.bottom - 50, hHP.c_str(), (_43)hHP.length());
        }
        else _15(state EQ S_MAP) {
            _15(sprites.count(100)) {
                HDC sdc = CreateCompatibleDC(mdc);
                SelectObject(sdc, sprites[100].hBmp);
                StretchBlt(mdc,0,0,r.right,r.bottom,sdc,0,0,sprites[100].w,sprites[100].h,SRCCOPY);
                DeleteDC(sdc);
            }
            _43 tSz = r.bottom/15;
            DrawSpriteTrans(mdc, 1, (r.right-tSz*15)/2 + heroes[curHeroIdx].x*tSz, heroes[curHeroIdx].y*tSz);
        }
        
        BitBlt(hdc, 0, 0, r.right, r.bottom, mdc, 0, 0, SRCCOPY);
        DeleteObject(mb);
        DeleteDC(mdc);
        DeleteObject(f);
        EndPaint(h, &ps);
    }
    _96 DefWindowProcA(h, m, w, l);
}
// <--- END SECTION --->


// <--- SECTION: FUNC_WinMain --->
_43 WINAPI WinMain(HINSTANCE i, HINSTANCE p, LPSTR c, _43 s) {
    WNDCLASSA wc = {0};
    wc.lpfnWndProc = GameProc;
    wc.hInstance = i;
    wc.lpszClassName = "RPG";
    RegisterClassA(&wc);
    
    HWND h = CreateWindowA("RPG", "Dark Path to Light", WS_OVERLAPPEDWINDOW | WS_VISIBLE | WS_MAXIMIZE, 0, 0, 1280, 720, NULL, NULL, i, NULL);
    
    initGame(h);
    MSG msg;
    _16(GetMessageA(&msg, NULL, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }
    _96 0;
}
// <--- END SECTION --->

