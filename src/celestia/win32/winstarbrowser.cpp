// winstarbrowser.cpp
//
// Copyright (C) 2001, Chris Laurel <claurel@shatters.net>
//
// Star browser tool for Windows.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#include <string>
#include <algorithm>
#include <set>
#include <windows.h>
#include <commctrl.h>
#include <cstring>
#include <celutil/gettext.h>
#include <celutil/winutil.h>
#include "winuiutils.h"
#include <celmath/mathlib.h>
#include "winstarbrowser.h"
#include "res/resource.h"
#include "winuiutils.h"

using namespace Eigen;
using namespace std;
using namespace celestia::win32;

static const int MinListStars = 10;
static const int MaxListStars = 500;
static const int DefaultListStars = 100;


// TODO: More of the functions in this module should be converted to
// methods of the StarBrowser class.

enum {
    BrightestStars = 0,
    NearestStars = 1,
    StarsWithPlanets = 2,
};


bool InitStarBrowserColumns(HWND listView)
{
    LVCOLUMN lvc;
    LVCOLUMN columns[5];

    lvc.mask = LVCF_FMT | LVCF_WIDTH | LVCF_TEXT | LVCF_SUBITEM;
    lvc.fmt = LVCFMT_LEFT;
    lvc.cx = DpToPixels(60, listView);
    lvc.pszText = const_cast<char*>("");

    int nColumns = sizeof(columns) / sizeof(columns[0]);
    int i;

    for (i = 0; i < nColumns; i++)
        columns[i] = lvc;

    string header0 = UTF8ToCurrentCP(_("Name"));
    string header1 = UTF8ToCurrentCP(_("Distance (ly)"));
    string header2 = UTF8ToCurrentCP(_("App. mag"));
    string header3 = UTF8ToCurrentCP(_("Abs. mag"));
    string header4 = UTF8ToCurrentCP(_("Type"));

    columns[0].pszText = const_cast<char*>(header0.c_str());
    columns[0].cx = DpToPixels(100, listView);
    columns[1].pszText = const_cast<char*>(header1.c_str());
    columns[1].fmt = LVCFMT_RIGHT;
    columns[1].cx = DpToPixels(115, listView);
    columns[2].pszText = const_cast<char*>(header2.c_str());
    columns[2].fmt = LVCFMT_RIGHT;
    columns[2].cx = DpToPixels(65, listView);
    columns[3].pszText = const_cast<char*>(header3.c_str());
    columns[3].fmt = LVCFMT_RIGHT;
    columns[3].cx = DpToPixels(65, listView);
    columns[4].pszText = const_cast<char*>(header4.c_str());

    for (i = 0; i < nColumns; i++)
    {
        columns[i].iSubItem = i;
        if (ListView_InsertColumn(listView, i, &columns[i]) == -1)
            return false;
    }

    return true;
}


struct CloserStarPredicate
{
    Vector3f pos;
    bool operator()(const Star* star0, const Star* star1) const
    {
        return ((pos - star0->getPosition()).squaredNorm() <
                (pos - star1->getPosition()).squaredNorm());

    }
};

struct BrighterStarPredicate
{
    Vector3f pos;
    UniversalCoord ucPos;
    bool operator()(const Star* star0, const Star* star1) const
    {
        float d0 = (pos - star0->getPosition()).norm();
        float d1 = (pos - star1->getPosition()).norm();

        // If the stars are closer than one light year, use
        // a more precise distance estimate.
        if (d0 < 1.0f)
            d0 = ucPos.offsetFromLy(star0->getPosition()).norm();
        if (d1 < 1.0f)
            d1 = ucPos.offsetFromLy(star1->getPosition()).norm();

        return (star0->getApparentMagnitude(d0) <
                star1->getApparentMagnitude(d1));
    }
};

struct SolarSystemPredicate
{
    Vector3f pos;
    SolarSystemCatalog* solarSystems;

