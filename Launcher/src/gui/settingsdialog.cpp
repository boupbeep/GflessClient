#include "settingsdialog.h"
#include "ui_settingsdialog.h"

#include <QApplication>
#include <QFile>
#include <QStyleFactory>
#include <QListWidgetItem>
#include <QMessageBox>
#include <QSettings>
#include <QDateTime>

#include <array>

namespace {

struct RoleColor
{
    QPalette::ColorRole role;
    QRgb value;
};

constexpr std::array<RoleColor, 12> kLightColors{{
    { QPalette::Window, 0xf6f8fa },
    { QPalette::WindowText, 0x1f2328 },
    { QPalette::Base, 0xffffff },
    { QPalette::AlternateBase, 0xeaeef2 },
    { QPalette::Text, 0x1f2328 },
    { QPalette::Button, 0xffffff },
    { QPalette::ButtonText, 0x1f2328 },
    { QPalette::Highlight, 0x7a5af8 },
    { QPalette::HighlightedText, 0xffffff },
    { QPalette::ToolTipBase, 0xffffff },
    { QPalette::ToolTipText, 0x1f2328 },
    { QPalette::PlaceholderText, 0x656d76 },
}};

constexpr std::array<RoleColor, 12> kDarkColors{{
    { QPalette::Window, 0x0e1117 },
    { QPalette::WindowText, 0xe6edf3 },
    { QPalette::Base, 0x161b22 },
    { QPalette::AlternateBase, 0x1c2128 },
    { QPalette::Text, 0xe6edf3 },
    { QPalette::Button, 0x21262d },
    { QPalette::ButtonText, 0xe6edf3 },
    { QPalette::Highlight, 0x7a5af8 },
    { QPalette::HighlightedText, 0xffffff },
    { QPalette::ToolTipBase, 0x161b22 },
    { QPalette::ToolTipText, 0xe6edf3 },
    { QPalette::PlaceholderText, 0x6e7681 },
}};

QPalette themePalette(bool light)
{
    const auto &colors = light ? kLightColors : kDarkColors;
    QPalette palette;

    for (const RoleColor &color : colors)
        palette.setColor(color.role, QColor(color.value));

    return palette;
}

QString firstAvailableStyle(const QStringList &preferred)
{
    const QStringList available = QStyleFactory::keys();

    for (const QString &key : preferred)
    {
        if (available.contains(key, Qt::CaseInsensitive))
            return key;
    }

    return {};
}

void applyNativeStyle(int theme)
{
    QStringList preferred;

    switch (theme)
    {
    case ThemeModernWindows:
        preferred = { QStringLiteral("windows11"), QStringLiteral("windowsvista"), QStringLiteral("windows"), QStringLiteral("fusion") };
        break;
    case ThemeWindows:
        preferred = { QStringLiteral("windowsvista"), QStringLiteral("windows"), QStringLiteral("fusion") };
        break;
    case ThemeFusion:
        preferred = { QStringLiteral("fusion"), QStringLiteral("windowsvista"), QStringLiteral("windows") };
        break;
    default:
        return;
    }

    const QString styleKey = firstAvailableStyle(preferred);
    if (styleKey.isEmpty())
        return;

    QApplication::setStyle(styleKey);
    qApp->setStyleSheet(QString());
    qApp->setPalette(QPalette());
}

void applyCustomTheme(bool light)
{
    qApp->setPalette(themePalette(light));

    QFile styleFile(light ? QStringLiteral(":/resources/light.qss") : QStringLiteral(":/resources/dark.qss"));
    if (styleFile.open(QFile::ReadOnly | QFile::Text))
        qApp->setStyleSheet(QString::fromUtf8(styleFile.readAll()));
}

} // namespace

SettingsDialog::SettingsDialog(QWidget *parent) : QDialog(parent),
                                                  ui(new Ui::SettingsDialog)
{
    ui->setupUi(this);
    setWindowFlag(Qt::WindowContextHelpButtonHint, false);
    initLanguageComboBox();
}

