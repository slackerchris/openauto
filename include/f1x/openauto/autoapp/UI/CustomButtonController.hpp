#pragma once
#include <QObject>
#include <QString>
#include <QPushButton>
#include <QFile>
#include <QTextStream>
#include <QStringList>

namespace f1x {
namespace openauto {
namespace autoapp {
namespace ui {

class CustomButtonController : public QObject {
public:
    explicit CustomButtonController(QObject* parent = nullptr);
    void setupButton(QPushButton* button, const QString& configFile, bool forceEnable);
    void handleButtonPress(const QString& command);
    void updateButtonAlpha(int alphaValue);

private:
    struct ButtonInfo {
        QPushButton* button;
        QString color;
    };
    QVector<ButtonInfo> buttons_;
};

}}}} // namespace
