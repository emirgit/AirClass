#ifndef UICONTROLLER_H
#define UICONTROLLER_H

#include <QObject>
#include <QWidget>
#include <QStackedWidget>
#include <QLabel>
#include <QString>
#include <QTimer>

// Forward declaration instead of including full header
namespace Ui { class MainWindow; }

class UIController : public QObject
{
    Q_OBJECT
public:
    enum ViewType {
        Dashboard,
        Presentation,
        Students,
        QRCode // Add QRCode view type
    };

    // Fix constructor signature to match implementation
    explicit UIController(QObject *parent = nullptr);

    void setupViews(QWidget *presentationView, QWidget *dashboardView, QWidget *studentsView, QWidget *qrCodeView); // Add qrCodeView parameter
    void switchToView(ViewType viewType);

public slots:
    void updateConnectionStatus(bool connected);
    void updatePageIndicator(int currentPage, int totalPages);
    void updateAttendanceDisplay(int totalCount);
    void showSpeakRequestNotification(const QString &studentId, const QString &studentName);
    void showNotification(const QString &message, int durationMs = 3000);

private:
    QWidget *m_presentationView;
    QWidget *m_dashboardView;
    QWidget *m_studentsView;
    QWidget *m_qrCodeView; // Add member for QR Code view
    ViewType m_currentView;
    QTimer m_notificationTimer;

    void setViewVisibility();
};

#endif // UICONTROLLER_H