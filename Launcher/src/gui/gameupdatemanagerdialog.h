#ifndef GAMEUPDATEMANAGERDIALOG_H
#define GAMEUPDATEMANAGERDIALOG_H

#include <QDialog>
#include <QStringList>
#include <QVector>

#include "gameforgeaccount.h"

namespace Ui {
class GameUpdateManagerDialog;
}

class QListWidgetItem;

class GameUpdateManagerDialog : public QDialog
{
    Q_OBJECT

public:
    explicit GameUpdateManagerDialog(const QVector<GameforgeAccount*>& accounts, const QString& globalGamePath, QWidget *parent = nullptr);
    ~GameUpdateManagerDialog();

    QStringList getSelectedGamePaths() const;

    void markPathUpdated(const QString& path);

signals:
    void updateRequested(const QStringList& paths);

private slots:
    void on_refreshPathsButton_clicked();

    void on_updateSelectedButton_clicked();

    void on_selectAllCheckBox_toggled(bool checked);

    void on_gamePathsListWidget_itemDoubleClicked(QListWidgetItem* item);

private:
    void populateGamePaths(const QStringList& paths);
    QStringList collectGamePaths() const;
    QStringList exePathsForDir(const QString& dir) const;
    QString sanitizePathForKey(const QString& path) const;
    QString formatPathDisplay(const QString& path) const;

    Ui::GameUpdateManagerDialog *ui;
    QVector<GameforgeAccount*> gfAccounts;
    QString globalGamePath;
};

#endif // GAMEUPDATEMANAGERDIALOG_H
