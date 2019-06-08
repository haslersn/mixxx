#include "library/export/dlglibraryexport.h"

#include <QStandardPaths>

#include "library/trackcollection.h"

DlgLibraryExport::DlgLibraryExport(
        QWidget *parent, UserSettingsPointer pConfig, TrackCollection &trackCollection)
        : QDialog(parent), m_pConfig{pConfig}, m_trackCollection{trackCollection} {
    m_pCratesList = make_parented<QListWidget>();
    m_pCratesList->setSelectionMode(QListWidget::ExtendedSelection);
    { // Populate list of crates.
        auto crates = m_trackCollection.crates().selectCrates();
        Crate crate;
        while (crates.populateNext(&crate)) {
            auto pItem = std::make_unique<QListWidgetItem>(crate.getName());
            pItem->setData(Qt::UserRole, crate.getId().value());
            m_pCratesList->addItem(pItem.release());
        }
    }

    m_pExportDirTextField = make_parented<QLineEdit>();
    m_pExportDirTextField->setReadOnly(true);
    m_pEngineLibraryDirTextField = make_parented<QLineEdit>();
    m_pEngineLibraryDirTextField->setReadOnly(true);
    m_pMusicFilesDirTextField = make_parented<QLineEdit>();
    m_pMusicFilesDirTextField->setReadOnly(true);

    auto pWholeLibraryRadio = make_parented<QRadioButton>(tr("Entire music library"));
    pWholeLibraryRadio->setChecked(true);
    this->exportWholeLibrarySelected();
    connect(pWholeLibraryRadio,
            &QRadioButton::clicked,
            this,
            &DlgLibraryExport::exportWholeLibrarySelected);

    auto pCratesRadio = make_parented<QRadioButton>(tr("Selected crates"));
    connect(pCratesRadio,
            &QRadioButton::clicked,
            this,
            &DlgLibraryExport::exportSelectedCratedSelected);

    auto pTrackAnalysisNoteField = make_parented<QLabel>(
            tr("Note: all affected music files will be scheduled for "
               "analysis before they can be exported.  This can take some "
               "time if there are many tracks requiring analysis in the music "
               "library or selected crates."));
    pTrackAnalysisNoteField->setWordWrap(true);

    auto pExportDirBrowseButton = make_parented<QPushButton>(tr("Browse"));
    connect(pExportDirBrowseButton,
            &QPushButton::clicked,
            this,
            &DlgLibraryExport::browseExportDirectory);

    auto pExportDirLayout = make_parented<QHBoxLayout>();
    pExportDirLayout->addWidget(m_pExportDirTextField);
    pExportDirLayout->addWidget(pExportDirBrowseButton);

    auto pFormLayout = make_parented<QFormLayout>();
    pFormLayout->addRow(tr("Base export directory"), pExportDirLayout);
    pFormLayout->addRow(tr("Engine Library export directory"), m_pEngineLibraryDirTextField);
    pFormLayout->addRow(tr("Copy music files to"), m_pMusicFilesDirTextField);
    pFormLayout->addRow(pTrackAnalysisNoteField);

    auto pExportButton = make_parented<QPushButton>(tr("Export"));
    pExportButton->setDefault(true);
    connect(pExportButton, &QPushButton::clicked, this, &DlgLibraryExport::exportRequested);

    auto pCancelButton = make_parented<QPushButton>(tr("Cancel"));
    connect(pCancelButton, &QPushButton::clicked, this, &QDialog::reject);

    auto pButtonBarLayout = make_parented<QHBoxLayout>();
    pButtonBarLayout->addStretch(1);
    pButtonBarLayout->addWidget(pExportButton);
    pButtonBarLayout->addWidget(pCancelButton);

    auto pLayout = make_parented<QGridLayout>();
    pLayout->setColumnStretch(0, 1);
    pLayout->setColumnStretch(1, 2);
    pLayout->addWidget(pWholeLibraryRadio, 0, 0);
    pLayout->addWidget(pCratesRadio, 1, 0);
    pLayout->addWidget(m_pCratesList, 2, 0);
    pLayout->addLayout(pFormLayout, 0, 1, 3, 1);
    pLayout->addLayout(pButtonBarLayout, 3, 0, 1, 2);

    setLayout(pLayout);
    setWindowTitle(tr("Export Library"));

    show();
    raise();
    activateWindow();
}

void DlgLibraryExport::exportWholeLibrarySelected() {
    m_pCratesList->setEnabled(false);
    m_model.exportEntireMusicLibrary = true;
}

void DlgLibraryExport::exportSelectedCratedSelected() {
    m_pCratesList->setEnabled(true);
    m_model.exportEntireMusicLibrary = false;
}

void DlgLibraryExport::browseExportDirectory() {
    QString lastExportDirectory =
            m_pConfig->getValue(ConfigKey("[Library]", "LastLibraryExportDirectory"),
                    QStandardPaths::writableLocation(QStandardPaths::DocumentsLocation));
    auto baseExportDirectoryStr =
            QFileDialog::getExistingDirectory(NULL, tr("Export Library To"), lastExportDirectory);
    if (baseExportDirectoryStr.isEmpty()) {
        return;
    }
    m_pConfig->set(ConfigKey("[Library]", "LastLibraryExportDirectory"),
            ConfigValue(baseExportDirectoryStr));

    QDir baseExportDirectory{baseExportDirectoryStr};
    m_model.engineLibraryDir =
            baseExportDirectory.filePath(LibraryExportModel::EngineLibraryDirName);
    m_model.musicFilesDir = baseExportDirectory.filePath(LibraryExportModel::MixxxExportDirName);

    m_pExportDirTextField->setText(baseExportDirectoryStr);
    m_pEngineLibraryDirTextField->setText(m_model.engineLibraryDir);
    m_pMusicFilesDirTextField->setText(m_model.musicFilesDir);
}

void DlgLibraryExport::exportRequested() {
    // Check a base export directory has been chosen
    if (m_pExportDirTextField->text().isEmpty()) {
        QMessageBox::information(this,
                tr("No Export Directory Chosen"),
                tr("No export directory was chosen.  Please choose a directory "
                   "in order to export the music library."),
                QMessageBox::Ok,
                QMessageBox::Ok);
        return;
    }

    for (auto *pItem : m_pCratesList->selectedItems()) {
        QVariant variant = pItem->data(Qt::UserRole);
        CrateId id{variant.value<int>()};
        m_model.selectedCrates.append(id);
    }

    startExport(m_model);
}