SettingsDialog::~SettingsDialog()
{
    delete ui;
}

QString SettingsDialog::getGameClientPath() const
{
    return ui->gameClientPathLineEdit->text();
}

QString SettingsDialog::getProfilesPath() const
{
    return ui->profilesPathLineEdit->text();
}

int SettingsDialog::getOpenIntervalMin() const
{
    return ui->openIntervalMinSpinBox->value();
}

int SettingsDialog::getOpenIntervalMax() const
{
    return ui->openIntervalMaxSpinBox->value();
}

int SettingsDialog::getGameLanguage() const
{
    return ui->gameLanguageComboBox->currentData().toInt();
}

int SettingsDialog::getGameLanguageIndex() const
{
    return ui->gameLanguageComboBox->currentIndex();
}

int SettingsDialog::getTheme() const
{
    return ui->themeComboBox->currentIndex();
}

void SettingsDialog::setGameClientPath(const QString &path)
{
    ui->gameClientPathLineEdit->setText(path);
}

void SettingsDialog::setProfilesPath(const QString &path)
{
    ui->profilesPathLineEdit->setText(path);
}

void SettingsDialog::setOpenIntervalMin(int n)
{
    ui->openIntervalMinSpinBox->setValue(n);
}

void SettingsDialog::setOpenIntervalMax(int n)
{
    ui->openIntervalMaxSpinBox->setValue(n);
}

void SettingsDialog::setGameLanguage(int language)
{
    ui->gameLanguageComboBox->setCurrentIndex(language);
}

void SettingsDialog::setTheme(int index)
{
    QSignalBlocker blocker(ui->themeComboBox);
    ui->themeComboBox->setCurrentIndex(index);
    applyTheme(index);
}

void SettingsDialog::applyTheme(int index)
{
    switch (index)
    {
    case ThemeModernWindows:
    case ThemeFusion:
    case ThemeWindows:
        applyNativeStyle(index);
        break;
    case ThemeLight:
    case ThemeDark:
    default:
        applyCustomTheme(index == ThemeLight);
        break;
    }
}

void SettingsDialog::setThemeComboBox(int index)
{
    ui->themeComboBox->setCurrentIndex(index);
}

void SettingsDialog::setDisabledNosmall(bool b)
{
    ui->disableNosmallChekcbox->setChecked(b);
}

bool SettingsDialog::getDisabledNosmall() const
{
    return ui->disableNosmallChekcbox->isChecked();
}

void SettingsDialog::setCheckUpdates(bool b)
{
    ui->checkUpdatesCheckbox->setChecked(b);
}

bool SettingsDialog::getCheckUpdates() const
{
    return ui->checkUpdatesCheckbox->isChecked();
}

void SettingsDialog::on_selectGamePathButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Select NostaleClientX.exe", QDir::rootPath(), "NostaleClientX.exe (NostaleClientX.exe)");

    if (path.isEmpty())
        return;

    setGameClientPath(path);
}

void SettingsDialog::initLanguageComboBox()
{
    ui->gameLanguageComboBox->addItem("English", 0);
    ui->gameLanguageComboBox->addItem("Deutsch", 1);
    ui->gameLanguageComboBox->addItem("Français", 2);
    ui->gameLanguageComboBox->addItem("Italiano", 3);
    ui->gameLanguageComboBox->addItem("Polski", 4);
    ui->gameLanguageComboBox->addItem("Español", 5);
    ui->gameLanguageComboBox->addItem("Türkçe", 8);
    ui->gameLanguageComboBox->addItem("Čeština", 7);
}

void SettingsDialog::on_selectProfilePathButton_clicked()
{
    QString path = QFileDialog::getOpenFileName(this, "Select profile", QDir::rootPath(), "(*.ini)");

    if (path.isEmpty())
        return;

    setProfilesPath(path);

    emit profilesPathSelected(path);
}

void SettingsDialog::on_themeComboBox_currentIndexChanged(int index)
{
    setTheme(index);
}

