/*
 * SPDX-License-Identifier: GPL-3.0-only
 * MuseScore-Studio-CLA-applies
 *
 * MuseScore Studio
 * Music Composition & Notation
 *
 * Copyright (C) 2021 MuseScore Limited
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 3 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <https://www.gnu.org/licenses/>.
 */

#include <csignal>
#include <cstdio>

#include <QApplication>
#include <QStyleHints>
#include <QQuickWindow>

#include "appfactory.h"
#include "internal/commandlineparser.h"
#include "global/iapplication.h"

#include "muse_framework_config.h"
#include "app_config.h"

#include "log.h"

#ifndef MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT
static void crashCallback(int signum)
{
    const char* signame = "UNKNOWN SIGNAME";
    const char* sigdescript = "";
    switch (signum) {
    case SIGILL:
        signame = "SIGILL";
        sigdescript = "Illegal Instruction";
        break;
    case SIGSEGV:
        signame = "SIGSEGV";
        sigdescript =  "Invalid memory reference";
        break;
    }
    LOGE() << "Oops! Application crashed with signal: [" << signum << "] " << signame << "-" << sigdescript;
    exit(EXIT_FAILURE);
}

#endif

static void app_init_qrc()
{
    Q_INIT_RESOURCE(app);

#ifdef Q_OS_WIN
    Q_INIT_RESOURCE(app_win);
#endif
}

