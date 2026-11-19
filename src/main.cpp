// TuxBlox Mac - Linux launcher for Roblox's macOS Player
// Copyright (C) 2026 TuxBlox Developers
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.

#include "mac_client.h"

#include <adwaita.h>
#include <gtk/gtk.h>

#include <cerrno>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <filesystem>
#include <functional>
#include <mutex>
#include <string>
#include <thread>
#include <vector>

#include <signal.h>
#include <sys/types.h>
#include <sys/utsname.h>
#include <sys/wait.h>
#include <unistd.h>

namespace fs = std::filesystem;

namespace {

struct State {
    std::mutex mutex;
    std::string installDir;
    std::string version;
    std::string status = "Checking the installed client…";
    std::string busyLabel;
    double progress = -1.0;
    bool installed = false;
    bool busy = false;
    bool running = false;
    pid_t playerPid = -1;
    bool closed = false;
    guint tickId = 0;

    GtkWindow *pWindow = nullptr;
    GtkLabel *pVersion = nullptr;
    GtkLabel *pStatus = nullptr;
    GtkButton *pAction = nullptr;
    GtkProgressBar *pProgress = nullptr;
};

void setStatus(State &state, const std::string &text) {
    std::lock_guard<std::mutex> lock(state.mutex);
    state.status = text;
}

void setBusy(State &state, bool busy, const std::string &label) {
    std::lock_guard<std::mutex> lock(state.mutex);
    state.busy = busy;
    state.busyLabel = label;
}

void refreshInstalled(State &state) {
    tuxblox::macclient::Build build;
    const bool installed = tuxblox::macclient::installedBuild(state.installDir, build);
    std::lock_guard<std::mutex> lock(state.mutex);
    state.installed = installed;
    state.version = installed ? build.version : std::string();
    if (!state.running && !state.busy) {
        state.status = installed ? ("Installed " + build.version) : "Not installed yet";
    }
}

void waitForPlayer(State &state, pid_t pid) {
    int status = 0;
    while (waitpid(pid, &status, 0) < 0 && errno == EINTR) {
    }
    std::lock_guard<std::mutex> lock(state.mutex);
    state.running = false;
    state.playerPid = -1;
    state.busy = false;
    state.busyLabel.clear();
    if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
        state.status = "The Player closed";
    } else if (WIFSIGNALED(status)) {
        state.status = "The Player was stopped";
    } else {
        state.status = "The Player exited";
    }
}

void install(State &state) {
    setBusy(state, true, "Installing…");
    setStatus(state, "Checking the current version…");
    const auto result = tuxblox::macclient::ensureCurrent(
        state.installDir, "live", [&state](uint64_t done, uint64_t total) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.progress = total > 0 ? static_cast<double>(done) / static_cast<double>(total) : -1.0;
            state.status = total > 0 ? "Downloading the Player…" : "Installing the Player…";
        });
    std::lock_guard<std::mutex> lock(state.mutex);
    state.busy = false;
    state.busyLabel.clear();
    state.progress = -1.0;
    if (!result.ok) {
        state.status = "Could not install: " + result.error;
        return;
    }
    state.installed = true;
    state.version = result.build.version;
    state.status = "Installed " + result.build.version;
}

void launch(State &state) {
    setBusy(state, true, "Starting…");
    setStatus(state, "Checking the current version…");
    const auto result = tuxblox::macclient::ensureCurrent(
        state.installDir, "live", [&state](uint64_t done, uint64_t total) {
            std::lock_guard<std::mutex> lock(state.mutex);
            state.progress = total > 0 ? static_cast<double>(done) / static_cast<double>(total) : -1.0;
            state.status = total > 0 ? "Downloading the Player…" : "Preparing the Player…";
        });
    if (!result.ok) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.busy = false;
        state.progress = -1.0;
        state.status = "Could not prepare: " + result.error;
        return;
    }

    const std::string settingsError = tuxblox::macclient::writeAppSettings(result.build, "{}");
    if (!settingsError.empty()) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.busy = false;
        state.progress = -1.0;
        state.status = "Could not write the Player settings: " + settingsError;
        return;
    }

    const std::string rosemary = tuxblox::macclient::resolveRosemary(state.installDir);
    if (rosemary.empty()) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.busy = false;
        state.progress = -1.0;
        state.status = "Rosemary was not found. Set TUXBLOX_ROSEMARY to its path.";
        return;
    }

    const std::vector<std::string> args = tuxblox::macclient::playerLaunchArgs(result.build.exe, "");
    const std::string prefix = (fs::path(state.installDir) / "mac-client" / "prefix").string();
    struct utsname host {};
    const bool armHost = uname(&host) == 0 &&
                         (std::strcmp(host.machine, "aarch64") == 0 ||
                          std::strcmp(host.machine, "arm64") == 0);

    const pid_t pid = fork();
    if (pid < 0) {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.busy = false;
        state.progress = -1.0;
        state.status = "Could not start the Player";
        return;
    }
    if (pid == 0) {
        setpgid(0, 0);
        setenv("ROSEMARY_PREFIX", prefix.c_str(), 1);
        setenv("ROSEMARY_PREFIX_SETUP", "download", 1);
        setenv("ROSEMARY_PROC_TRANSLATED", armHost ? "1" : "0", 1);
        setenv("SDL_APP_ID", "net.tuxblox.TuxBloxMac.Player", 1);

        std::vector<char *> argv;
        argv.push_back(const_cast<char *>(rosemary.c_str()));
        for (const std::string &arg : args) argv.push_back(const_cast<char *>(arg.c_str()));
        argv.push_back(nullptr);
        if (rosemary.find('/') == std::string::npos) {
            execvp(rosemary.c_str(), argv.data());
        } else {
            execv(rosemary.c_str(), argv.data());
        }
        _exit(127);
    }

    setpgid(pid, pid);
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        state.playerPid = pid;
        state.running = true;
        state.busy = false;
        state.busyLabel.clear();
        state.progress = -1.0;
        state.status = "The Player is running";
    }
    std::thread(waitForPlayer, std::ref(state), pid).detach();
}