    bool operator()(const Star* star0, const Star* star1) const
    {
        SolarSystemCatalog::iterator iter;

        iter = solarSystems->find(star0->getIndex());
        bool hasPlanets0 = (iter != solarSystems->end());
        iter = solarSystems->find(star1->getIndex());
        bool hasPlanets1 = (iter != solarSystems->end());
        if (hasPlanets1 == hasPlanets0)
        {
            return ((pos - star0->getPosition()).squaredNorm() <
                    (pos - star1->getPosition()).squaredNorm());
        }
        else
        {
            return hasPlanets0;
        }
    }
};


// Find the nearest/brightest/X-est N stars in a database.  The
// supplied predicate determines which of two stars is a better match.
template<class Pred> vector<const Star*>*
FindStars(const StarDatabase& stardb, Pred pred, int nStars)
{
    vector<const Star*>* finalStars = new vector<const Star*>();
    if (nStars == 0)
        return finalStars;

    typedef multiset<const Star*, Pred> StarSet;
    StarSet firstStars(pred);

    int totalStars = stardb.size();
    if (totalStars < nStars)
        nStars = totalStars;

    // We'll need at least nStars in the set, so first fill
    // up the list indiscriminately.
    int i = 0;
    for (i = 0; i < nStars; i++)
    {
        Star* star = stardb.getStar(i);
        if (star->getVisibility())
            firstStars.insert(star);
    }

    // From here on, only add a star to the set if it's
    // a better match than the worst matching star already
    // in the set.
    const Star* lastStar = *--firstStars.end();
    for (; i < totalStars; i++)
    {
        Star* star = stardb.getStar(i);
        if (star->getVisibility() && pred(star, lastStar))
        {
            firstStars.insert(star);
            firstStars.erase(--firstStars.end());
            lastStar = *--firstStars.end();
        }
    }

    // Move the best matching stars into the vector
    finalStars->reserve(nStars);
    for (const auto& star : firstStars)
    {
        finalStars->insert(finalStars->end(), star);
    }

    return finalStars;
}


bool InitStarBrowserLVItems(HWND listView, vector<const Star*>& stars)
{
    LVITEM lvi;

    lvi.mask = LVIF_TEXT | LVIF_PARAM | LVIF_STATE;
    lvi.state = 0;
    lvi.stateMask = 0;
    lvi.pszText = LPSTR_TEXTCALLBACK;

    for (unsigned int i = 0; i < stars.size(); i++)
    {
        lvi.iItem = i;
        lvi.iSubItem = 0;
        lvi.lParam = (LPARAM) stars[i];
        ListView_InsertItem(listView, &lvi);
    }

    return true;
}


bool InitStarBrowserItems(HWND listView, StarBrowser* browser)
{
    Universe* univ = browser->appCore->getSimulation()->getUniverse();
    StarDatabase* stardb = univ->getStarCatalog();
    SolarSystemCatalog* solarSystems = univ->getSolarSystemCatalog();

    vector<const Star*>* stars = NULL;
    switch (browser->predicate)
    {
    case BrightestStars:
        {
            BrighterStarPredicate brighterPred;
            brighterPred.pos = browser->pos;
            brighterPred.ucPos = browser->ucPos;
            stars = FindStars(*stardb, brighterPred, browser->nStars);
        }
        break;

    case NearestStars:
        {
            CloserStarPredicate closerPred;
            closerPred.pos = browser->pos;
            stars = FindStars(*stardb, closerPred, browser->nStars);
        }
        break;

    case StarsWithPlanets:
        {
            if (solarSystems == NULL)
                return false;
            SolarSystemPredicate solarSysPred;
            solarSysPred.pos = browser->pos;
            solarSysPred.solarSystems = solarSystems;
            stars = FindStars(*stardb, solarSysPred,
                              min((unsigned int) browser->nStars, (unsigned int) solarSystems->size()));
        }
        break;

    default:
        return false;
    }

    bool succeeded = InitStarBrowserLVItems(listView, *stars);
    delete stars;

    return succeeded;
}