int main(int argc, char** argv)
{
    // DEBUG: Print to verify main() is entered
    fprintf(stdout, "DEBUG: MuseScore main() started\n");
    fflush(stdout);
    
#ifndef MUSE_MODULE_DIAGNOSTICS_CRASHPAD_CLIENT
    signal(SIGSEGV, crashCallback);
    signal(SIGILL, crashCallback);
    signal(SIGFPE, crashCallback);
#endif

    // ====================================================
    // Setup global Qt application variables
    // ====================================================

    fprintf(stdout, "DEBUG: About to init QRC resources\n");
    fflush(stdout);
    app_init_qrc();
    fprintf(stdout, "DEBUG: QRC resources initialized\n");
    fflush(stdout);

    qputenv("QT_STYLE_OVERRIDE", "Fusion");
    qputenv("QML_DISABLE_DISK_CACHE", "true");
    fprintf(stdout, "DEBUG: Qt environment variables set\n");
    fflush(stdout);

    // HACK: Workaround for crash #28840. This disables the incremental GC
    if (!qEnvironmentVariableIsSet("MU_QV4_GC_TIMELIMIT")) {
        qputenv("QV4_GC_TIMELIMIT", "0");
    }

    if (!qEnvironmentVariableIsSet("QT_QUICK_FLICKABLE_WHEEL_DECELERATION")) {
        qputenv("QT_QUICK_FLICKABLE_WHEEL_DECELERATION", "5000");
    }

#ifdef Q_OS_LINUX
    if (qEnvironmentVariable("MU_QT_QPA_PLATFORM") != "offscreen") {
        qputenv("QT_QPA_PLATFORMTHEME", "gtk3");
    }

    //! NOTE Forced X11, with Wayland there are a number of problems now
    if (qEnvironmentVariable("MU_QT_QPA_PLATFORM") == "") {
        qputenv("QT_QPA_PLATFORM", "xcb");
    }
#endif

#ifdef Q_OS_WIN
    // NOTE: There are some problems with rendering the application window on some integrated graphics processors
    //       see https://github.com/musescore/MuseScore/issues/8270
    if (!qEnvironmentVariableIsSet("QT_OPENGL_BUGLIST")) {
        qputenv("QT_OPENGL_BUGLIST", ":/resources/win_opengl_buglist.json");
    }
#endif

    fprintf(stdout, "DEBUG: About to set style hints\n");
    fflush(stdout);
    QGuiApplication::styleHints()->setMousePressAndHoldInterval(250);
    fprintf(stdout, "DEBUG: Style hints set\n");
    fflush(stdout);

#ifdef MUSE_APP_UNSTABLE
    QCoreApplication::setApplicationName(MUSE_APP_NAME_MACHINE_READABLE MUSE_APP_VERSION_MAJOR "Development");
#else
    QCoreApplication::setApplicationName(MUSE_APP_NAME_MACHINE_READABLE MUSE_APP_VERSION_MAJOR);
#endif
    QCoreApplication::setOrganizationName("MuseScore");
    QCoreApplication::setOrganizationDomain("musescore.org");
    QCoreApplication::setApplicationVersion(MUSE_APP_VERSION);

#if !defined(Q_OS_WIN) && !defined(Q_OS_DARWIN) && !defined(Q_OS_WASM)
    // Any OS that uses Freedesktop.org Desktop Entry Specification (e.g. Linux, BSD)
#ifndef MUSE_APP_INSTALL_SUFFIX
#define MUSE_APP_INSTALL_SUFFIX ""
#endif
    QGuiApplication::setDesktopFileName("org.musescore.MuseScore" MUSE_APP_INSTALL_SUFFIX);
#endif

    using namespace muse;
    using namespace mu::app;

    // ====================================================
    // Parse command line options
    // ====================================================
#ifdef MUE_ENABLE_CONSOLEAPP
    CommandLineParser commandLineParser;
    commandLineParser.init();
    commandLineParser.parse(argc, argv);

    IApplication::RunMode runMode = commandLineParser.runMode();
    QCoreApplication* qapp = nullptr;

    if (runMode == IApplication::RunMode::AudioPluginRegistration) {
        qapp = new QCoreApplication(argc, argv);
    } else {
        qapp = new QApplication(argc, argv);
    }

    commandLineParser.processBuiltinArgs(*qapp);
    CmdOptions opt = commandLineParser.options();

#else
    fprintf(stdout, "DEBUG: MUE_ENABLE_CONSOLEAPP not defined, using default GuiApp mode\n");
    fflush(stdout);
    fprintf(stdout, "DEBUG: Creating QApplication instance\n");
    fflush(stdout);
    QCoreApplication* qapp = new QApplication(argc, argv);
    fprintf(stdout, "DEBUG: QApplication created successfully\n");
    fflush(stdout);
    CmdOptions opt;
    opt.runMode = IApplication::RunMode::GuiApp;
#endif

    fprintf(stdout, "DEBUG: About to setup application event loop\n");
    fflush(stdout);

    // ====================================================
    // Setup application
    // ====================================================

    //! NOTE: We immediately launch the application's event loop
    // to be able to show a splash screen (on Linux, splash screen won't show without event loop).
    // All subsequent initialization steps will be executed as events in the event loop.

    std::shared_ptr<muse::IApplication> app;
    fprintf(stdout, "DEBUG: About to invoke AppFactory creation\n");
    fflush(stdout);
    QMetaObject::invokeMethod(qapp, [qapp, &app, &opt]() {
        fprintf(stdout, "DEBUG: Inside AppFactory lambda\n");
        fflush(stdout);
        AppFactory f;
        fprintf(stdout, "DEBUG: AppFactory created\n");
        fflush(stdout);
        app = f.newApp(opt);
        fprintf(stdout, "DEBUG: newApp() returned\n");
        fflush(stdout);
        try {
            fprintf(stdout, "DEBUG: About to call showSplash()\n");
            fflush(stdout);
            // TEMPORARILY COMMENTED OUT
            // app->showSplash();
            fprintf(stdout, "DEBUG: showSplash() skipped for testing\n");
            fflush(stdout);
        } catch (const std::exception& e) {
            fprintf(stdout, "DEBUG: Exception in showSplash: %s\n", e.what());
            fflush(stdout);
            return 1;
        }
        fprintf(stdout, "DEBUG: After showSplash try/catch\n");
        fflush(stdout);
        QMetaObject::invokeMethod(qapp, [qapp, &app]() {
            fprintf(stdout, "DEBUG: In second lambda - app->setup()\n");
            fflush(stdout);
            app->setup();
            fprintf(stdout, "DEBUG: app->setup() completed\n");
            fflush(stdout);
            QMetaObject::invokeMethod(qapp, [qapp, &app]() {
                fprintf(stdout, "DEBUG: In third lambda - app->showContextSplash()\n");
                fflush(stdout);
                app->showContextSplash();
                fprintf(stdout, "DEBUG: app->showContextSplash() completed\n");
                fflush(stdout);
                QMetaObject::invokeMethod(qapp, [&app]() {
                    fprintf(stdout, "DEBUG: In fourth lambda - app->setupNewContext()\n");
                    fflush(stdout);
                    app->setupNewContext();
                    fprintf(stdout, "DEBUG: app->setupNewContext() completed\n");
                    fflush(stdout);
                }, Qt::QueuedConnection);
            }, Qt::QueuedConnection);
        }, Qt::QueuedConnection);
    }, Qt::QueuedConnection);

    // ====================================================
    // Run main loop
    // ====================================================
    fprintf(stdout, "DEBUG: About to enter event loop with qapp->exec()\n");
    fflush(stdout);
    int code = qapp->exec();
    fprintf(stdout, "DEBUG: Event loop exited with code: %d\n", code);
    fflush(stdout);

    // ====================================================
    // Quit
    // ====================================================

    fprintf(stdout, "DEBUG: Cleaning up\n");
    fflush(stdout);
    if (app) {
        app->finish();
    }

    delete qapp;

    fprintf(stdout, "DEBUG: MuseScore main() closing normally\n");
    fflush(stdout);

    LOGI() << "Goodbye!! code: " << code;
    return code;
}
