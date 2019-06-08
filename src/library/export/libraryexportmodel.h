#ifndef LIBRARYEXPORTMODEL_H
#define LIBRARYEXPORTMODEL_H

#include <QList>
#include <QString>
#include "library/crate/crateid.h"

struct LibraryExportModel {
    static const QString EngineLibraryDirName;
    static const QString MixxxExportDirName;

    QString engineLibraryDir;
    QString musicFilesDir;
    bool exportEntireMusicLibrary;
    QList<CrateId> selectedCrates;

    void clear() {
        engineLibraryDir.clear();
        musicFilesDir.clear();
        exportEntireMusicLibrary = false;
        selectedCrates.clear();
    }
};

#endif // LIBRARYEXPORTMODEL_H