// Crud used for the list item display callbacks
static string starNameString("");
static char callbackScratch[256];

struct StarBrowserSortInfo
{
    int subItem;
    Vector3f pos;
    UniversalCoord ucPos;
};

int CALLBACK StarBrowserCompareFunc(LPARAM lParam0, LPARAM lParam1,
                                    LPARAM lParamSort)
{
    StarBrowserSortInfo* sortInfo = reinterpret_cast<StarBrowserSortInfo*>(lParamSort);
    Star* star0 = reinterpret_cast<Star*>(lParam0);
    Star* star1 = reinterpret_cast<Star*>(lParam1);

    switch (sortInfo->subItem)
    {
    case 0:
        return 0;

    case 1:
        {
            float d0 = (sortInfo->pos - star0->getPosition()).norm();
            float d1 = (sortInfo->pos - star1->getPosition()).norm();
            return (int) celmath::sign(d0 - d1);
        }

    case 2:
        {
            float d0 = (sortInfo->pos - star0->getPosition()).norm();
            float d1 = (sortInfo->pos - star1->getPosition()).norm();
            if (d0 < 1.0f)
                d0 = sortInfo->ucPos.offsetFromLy(star0->getPosition()).norm();
            if (d1 < 1.0f)
                d1 = sortInfo->ucPos.offsetFromLy(star1->getPosition()).norm();
            return (int) celmath::sign(star0->getApparentMagnitude(d0) -
                                       star1->getApparentMagnitude(d1));
        }

    case 3:
        return (int) celmath::sign(star0->getAbsoluteMagnitude() - star1->getAbsoluteMagnitude());

    case 4:
        return strcmp(star0->getSpectralType(), star1->getSpectralType());

    default:
        return 0;
    }
}


void StarBrowserDisplayItem(LPNMLVDISPINFOA nm, StarBrowser* browser)
{
    double tdb = browser->appCore->getSimulation()->getTime();

    Star* star = reinterpret_cast<Star*>(nm->item.lParam);
    if (star == NULL)
    {
        nm->item.pszText = const_cast<char*>("");
        return;
    }

    switch (nm->item.iSubItem)
    {
    case 0:
        {
            Universe* u = browser->appCore->getSimulation()->getUniverse();
            starNameString = UTF8ToCurrentCP(u->getStarCatalog()->getStarName(*star));
            nm->item.pszText = const_cast<char*>(starNameString.c_str());
        }
        break;

    case 1:
        {
            Vector3d r = star->getPosition(tdb).offsetFromKm(browser->ucPos);
            sprintf(callbackScratch, "%.4g", astro::kilometersToLightYears(r.norm()));
            nm->item.pszText = callbackScratch;
        }
        break;

    case 2:
        {
            Vector3d r = star->getPosition(tdb).offsetFromKm(browser->ucPos);
            float appMag = star->getApparentMagnitude(astro::kilometersToLightYears(r.norm()));
            sprintf(callbackScratch, "%.2f", appMag);
            nm->item.pszText = callbackScratch;
        }
        break;

    case 3:
        sprintf(callbackScratch, "%.2f", star->getAbsoluteMagnitude());
        nm->item.pszText = callbackScratch;
        break;

    case 4:
        strncpy(callbackScratch, star->getSpectralType(),
                sizeof(callbackScratch));
        callbackScratch[sizeof(callbackScratch) - 1] = '\0';
        nm->item.pszText = callbackScratch;
        break;
    }
}

void RefreshItems(HWND hDlg, StarBrowser* browser)
{
    SetMouseCursor(IDC_WAIT);

    Simulation* sim = browser->appCore->getSimulation();
    browser->ucPos = sim->getObserver().getPosition();
    browser->pos = browser->ucPos.toLy().cast<float>();
    HWND hwnd = GetDlgItem(hDlg, IDC_STARBROWSER_LIST);
    if (hwnd != 0)
    {
        ListView_DeleteAllItems(hwnd);
        InitStarBrowserItems(hwnd, browser);
    }

    SetMouseCursor(IDC_ARROW);
}

