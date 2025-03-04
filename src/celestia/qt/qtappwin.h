// qtappwin.h
//
// Copyright (C) 2007-2008, Celestia Development Team
// celestia-developers@lists.sourceforge.net
//
// Main window for Celestia Qt front-end.
//
// This program is free software; you can redistribute it and/or
// modify it under the terms of the GNU General Public License
// as published by the Free Software Foundation; either version 2
// of the License, or (at your option) any later version.

#ifndef _QTAPPWIN_H_
#define _QTAPPWIN_H_

#include <celestia/celestiacore.h>
#include <QMainWindow>
#include "qttimetoolbar.h"


class QMenu;
class QCloseEvent;
class QDockWidget;
struct CelestiaCommandLineOptions;
class CelestiaGlWidget;
class CelestialBrowser;
class InfoPanel;
class EventFinder;
class CelestiaActions;
class FPSActionGroup;

class PreferencesDialog;
class BookmarkManager;
class BookmarkToolBar;
class QUrl;

class CelestiaAppWindow : public QMainWindow, public CelestiaCore::ContextMenuHandler
{
 Q_OBJECT

 public:
    CelestiaAppWindow(QWidget* parent = nullptr);
    ~CelestiaAppWindow();

    void init(const CelestiaCommandLineOptions&);

    void readSettings();
    void writeSettings();
    bool loadBookmarks();
    void saveBookmarks();

    void requestContextMenu(float x, float y, Selection sel);

    void loadingProgressUpdate(const QString& s);

 public slots:
    void celestia_tick();
    void slotShowSelectionContextMenu(const QPoint& pos, Selection& sel);
    void slotManual();
    void setCheckedFPS();
    void setFPS(int);
    void setCustomFPS();

 private slots:
    void slotGrabImage();
    void slotCaptureVideo();
    void slotCopyImage();
    void slotCopyURL();
    void slotPasteURL();

    void centerSelection();
    void gotoSelection();
    void gotoObject();
    void selectSun();

    void slotPreferences();

    void slotSplitViewVertically();
    void slotSplitViewHorizontally();
    void slotCycleView();
    void slotSingleView();
    void slotDeleteView();
    void slotToggleFramesVisible();
    void slotToggleActiveFrameVisible();
    void slotToggleSyncTime();

    void slotShowObjectInfo(Selection& sel);

    void slotOpenScriptDialog();
    void slotOpenScript();

    void slotShowTimeDialog();

    void slotToggleFullScreen();

    void slotShowAbout();
    void slotShowGLInfo();

    void slotAddBookmark();
    void slotOrganizeBookmarks();
    void slotBookmarkTriggered(const QString& url);

    void handleCelUrl(const QUrl& url);

    void copyText();
    void pasteText();

    void copyTextOrURL();
    void pasteTextOrURL();

 signals:
    void progressUpdate(const QString& s, int align, const QColor& c);

 private:
    void initAppDataDirectory();

    void createActions();
    void createMenus();
    QMenu* buildScriptsMenu();
    void populateBookmarkMenu();

    void closeEvent(QCloseEvent* event);

    void switchToNormal();
    void switchToFullscreen();

 private:
    CelestiaGlWidget* glWidget{ nullptr };
    QDockWidget* toolsDock{ nullptr };
    CelestialBrowser* celestialBrowser{ nullptr };

    CelestiaCore* m_appCore{ nullptr };

    CelestiaActions* actions{ nullptr };

    FPSActionGroup *fpsActions;

    QMenu* fileMenu{ nullptr };
    QMenu* navMenu{ nullptr };
    QMenu* timeMenu{ nullptr };
    QMenu* displayMenu{ nullptr };
    QMenu* bookmarkMenu{ nullptr };
    QMenu* viewMenu{ nullptr };
    QMenu* helpMenu{ nullptr };
    TimeToolBar* timeToolBar{ nullptr };
    QToolBar* guidesToolBar{ nullptr };

    InfoPanel* infoPanel{ nullptr };
    EventFinder* eventFinder{ nullptr };

    CelestiaCore::Alerter* alerter{ nullptr };

    PreferencesDialog* m_preferencesDialog{ nullptr };
    BookmarkManager* m_bookmarkManager{ nullptr };
    BookmarkToolBar* m_bookmarkToolBar{ nullptr };

    QString m_dataDirPath;

    QTimer *timer;
};


#endif // _QTAPPWIN_H_