void stop(State &state) {
    pid_t pid = -1;
    {
        std::lock_guard<std::mutex> lock(state.mutex);
        pid = state.playerPid;
        if (pid > 0) {
            state.busy = true;
            state.busyLabel = "Stopping…";
            state.status = "Stopping the Player…";
        }
    }
    if (pid > 0) {
        if (kill(-pid, SIGTERM) != 0) kill(pid, SIGTERM);
    }
}

void onActionSimple(GtkButton *, gpointer data) {
    auto *state = static_cast<State *>(data);
    bool installed = false;
    bool running = false;
    bool busy = false;
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        installed = state->installed;
        running = state->running;
        busy = state->busy;
    }
    if (busy) return;
    if (running) {
        stop(*state);
        return;
    }
    std::thread(installed ? launch : install, std::ref(*state)).detach();
}

gboolean onTick(gpointer data) {
    auto *state = static_cast<State *>(data);
    std::lock_guard<std::mutex> lock(state->mutex);
    if (state->closed) return G_SOURCE_REMOVE;

    const std::string versionText = state->version.empty() ? "Not installed yet"
                                                           : state->version + "  ·  live";
    gtk_label_set_text(state->pVersion, versionText.c_str());
    gtk_label_set_text(state->pStatus, state->status.c_str());

    if (state->progress >= 0.0) {
        gtk_widget_set_visible(GTK_WIDGET(state->pProgress), TRUE);
        gtk_progress_bar_set_fraction(state->pProgress, state->progress);
    } else {
        gtk_widget_set_visible(GTK_WIDGET(state->pProgress), FALSE);
    }

    const char *pLabel = "Install Player";
    bool sensitive = !state->busy;
    bool suggested = true;
    bool destructive = false;
    if (state->busy) {
        pLabel = state->busyLabel.empty() ? "Working…" : state->busyLabel.c_str();
        suggested = false;
    } else if (state->running) {
        pLabel = "Stop Player";
        suggested = false;
        destructive = true;
    } else if (state->installed) {
        pLabel = "Launch Player";
    }
    gtk_button_set_label(state->pAction, pLabel);
    gtk_widget_set_sensitive(GTK_WIDGET(state->pAction), sensitive);
    if (suggested) {
        gtk_widget_add_css_class(GTK_WIDGET(state->pAction), "suggested-action");
    } else {
        gtk_widget_remove_css_class(GTK_WIDGET(state->pAction), "suggested-action");
    }
    if (destructive) {
        gtk_widget_add_css_class(GTK_WIDGET(state->pAction), "destructive-action");
    } else {
        gtk_widget_remove_css_class(GTK_WIDGET(state->pAction), "destructive-action");
    }
    return G_SOURCE_CONTINUE;
}

void onWindowDestroy(GtkWidget *, gpointer data) {
    auto *state = static_cast<State *>(data);
    std::lock_guard<std::mutex> lock(state->mutex);
    state->closed = true;
    if (state->tickId != 0) {
        g_source_remove(state->tickId);
        state->tickId = 0;
    }
    state->pWindow = nullptr;
}