void SettingsDialog::on_openIntervalMinSpinBox_valueChanged(int value)
{
    if (value > ui->openIntervalMaxSpinBox->value())
        ui->openIntervalMaxSpinBox->setValue(value);
}

void SettingsDialog::on_openIntervalMaxSpinBox_valueChanged(int value)
{
    if (value < ui->openIntervalMinSpinBox->value())
        ui->openIntervalMinSpinBox->setValue(value);
}

QStringList SettingsDialog::collectGamePaths() const
{
    QStringList paths;

    QString globalPath = getGameClientPath();
    if (!globalPath.isEmpty()) {
        QString dir = globalPath.left(globalPath.lastIndexOf("/"));

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

QStringList SettingsDialog::exePathsForDir(const QString &dir) const
{
    QStringList exePaths;

    QString globalPath = getGameClientPath();
    if (!globalPath.isEmpty() && globalPath.left(globalPath.lastIndexOf("/")).compare(dir, Qt::CaseInsensitive) == 0) {
        if (!exePaths.contains(globalPath, Qt::CaseInsensitive))
            exePaths.append(globalPath);
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

QString SettingsDialog::sanitizeKey(const QString &path) const
{
    QString key = path;
    key.replace('/', '_');
    key.replace('\\', '_');
    key.replace(':', '_');
    return key;
}

QString SettingsDialog::formatPathDisplay(const QString &path) const
{
    QSettings settings;
    settings.beginGroup("GameUpdates");
    QDateTime timestamp = settings.value(sanitizeKey(path)).toDateTime();
    settings.endGroup();

    if (!timestamp.isValid())
        return path;

    return path + "    " + QChar(0x2713) + " " + timestamp.toString("dd/MM/yyyy HH:mm");
}

void SettingsDialog::populateGamePaths(const QStringList &paths)
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

void SettingsDialog::markPathUpdated(const QString &path)
{
    QSettings settings;
    settings.beginGroup("GameUpdates");
    settings.setValue(sanitizeKey(path), QDateTime::currentDateTime());
    settings.endGroup();

    for (int i = 0; i < ui->gamePathsListWidget->count(); ++i) {
        QListWidgetItem* item = ui->gamePathsListWidget->item(i);

        if (item->data(Qt::UserRole).toString().compare(path, Qt::CaseInsensitive) == 0) {
            item->setText(formatPathDisplay(path));
            break;
        }
    }
}

void SettingsDialog::on_gamePathsListWidget_itemDoubleClicked(QListWidgetItem *item)
{
    QStringList exePaths = item->data(Qt::UserRole + 1).toStringList();

    QString text = exePaths.isEmpty()
        ? QStringLiteral("No known executable path for this directory.")
        : exePaths.join("\n");

    QMessageBox::information(this, "Game executable path(s)", text);
}

void SettingsDialog::setGameforgeAccounts(const QVector<GameforgeAccount *> &accounts)
{
    gfAccounts = accounts;
    populateGamePaths(collectGamePaths());
}

QStringList SettingsDialog::getSelectedGamePaths() const
{
    QStringList paths;

    for (int i = 0; i < ui->gamePathsListWidget->count(); ++i) {
        QListWidgetItem* item = ui->gamePathsListWidget->item(i);

        if (item->checkState() == Qt::Checked)
            paths.append(item->data(Qt::UserRole).toString());
    }

    return paths;
}

void SettingsDialog::on_refreshPathsButton_clicked()
{
    populateGamePaths(collectGamePaths());
}

void SettingsDialog::on_updateSelectedButton_clicked()
{
    emit updateRequested(getSelectedGamePaths());
}

void SettingsDialog::on_selectAllCheckBox_toggled(bool checked)
{
    for (int i = 0; i < ui->gamePathsListWidget->count(); ++i) {
        QListWidgetItem* item = ui->gamePathsListWidget->item(i);
        item->setCheckState(checked ? Qt::Checked : Qt::Unchecked);
    }
}
