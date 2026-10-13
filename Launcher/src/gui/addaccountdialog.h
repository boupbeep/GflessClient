#ifndef ADDACCOUNTDIALOG_H
#define ADDACCOUNTDIALOG_H

#include <QDialog>
#include <QMessageBox>

namespace Ui {
class AddAccountDialog;
}

class AddAccountDialog : public QDialog
{
    Q_OBJECT

public:
    explicit AddAccountDialog(QWidget *parent = nullptr);
    ~AddAccountDialog();

    const QString &getEmail() const;

    const QString &getPassword() const;

    QString getIdentityPath() const;

    QString getProxyIp() const;
    QString getSocksPort() const;
    bool getUseProxy() const;

    QString getProxyUsername() const;

    QString getProxyPassword() const;

    QString getCustomClientPath() const;

    QString getInstallationId() const;

    void prefillForEdit(const QString& email, const QString& password,
                        const QString& identityPath, const QString& customClientPath,
                        const QString& installationId, bool useProxy,
                        const QString& proxyIp, const QString& socksPort,
                        const QString& proxyUsername, const QString& proxyPassword);

private slots:
    void on_showPasswordCheckBox_stateChanged(int arg1);

    void on_loginButton_clicked();

    void on_selectIdentityButton_clicked();

    void on_proxyCheckBox_stateChanged(int arg1);

    void on_removeCustomGamePathButton_clicked();

    void on_selectCustomGamePathButton_clicked();

    void on_installationIdButton_clicked();

private:
    Ui::AddAccountDialog *ui;
    QString email;
    QString password;
    QString identityPath;
    QString proxyIp;
    QString socksPort;
    QString proxyUsername;
    QString proxyPassword;
    QString customClientPath;
    QString installationId;
    bool useProxy;
    bool editMode = false;
};

#endif // ADDACCOUNTDIALOG_H
