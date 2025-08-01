#include <f1x/openauto/autoapp/UI/CustomButtonController.hpp>
#include <QFile>
#include <QTextStream>
#include <QStringList>
#include <QDebug>
#include <QProcess>

namespace f1x {
namespace openauto {
namespace autoapp {
namespace ui {

CustomButtonController::CustomButtonController(QObject* parent)
    : QObject(parent)
{
}

void CustomButtonController::setupButton(QPushButton* button, const QString& configFile, bool forceEnable)
{
    if (!forceEnable) {
        button->hide();
        return;
    }
    QFile paramFile(configFile);
    if (!paramFile.open(QIODevice::ReadOnly)) {
        button->hide();
        return;
    }
    QTextStream data(&paramFile);
    QStringList params = data.readAll().split("#");
    paramFile.close();
    button->setText(params[0].simplified());
    QString command = params[1].simplified();
    QString color = (params.size() > 2 && !params[2].isEmpty()) ? params[2].simplified() : "186,189,192";
    
    // Store button info for alpha updates
    ButtonInfo info = {button, color};
    buttons_.append(info);
    
    // Store command in button property for later use
    button->setProperty("customCommand", command);
    connect(button, &QPushButton::clicked, this, [this, button]() {
        handleButtonPress(button->property("customCommand").toString());
    });
}

void CustomButtonController::updateButtonAlpha(int alphaValue)
{
    QString alp = QString::number(alphaValue);
    for (const auto& buttonInfo : buttons_) {
        if (buttonInfo.button && buttonInfo.button->isVisible()) {
            buttonInfo.button->setStyleSheet(
                "QPushButton{background-color: rgba(" + buttonInfo.color + ", " + alp + " ); "
                "outline-style: dotted; outline-color: #92a8d1; border-radius: 4px; "
                "border: 2px solid rgba(255,255,255,0.5); color: rgb(255,255,255);} "
                "QPushButton:focus {border: 2px solid rgba(125,125,125,0.5);}"
            );
        }
    }
}

void CustomButtonController::handleButtonPress(const QString& command)
{
    QProcess::startDetached(command + " &");
}

}}}} // namespace
