#ifndef DIALOG_H
#define DIALOG_H

#include <QtGui/QMainWindow>
#include "ui_dialog.h"

#include "patch.h"

class dialog : public QMainWindow
{
  Q_OBJECT

public:
  dialog(QWidget *parent = 0, Qt::WFlags flags = 0);
  ~dialog();

  void updatePatch(const QString &file);

public slots:
  void browse();
  void fileNameChanged(const QString &text);
  void apply();
  void restore();

private:
  Ui::dialogClass ui;

  utils::fileversion ver;
  bool ver_valid;
};

#endif // DIALOG_H
