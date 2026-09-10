#include <sys.h>
#include <QApplication>
#include <QWidget>
#include <QPainter>
#include <QMouseEvent>
#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QVBoxLayout>
#include <QFont>
#include <QScreen>
#include <cstdint>

static bool initializeSystemLayer()
{
    sys_buf_t* buffer = nullptr;
    if (sys_buf_new(0, &buffer) != SYS_OK) {
        printf("Failed to create buffer\n");
        return false;
    } else {
        printf("OK\n");
    }
    const char message[] = "test_window";
    const bool ok = (sys_buf_append(buffer, message, sizeof(message) - 1) == SYS_OK);
    if (!ok) {
        printf("Failed to append message to buffer\n");
        sys_buf_free(buffer);
        return false;
    }
    printf("Message appended successfully: %s\n", message);
    sys_buf_free(buffer);
    printf("Buffer freed successfully\n");
    return true;
}
class BaseWindow : public QWidget
{
public:
    BaseWindow()
    {
        setWindowFlags(Qt::FramelessWindowHint | Qt::Window);
        setAttribute(Qt::WA_OpaquePaintEvent);
        setFixedSize(1000, 505);
        setStyleSheet(
            "QWidget {"
            "  background: #c0c0c0;"
            "  color: #000000;"
            "  font-family: 'MS Sans Serif', Arial, sans-serif;"
            "}"
        );
        buildUi();
        centerOnScreen();
    }
protected:
    void paintEvent(QPaintEvent*) override
    {
        QPainter p(this);
        p.setRenderHint(QPainter::Antialiasing, false);
        p.fillRect(rect(), QColor("#c0c0c0"));
        p.setPen(QColor("#ffffff"));

        p.drawLine(0, 0, width() - 1, 0);
        p.drawLine(0, 0, 0, height() - 1);
        p.setPen(QColor("#808080"));
        
        p.drawLine(width() - 1, 0, width() - 1, height() - 1);
        p.drawLine(0, height() - 1, width() - 1, height() - 1);
        p.setPen(QColor("#ffffff"));
        
        p.drawLine(3, 3, width() - 4, 3);
        p.drawLine(3, 3, 3, height() - 4);
        p.setPen(QColor("#808080"));
        
        p.drawLine(width() - 4, 3, width() - 4, height() - 4);
        p.drawLine(3, height() - 4, width() - 4, height() - 4);
    }
    void mousePressEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton &&
            event->position().y() <= titleBarHeight) {
            dragging = true;
            dragOffset = event->globalPosition().toPoint() - frameGeometry().topLeft();
            event->accept();
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override
    {
        if (dragging) {
            move(event->globalPosition().toPoint() - dragOffset);
            event->accept();
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override
    {
        if (event->button() == Qt::LeftButton)
            dragging = false;
        QWidget::mouseReleaseEvent(event);
    }
private:
    static constexpr int titleBarHeight = 30;

    bool dragging = false;
    QPoint dragOffset;

    void buildUi()
    {
        auto* root = new QVBoxLayout(this);
        root->setContentsMargins(7, 7, 7, 7);
        root->setSpacing(0);

        // Title bar.
        auto* titleBar = new QWidget(this);
        titleBar->setFixedHeight(titleBarHeight);
        titleBar->setStyleSheet("background:#0000aa;");

        auto* titleLayout = new QHBoxLayout(titleBar);
        titleLayout->setContentsMargins(18, 0, 8, 0);
        titleLayout->setSpacing(0);

        auto* title = new QLabel("test_window", titleBar);
        title->setStyleSheet(
            "color:white;"
            "background:transparent;"
            "font-family:'MS Sans Serif', Arial, sans-serif;"
            "font-size:10px;"
            "font-weight:bold;"
        );

        auto* close = new QPushButton("X", titleBar);
        close->setFixedSize(18, 14);
        close->setFont(QFont("Arial", 25));
        close->setStyleSheet(
            "QPushButton {"
            " background:#c0c0c0;"
            " color:#000000;"
            " border:3px solid;"
            " border-color:#ffffff #404040 #404040 #ffffff;"
            " padding:0;"
            "}"
            "QPushButton:pressed {"
            " border-color:#404040 #ffffff #ffffff #404040;"
            " padding-left:3px;"
            " padding-top:3px;"
            "}"
        );

        connect(close, &QPushButton::clicked, this, &QWidget::close);

        titleLayout->addWidget(title);
        titleLayout->addStretch();
        titleLayout->addWidget(close);

        // Main content.
        auto* content = new QWidget(this);
        content->setStyleSheet("background:#c0c0c0;");

        root->addWidget(titleBar);
        root->addWidget(content);
    }

    void centerOnScreen()
    {
        QScreen *screen = QApplication::primaryScreen();
        if (!screen)
            return;

        const QRect geometry = screen->availableGeometry();
        move(
            geometry.x() + (geometry.width() - width()) / 2,
            geometry.y() + (geometry.height() - height()) / 2
        );
    }
};

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    if (!initializeSystemLayer()) return 1;
    QWidget desktop;
    desktop.setWindowFlags(Qt::FramelessWindowHint);
    desktop.setStyleSheet("background:#008080;");

    BaseWindow window;
    window.show();

    return app.exec();
}