BOOL APIENTRY StarBrowserProc(HWND hDlg,
                              UINT message,
                              WPARAM wParam,
                              LPARAM lParam)
{
    StarBrowser* browser = reinterpret_cast<StarBrowser*>(GetWindowLongPtr(hDlg, DWLP_USER));

    switch (message)
    {
    case WM_INITDIALOG:
        {
            StarBrowser* browser = reinterpret_cast<StarBrowser*>(lParam);
            if (browser == NULL)
                return EndDialog(hDlg, 0);
            SetWindowLongPtr(hDlg, DWLP_USER, lParam);

            HWND hwnd = GetDlgItem(hDlg, IDC_STARBROWSER_LIST);
            InitStarBrowserColumns(hwnd);
            InitStarBrowserItems(hwnd, browser);
            CheckRadioButton(hDlg, IDC_RADIO_NEAREST, IDC_RADIO_WITHPLANETS, IDC_RADIO_NEAREST);

            //Initialize Max Stars edit box
            char val[16];
            hwnd = GetDlgItem(hDlg, IDC_MAXSTARS_EDIT);
            sprintf(val, "%d", DefaultListStars);
            SetWindowText(hwnd, val);
            SendMessage(hwnd, EM_LIMITTEXT, 3, 0);

            //Initialize Max Stars Slider control
            SendDlgItemMessage(hDlg, IDC_MAXSTARS_SLIDER, TBM_SETRANGE,
                (WPARAM)TRUE, (LPARAM)MAKELONG(MinListStars, MaxListStars));
            SendDlgItemMessage(hDlg, IDC_MAXSTARS_SLIDER, TBM_SETPOS,
                (WPARAM)TRUE, (LPARAM)DefaultListStars);

            return(TRUE);
        }

    case WM_DESTROY:
        if (browser != NULL && browser->parent != NULL)
        {
            SendMessage(browser->parent, WM_COMMAND, IDCLOSE,
                        reinterpret_cast<LPARAM>(browser));
        }
        break;

    case WM_COMMAND:
        switch (LOWORD(wParam))
        {
        case IDOK:
        case IDCANCEL:
            if (browser != NULL && browser->parent != NULL)
            {
                SendMessage(browser->parent, WM_COMMAND, IDCLOSE,
                            reinterpret_cast<LPARAM>(browser));
            }
            EndDialog(hDlg, 0);
            return TRUE;

        case IDC_BUTTON_CENTER:
            browser->appCore->charEntered('c');
            break;

        case IDC_BUTTON_GOTO:
            browser->appCore->charEntered('G');
            break;

        case IDC_RADIO_BRIGHTEST:
            browser->predicate = BrightestStars;
            RefreshItems(hDlg, browser);
            break;

        case IDC_RADIO_NEAREST:
            browser->predicate = NearestStars;
            RefreshItems(hDlg, browser);
            break;

        case IDC_RADIO_WITHPLANETS:
            browser->predicate = StarsWithPlanets;
            RefreshItems(hDlg, browser);
            break;

        case IDC_BUTTON_REFRESH:
            RefreshItems(hDlg, browser);
            break;

        case IDC_MAXSTARS_EDIT:
            // TODO: browser != NULL check should be in a lot more places
            if (HIWORD(wParam) == EN_KILLFOCUS && browser != NULL)
            {
                char val[16];
                DWORD nNewStars;
                DWORD minRange, maxRange;
                GetWindowText((HWND) lParam, val, sizeof(val));
                nNewStars = atoi(val);

                // Check if new value is different from old. Don't want to
                // cause a refresh to occur if not necessary.
                if (nNewStars != browser->nStars)
                {
                    minRange = SendDlgItemMessage(hDlg, IDC_MAXSTARS_SLIDER, TBM_GETRANGEMIN, 0, 0);
                    maxRange = SendDlgItemMessage(hDlg, IDC_MAXSTARS_SLIDER, TBM_GETRANGEMAX, 0, 0);
                    if (nNewStars < minRange)
                        nNewStars = minRange;
                    else if (nNewStars > maxRange)
                        nNewStars = maxRange;

                    // If new value has been adjusted from what was entered,
                    // reflect new value back in edit control.
                    if (atoi(val) != nNewStars)
                    {
                        sprintf(val, "%d", nNewStars);
                        SetWindowText((HWND)lParam, val);
                    }

                    // Recheck value if different from original.
                    if (nNewStars != browser->nStars)
                    {
                        browser->nStars = nNewStars;
                        SendDlgItemMessage(hDlg,
                                           IDC_MAXSTARS_SLIDER,
                                           TBM_SETPOS,
                                           (WPARAM) TRUE,
                                           (LPARAM) browser->nStars);
                        RefreshItems(hDlg, browser);
                    }
                }
            }
            break;
        }
        break;

    case WM_NOTIFY:
        {
            LPNMHDR hdr = (LPNMHDR) lParam;

            if (hdr->idFrom == IDC_STARBROWSER_LIST && browser != NULL)
            {
                switch(hdr->code)
                {
                case LVN_GETDISPINFO:
                    StarBrowserDisplayItem((LPNMLVDISPINFOA) lParam, browser);
                    break;
                case LVN_ITEMCHANGED:
                    {
                        LPNMLISTVIEW nm = (LPNMLISTVIEW) lParam;
                        if ((nm->uNewState & LVIS_SELECTED) != 0)
                        {
                            Simulation* sim = browser->appCore->getSimulation();
                            Star* star = reinterpret_cast<Star*>(nm->lParam);
                            if (star != NULL)
                                sim->setSelection(Selection(star));
                        }
                        break;
                    }
                case LVN_COLUMNCLICK:
                    {
                        HWND hwnd = GetDlgItem(hDlg, IDC_STARBROWSER_LIST);
                        if (hwnd != 0)
                        {
                            LPNMLISTVIEW nm = (LPNMLISTVIEW) lParam;
                            StarBrowserSortInfo sortInfo;
                            sortInfo.subItem = nm->iSubItem;
                            sortInfo.ucPos = browser->ucPos;
                            sortInfo.pos = browser->pos;
                            ListView_SortItems(hwnd, StarBrowserCompareFunc,
                                               reinterpret_cast<LPARAM>(&sortInfo));
                        }
                    }

                }
            }
        }
        break;

    case WM_HSCROLL:
        {
            WORD sbValue = LOWORD(wParam);
            switch(sbValue)
            {
                case SB_THUMBTRACK:
                {
                    char val[16];
                    HWND hwnd = GetDlgItem(hDlg, IDC_MAXSTARS_EDIT);
                    sprintf(val, "%d", HIWORD(wParam));
                    SetWindowText(hwnd, val);
                    break;
                }
                case SB_THUMBPOSITION:
                {
                    browser->nStars = (int)HIWORD(wParam);
                    RefreshItems(hDlg, browser);
                    break;
                }
            }
        }
        break;
    }

    return FALSE;
}


StarBrowser::StarBrowser(HINSTANCE appInstance,
                         HWND _parent,
                         CelestiaCore* _appCore) :
    appCore(_appCore),
    parent(_parent)
{
    ucPos = appCore->getSimulation()->getObserver().getPosition();
    pos = ucPos.toLy().cast<float>();

    predicate = NearestStars;
    nStars = DefaultListStars;

    hwnd = CreateDialogParam(appInstance,
                             MAKEINTRESOURCE(IDD_STARBROWSER),
                             parent,
                             (DLGPROC)StarBrowserProc,
                             reinterpret_cast<LONG_PTR>(this));
}


StarBrowser::~StarBrowser()
{
    SetWindowLongPtr(hwnd, DWLP_USER, 0);
}
