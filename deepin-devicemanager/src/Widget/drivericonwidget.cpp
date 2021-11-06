#include "drivericonwidget.h"

#include <DBlurEffectWidget>
#include <DWidget>
#include <DLabel>
#include <DWaterProgress>
#include <DFontSizeManager>
#include <DApplicationHelper>
#include <DRadioButton>
#include <DGuiApplicationHelper>
#include <DLabel>
#include <DCheckBox>
#include <DFrame>

#include <QVBoxLayout>

DWIDGET_USE_NAMESPACE
/**
 * @brief palrtteTransparency 设置widget的字体颜色
 * @param alphaFloat 增加的透明度，如alphaFloat=-10：透明度90（100 - 10）
 */
static void palrtteTransparency(QWidget *widget, qint8 alphaFloat)
{
    QPalette palette = widget->palette();
    QColor color = DGuiApplicationHelper::adjustColor(palette.color(QPalette::Active, QPalette::BrightText), 0, 0, 0, 0, 0, 0, alphaFloat);
    palette.setColor(QPalette::WindowText, color);
    widget->setPalette(palette);
}

DriverIconWidget::DriverIconWidget(const QString &strTitle, const QString &strDesc, QWidget *parent)
    : DWidget(parent)
{
    iconWidget = new DWaterProgress;
    iconWidget->setFixedSize(80, 80);
    iconWidget->setValue(50);
    iconWidget->start();

    initUI(iconWidget, strTitle, strDesc);
}

DriverIconWidget::DriverIconWidget(const QPixmap &pixmap, const QString &strTitle, const QString &strDesc, QWidget *parent)
    : DWidget(parent)
{
    DLabel *iconWidget = new DLabel;
    iconWidget->setFixedSize(80, 80);
    iconWidget->setPixmap(pixmap);
    iconWidget->setScaledContents(true);

    initUI(iconWidget, strTitle, strDesc);
}

DriverIconWidget::~DriverIconWidget()
{

}

void DriverIconWidget::initUI(QWidget *iconWidget, const QString &strTitle, const QString &strDesc)
{
    QVBoxLayout *vlayout = new QVBoxLayout;
    vlayout->addSpacing(55);
    //iconWidget
    vlayout->addWidget(iconWidget, 0, Qt::AlignHCenter);
    vlayout->addSpacing(20);
    //strTitle
    {
        DLabel *label = new DLabel(strTitle);
        label->setMinimumHeight(20);
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);
        DFontSizeManager::instance()->bind(label, DFontSizeManager::T7, QFont::Medium);

        //透明度90
        palrtteTransparency(label, -10);
        QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [label]() {
            //透明度90
            palrtteTransparency(label, -10);
        });

        vlayout->addWidget(label, 0, Qt::AlignCenter);
    }
    vlayout->addSpacing(10);
    //strDesc
    {
        DLabel *label = new DLabel(strDesc);
        label->setMinimumHeight(20);
        label->setAlignment(Qt::AlignCenter);
        label->setWordWrap(true);
        DFontSizeManager::instance()->bind(label, DFontSizeManager::T7, QFont::Medium);

        //透明度50
        palrtteTransparency(label, -50);
        QObject::connect(DGuiApplicationHelper::instance(), &DGuiApplicationHelper::themeTypeChanged, this, [label]() {
            //透明度50
            palrtteTransparency(label, -50);
        });

        vlayout->addWidget(label, 0, Qt::AlignCenter);
    }
    vlayout->addStretch();

    setLayout(vlayout);
}

void DriverIconWidget::updateProgress(qint32 progress, const QString &detail)
{
    Q_UNUSED(detail)
    if(100 == progress) {
        emit sigProgressFinished();
    }
}
