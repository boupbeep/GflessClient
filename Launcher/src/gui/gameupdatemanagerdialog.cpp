#include "gameupdatemanagerdialog.h"
#include "ui_gameupdatemanagerdialog.h"

#include <QListWidgetItem>
#include <QMessageBox>
#include <QSettings>
#include <QDateTime>

GameUpdateManagerDialog::GameUpdateManagerDialog(const QVector<GameforgeAccount *> &accounts, const QString &globalGamePath, QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::GameUpdateManagerDialog)
    , gfAccounts(accounts)
    , globalGamePath(globalGamePath)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);

    populateGamePaths(collectGamePaths());
}

GameUpdateManagerDialog::~GameUpdateManagerDialog()
{
    delete ui;
}

QStringList GameUpdateManagerDialog::collectGamePaths() const
{
    QStringList paths;

    if (!globalGamePath.isEmpty()) {
        QString dir = globalGamePath.left(globalGamePath.lastIndexOf("/"));

        if (!dir.isEmpty() && !paths.contains(dir, Qt::CaseInsensitive))
            paths.append(dir);
    }

    for (GameforgeAccount* acc : gfAccounts) {
        QString customPath = acc->getcustomClientPath();

        if (customPath.isEmpty())
            continue;

        QString dir = customPath.left(customPath.lastIndexOf("/"));

        if (!dir.isEmpty() && !paths.contains(dir, Qt::CaseInsensitive))
            paths.append(dir);
    }

    return paths;
}

QStringList GameUpdateManagerDialog::exePathsForDir(const QString &dir) const
{
    QStringList exePaths;

    if (!globalGamePath.isEmpty() && globalGamePath.left(globalGamePath.lastIndexOf("/")).compare(dir, Qt::CaseInsensitive) == 0) {
        if (!exePaths.contains(globalGamePath, Qt::CaseInsensitive))
            exePaths.append(globalGamePath);
    }

    for (GameforgeAccount* acc : gfAccounts) {
        QString customPath = acc->getcustomClientPath();

        if (customPath.isEmpty())
            continue;

        if (customPath.left(customPath.lastIndexOf("/")).compare(dir, Qt::CaseInsensitive) == 0) {
            if (!exePaths.contains(customPath, Qt::CaseInsensitive))
                exePaths.append(customPath);
        }
    }

    return exePaths;
}

QString GameUpdateManagerDialog::sanitizePathForKey(const QString &path) const
{
    QString key = path;
    key.replace('/', '_');
    key.replace('\\', '_');
    key.replace(':', '_');
    return key;
}

QString GameUpdateManagerDialog::formatPathDisplay(const QString &path) const
{
    QSettings settings;
    settings.beginGroup("GameUpdates");
    QDateTime timestamp = settings.value(sanitizePathForKey(path)).toDateTime();
    settings.endGroup();

    if (!timestamp.isValid())
        return path;

    return path + "    " + QChar(0x2713) + " " + timestamp.toString("dd/MM/yyyy HH:mm");
}

void GameUpdateManagerDialog::populateGamePaths(const QStringList &paths)
{
    ui->gamePathsListWidget->clear();

    for (const QString& path : paths) {
        QListWidgetItem* item = new QListWidgetItem(formatPathDisplay(path), ui->gamePathsListWidget);
        item->setFlags(item->flags() | Qt::ItemIsUserCheckable);
        item->setCheckState(Qt::Checked);
        item->setData(Qt::UserRole, path);
        item->setData(Qt::UserRole + 1, exePathsForDir(path));
    }
}

void GameUpdateManagerDialog::markPathUpdated(const QString &path)
{
    QSettings settings;
    settings.beginGroup("GameUpdates");
    settings.setValue(sanitizePathForKey(path), QDateTime::currentDateTime());
    settings.endGroup();

    for (int i = 0; i < ui->gamePathsListWidget->count(); ++i) {
        QListWidgetItem* item = ui->gamePathsListWidget->item(i);

        if (item->data(Qt::UserRole).toString().compare(path, Qt::CaseInsensitive) == 0) {
            item->setText(formatPathDisplay(path));
            break;
        }
    }
}

QStringList GameUpdateManagerDialog::getSelectedGamePaths() const
{
    QStringList paths;

    for (int i = 0; i < ui->gamePathsListWidget->count(); ++i) {
        QListWidgetItem* item = ui->gamePathsListWidget->item(i);

        if (item->checkState() == Qt::Checked)
            paths.append(item->data(Qt::UserRole).toString());
    }

    return paths;
}

void GameUpdateManagerDialog::on_refreshPathsButton_clicked()
{
    populateGamePaths(collectGamePaths());
}

void GameUpdateManagerDialog::on_updateSelectedButton_clicked()
{
    emit updateRequested(getSelectedGamePaths());
}

void GameUpdateManagerDialog::on_selectAllCheckBox_toggled(bool checked)
{
    for (int i = 0; i < ui->gamePathsListWidget->count(); ++i) {
        QListWidgetItem* item = ui->gamePathsListWidget->item(i);
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
}

void GameUpdateManagerDialog::on_gamePathsListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QStringList exePaths = item->data(Qt::UserRole + 1).toStringList();

    QString text = exePaths.isEmpty()
        ? QStringLiteral("No known executable path for this directory.")
        : exePaths.join("\n");

    QMessageBox::information(this, "Game executable path(s)", text);
}