void onActivate(GtkApplication *pApp, gpointer data) {
    auto *state = static_cast<State *>(data);
    {
        std::lock_guard<std::mutex> lock(state->mutex);
        if (state->pWindow != nullptr) {
            gtk_window_present(state->pWindow);
            return;
        }
    }

    GtkWidget *pWindow = gtk_application_window_new(pApp);
    gtk_window_set_title(GTK_WINDOW(pWindow), "TuxBlox Mac");
    gtk_window_set_default_size(GTK_WINDOW(pWindow), 560, 320);
    gtk_window_set_application(GTK_WINDOW(pWindow), pApp);

    GtkWidget *pHeader = gtk_header_bar_new();
    GtkWidget *pTitle = gtk_label_new("TuxBlox Mac");
    gtk_widget_add_css_class(pTitle, "title");
    gtk_header_bar_set_title_widget(GTK_HEADER_BAR(pHeader), pTitle);
    gtk_window_set_titlebar(GTK_WINDOW(pWindow), pHeader);

    GtkWidget *pBox = gtk_box_new(GTK_ORIENTATION_VERTICAL, 14);
    gtk_widget_set_margin_start(pBox, 28);
    gtk_widget_set_margin_end(pBox, 28);
    gtk_widget_set_margin_top(pBox, 24);
    gtk_widget_set_margin_bottom(pBox, 24);
    gtk_widget_set_valign(pBox, GTK_ALIGN_CENTER);

    GtkWidget *pName = gtk_label_new("Roblox Player");
    gtk_label_set_xalign(GTK_LABEL(pName), 0.0f);
    gtk_widget_add_css_class(pName, "title-1");
    gtk_box_append(GTK_BOX(pBox), pName);

    GtkWidget *pVersion = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(pVersion), 0.0f);
    gtk_widget_add_css_class(pVersion, "dim-label");
    gtk_box_append(GTK_BOX(pBox), pVersion);

    GtkWidget *pStatus = gtk_label_new("");
    gtk_label_set_xalign(GTK_LABEL(pStatus), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(pStatus), TRUE);
    gtk_widget_add_css_class(pStatus, "dim-label");
    gtk_box_append(GTK_BOX(pBox), pStatus);

    GtkWidget *pProgress = gtk_progress_bar_new();
    gtk_widget_set_visible(pProgress, FALSE);
    gtk_box_append(GTK_BOX(pBox), pProgress);

    GtkWidget *pAction = gtk_button_new_with_label("Install Player");
    gtk_widget_add_css_class(pAction, "suggested-action");
    gtk_widget_set_halign(pAction, GTK_ALIGN_START);
    g_signal_connect(pAction, "clicked", G_CALLBACK(onActionSimple), state);
    gtk_box_append(GTK_BOX(pBox), pAction);

    GtkWidget *pFoot = gtk_label_new("Runs the official macOS Player through Rosemary.");
    gtk_label_set_xalign(GTK_LABEL(pFoot), 0.0f);
    gtk_label_set_wrap(GTK_LABEL(pFoot), TRUE);
    gtk_widget_add_css_class(pFoot, "caption");
    gtk_widget_add_css_class(pFoot, "dim-label");
    gtk_box_append(GTK_BOX(pBox), pFoot);

    GtkWidget *pClamp = adw_clamp_new();
    adw_clamp_set_maximum_size(ADW_CLAMP(pClamp), 520);
    adw_clamp_set_child(ADW_CLAMP(pClamp), pBox);
    gtk_window_set_child(GTK_WINDOW(pWindow), pClamp);

    {
        std::lock_guard<std::mutex> lock(state->mutex);
        state->pWindow = GTK_WINDOW(pWindow);
        state->pVersion = GTK_LABEL(pVersion);
        state->pStatus = GTK_LABEL(pStatus);
        state->pAction = GTK_BUTTON(pAction);
        state->pProgress = GTK_PROGRESS_BAR(pProgress);
        state->tickId = g_timeout_add(200, onTick, state);
    }
    g_signal_connect(pWindow, "destroy", G_CALLBACK(onWindowDestroy), state);
    gtk_window_present(GTK_WINDOW(pWindow));

    std::thread([state] {
        refreshInstalled(*state);
    }).detach();
}

std::string defaultInstallDir() {
    const char *pDir = std::getenv("TUXBLOX_MAC_DIR");
    if (pDir != nullptr && *pDir != '\0') return pDir;
    return std::string(g_get_user_data_dir()) + "/tuxblox-mac";
}

} // namespace

int main(int argc, char **argv) {
    State state;
    state.installDir = defaultInstallDir();
    std::error_code ec;
    fs::create_directories(state.installDir, ec);

    if (argc > 1 && std::strcmp(argv[1], "--check") == 0) {
        const auto result = tuxblox::macclient::ensureCurrent(
            state.installDir, "live", [](uint64_t done, uint64_t total) {
                if (total > 0) {
                    std::fprintf(stderr, "\r%llu / %llu bytes",
                                 static_cast<unsigned long long>(done),
                                 static_cast<unsigned long long>(total));
                    std::fflush(stderr);
                }
            });
        if (result.ok) {
            std::printf("ok %s\n", result.build.version.c_str());
            return 0;
        }
        std::fprintf(stderr, "\n%s\n", result.error.c_str());
        return 1;
    }

    AdwApplication *pApp = adw_application_new("net.tuxblox.TuxBloxMac", G_APPLICATION_DEFAULT_FLAGS);
    g_signal_connect(pApp, "activate", G_CALLBACK(onActivate), &state);
    const int status = g_application_run(G_APPLICATION(pApp), argc, argv);
    g_object_unref(pApp);
    return status;
}
