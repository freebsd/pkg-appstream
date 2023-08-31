/*
 * Copyright (c) 2023 Serenity Cyber Security, LLC
 * Author: Gleb Popov <arrowd@FreeBSD.org>
 * SPDX-License-Identifier: BSD-2-Clause
 */

#include <unistd.h>
#include <string.h>
#include <errno.h>

#include <sys/fcntl.h>
#include <sys/stat.h>

#include <pkg.h>

#define APPSTREAM_CATALOG_DIR "/var/lib/swcatalog/xml"

static char plugin_name[] = "appstream";
static char plugin_version[] = "1.0.2.0";
static char plugin_descr[] = "Plugin for downloading AppStream metadata files";

static
int on_update_success_cb(void *data, struct pkgdb *db) {
    int rc;
    time_t local_t = 0;
    struct pkg_repo *repo = data;
    struct timespec times[2] = {0};

    char appstream_catalog_path[PATH_MAX];
    FILE *appstream_catalog_f = NULL;
    int tmp_fd, appstream_catalog_fd;

    tmp_fd = pkg_repo_fetch_remote_tmp(repo,
        "AppStreamComponents", "xml.gz", &local_t, &rc, false);
    if (tmp_fd == -1) {
        pkg_emit_notice("Repository %s has no AppStream metadata", pkg_repo_name(repo));
        return EPKG_FATAL;
    }
    appstream_catalog_f = fdopen(tmp_fd, "r");
    rewind(appstream_catalog_f);

    if (pkg_mkdirs(APPSTREAM_CATALOG_DIR) != EPKG_OK) {
        pkg_emit_error("Could not create AppStream catalog directory " APPSTREAM_CATALOG_DIR);
        return EPKG_FATAL;
    }

    snprintf(appstream_catalog_path, PATH_MAX,
        APPSTREAM_CATALOG_DIR "/%s-Components.xml.gz", pkg_repo_name(repo));
    appstream_catalog_fd = open(appstream_catalog_path, O_CREAT | O_TRUNC | O_WRONLY, 0644);
    if (appstream_catalog_fd == -1) {
        pkg_emit_error("Could not create %s: %s", appstream_catalog_path, strerror(errno));
        return EPKG_FATAL;
    }
    if (!pkg_copy_file(tmp_fd, appstream_catalog_fd)) {
        pkg_emit_error("Could not copy to %s: %s", appstream_catalog_path, strerror(errno));
        return EPKG_FATAL;
    }

    times[0].tv_nsec = UTIME_OMIT;
    times[1].tv_nsec = UTIME_NOW;
    utimensat (AT_FDCWD, APPSTREAM_CATALOG_DIR, times, 0);
    close(appstream_catalog_fd);
    fclose(appstream_catalog_f);

    (void)db;
    return EPKG_OK;
}

int
pkg_plugin_init(struct pkg_plugin *p)
{
    pkg_plugin_set(p, PKG_PLUGIN_NAME, plugin_name);
    pkg_plugin_set(p, PKG_PLUGIN_VERSION, plugin_version);
    pkg_plugin_set(p, PKG_PLUGIN_DESC, plugin_descr);

    pkg_plugin_hook_register(p, PKG_PLUGIN_HOOK_REPO_UPDATE_SUCCESS, on_update_success_cb);

    return (EPKG_OK);
}
