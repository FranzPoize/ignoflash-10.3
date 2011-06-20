#include <QFileDialog>
#include <QDir>
#include <QDirModel>
#include <QCompleter>

#include "dialog.h"
#include "detect.h"
#include "backup.h"

#include "activex.h"
#include "netscape_win32.h"
#include "netscape_linux32.h"

#define BACKUP_SUFFIX "ignobak"

dialog::dialog(QWidget *parent, Qt::WFlags flags) : QMainWindow(parent, flags)
{
  /* Create the UI */
  ui.setupUi(this);

  /* Connect the signals to slots */
  connect(ui.browse, SIGNAL(clicked()), this, SLOT(browse()));
  connect(ui.fileName, SIGNAL(currentIndexChanged(const QString&)), this, SLOT(fileNameChanged(const QString&)));
  connect(ui.apply, SIGNAL(clicked()), this, SLOT(apply()));
  connect(ui.restore, SIGNAL(clicked()), this, SLOT(restore()));

  /* Auto completion for file name entry */
  QCompleter *completer = new QCompleter(this);
  completer->setModel(new QDirModel(completer));
  ui.fileName->setCompleter(completer);

  /* Prepopulate the file name selection with detected plugins */
  ui.fileName->clear();
  ui.fileName->insertItems(0, detect_plugins());
}

dialog::~dialog()
{
}

void dialog::updatePatch(const QString &file)
{
  if (file.isEmpty() || !ver_valid) {
    ui.restore->setEnabled(false);
    ui.apply->setEnabled(false);
    ui.ignoreFocus->setEnabled(false);
    ui.disableTopmost->setEnabled(false);
    return;
  }

  /* Check for backup, if it exists, only enable the restore button. */
  backup filebak(file, BACKUP_SUFFIX);
  bool hasBackup = filebak.exists();

  ui.restore->setEnabled(hasBackup ? true : false);
  ui.apply->setEnabled(hasBackup ? false : true);
  ui.ignoreFocus->setEnabled(hasBackup ? false : true);
  ui.disableTopmost->setEnabled(hasBackup ? false : true);
}

void dialog::browse()
{
  QString plugin = QFileDialog::getOpenFileName(
    this,
    tr("Select Flash plugin"),
    QString(),
    tr("All plugins (*.dll *.ocx *.so);;Windows Netscape plugins (*.dll);;Windows ActiveX plugins (*.ocx);;Linux Netscape plugins (*.so);;All files(*.*)"));

  if (plugin.isEmpty())
    return;

  ui.fileName->insertItem(0, QDir::toNativeSeparators(plugin));
  ui.fileName->setCurrentIndex(0);
}

void dialog::fileNameChanged(const QString &text)
{
  if (!text.isEmpty()) {
    if (text.endsWith(".ocx", Qt::CaseInsensitive)) {
      ui.netscape->setChecked(false);
      ui.activeX->setChecked(true);
    } else {
      ui.netscape->setChecked(true);
      ui.activeX->setChecked(false);
    }
  }

  ver_valid = utils::get_flash_file_version(text, ver);
  if (ver_valid) {
    ui.logBox->append(tr("Detected file version as ") + ver.str());
  } else {
    ui.logBox->append(tr("Failed to detect file version!"));
  }

  updatePatch(text);
}

void dialog::apply()
{
  QString file = ui.fileName->currentText();
  if (file.isEmpty()) {
    ui.logBox->append(tr("ERROR: Select a file first!"));
    return;
  }

  ui.logBox->append("");

  ui.logBox->append(tr("Applying patch(es)..."));

  if (ui.activeX->isChecked()) {
    if (!utils::remove_acls(file)) {
      ui.logBox->append(tr("WARNING: Failed to remove the ACLs!"));
    }
  }

  backup filebak(file, BACKUP_SUFFIX);
  if (!filebak.create()) {
    ui.logBox->append(tr("ERROR: Failed to create a backup of the original file!"));
    return;
  }
  updatePatch(file);

  bool ignoreFocus, disableTopmost;

  ignoreFocus = ui.ignoreFocus->isChecked();
  disableTopmost = ui.disableTopmost->isChecked();

  std::list<patch::base*> pl;

  bool error = false;

  if (ui.activeX->isChecked()) {
    /* ActiveX */
    if (ignoreFocus) {
      pl.push_back(new activex_patch());
    }

    if (disableTopmost) {
      pl.push_back(new activex_topmost_patch());
      pl.push_back(new activex_10_1_topmost_patch());
    }
  } else {
    /* Netscape */
    if (ignoreFocus) {
      pl.push_back(new netscape_win32_patch());
      pl.push_back(new netscape_10_1_win32_patch());
      pl.push_back(new netscape_10_1_linux32_patch());
    }

    if (disableTopmost) {
      pl.push_back(new netscape_win32_topmost_patch());
      pl.push_back(new netscape_10_1_win32_topmost_patch());
    }
  }

  for (std::list<patch::base*>::iterator ip = pl.begin(); ip != pl.end(); ip++) {
    if (!(*ip)->probe(ver))
      continue;

    ui.logBox->append(tr("Applying ") + (*ip)->get_name() + "...");

    patch::status s = (*ip)->patch(file, ver);
    
    if (s != patch::ok) {
      ui.logBox->append(tr("ERROR: ") + patch::get_status_string(s) + "!");
      error = true;
    } else {
      ui.logBox->append(tr("SUCCESS!!!"));
    }

    delete (*ip);
  }

  if (!error)
    ui.logBox->append(tr("Success!"));
}

void dialog::restore()
{
  QString file = ui.fileName->currentText();
  if (file.isEmpty()) {
    ui.logBox->append(tr("ERROR: Select a file first!"));
    return;
  }

  ui.logBox->append("");

  ui.logBox->append(tr("Restoring backup..."));

  backup filebak(file, BACKUP_SUFFIX);
  if (!filebak.restore()) {
    ui.logBox->append(tr("ERROR: Failed to restore the original file from the backup!"));
    return;
  }
  updatePatch(file);

  ui.logBox->append(tr("Success!"));
}
