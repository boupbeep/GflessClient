#ifndef SETTINGSDIALOG_H
#define SETTINGSDIALOG_H

#include <QDialog>
#include <QFileDialog>
#include <QStringList>
#include <QVector>

#include "gameforgeaccount.h"

namespace Ui {
class SettingsDialog;
}

class QListWidgetItem;

enum Theme
{
    ThemeModernWindows = 0,
    ThemeFusion,
    ThemeWindows,
    ThemeDark,
    ThemeLight,
    ThemeDefault = ThemeDark
};

class SettingsDialog : public QDialog
{
    Q_OBJECT

public:
    explicit SettingsDialog(QWidget *parent = nullptr);
    ~SettingsDialog();

    QString getGameClientPath() const;

    QString getProfilesPath() const;

    int getOpenIntervalMin() const;

    int getOpenIntervalMax() const;

    int getGameLanguage() const;

    int getGameLanguageIndex() const;

    int getTheme() const;

    void setGameClientPath(const QString& path);

    void setProfilesPath(const QString& path);

    void setOpenIntervalMin(int n);

    void setOpenIntervalMax(int n);

    void setGameLanguage(int language);

    void setTheme(int index);

    void setThemeComboBox(int index);

    static void applyTheme(int index);

    void setDisabledNosmall(bool b);

    bool getDisabledNosmall() const;

    void setCheckUpdates(bool b);

    bool getCheckUpdates() const;

    QStringList getSelectedGamePaths() const;

    void populateGamePaths(const QStringList& paths);

    void setGameforgeAccounts(const QVector<GameforgeAccount*>& accounts);

    void markPathUpdated(const QString& path);

signals:
    void profilesPathSelected(QString);

    void updateRequested(const QStringList& paths);

private slots:
    void on_selectGamePathButton_clicked();

    void on_selectProfilePathButton_clicked();

    void on_themeComboBox_currentIndexChanged(int index);

    void on_openIntervalMinSpinBox_valueChanged(int value);

    void on_openIntervalMaxSpinBox_valueChanged(int value);

    void on_refreshPathsButton_clicked();

    void on_updateSelectedButton_clicked();

    void on_selectAllCheckBox_toggled(bool checked);

    void on_gamePathsListWidget_itemDoubleClicked(QListWidgetItem* item);

private:
    void initLanguageComboBox();
    QStringList collectGamePaths() const;
    QStringList exePathsForDir(const QString& dir) const;
    QString sanitizeKey(const QString& path) const;
    QString formatPathDisplay(const QString& path) const;

    Ui::SettingsDialog *ui;
    QVector<GameforgeAccount*> gfAccounts;
};


#endif // SETTINGSDIALOG_H
