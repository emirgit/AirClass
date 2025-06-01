#include "uicontroller.h"
#include <QDebug>
#include <QTimer>

#include "uicontroller.h"
#include <QDebug>

void UIController::updateConnectionStatus(bool connected)
{
    // Log connection status change
    if (connected) {
        qDebug() << "Connected to server2";
    } else {
        qDebug() << "Disconnected from server2";
    }
}

void UIController::updatePageIndicator(int currentPage, int totalPages)
{
    // Log page indicator update
    qDebug() << "Page " << currentPage + 1 << " of " << totalPages;
}

void UIController::updateAttendanceDisplay(int totalCount)
{
    // Log attendance count update
    qDebug() << "Students: " << totalCount;
}

void UIController::showSpeakRequestNotification(const QString &studentId, const QString &studentName)
{
    // Log speak request
    qDebug() << "Student with ID" << studentId << "requested to speak";

    QString message = QString("%1 wants to speak").arg(studentName);
    showNotification(message, 5000);
}

void UIController::showNotification(const QString &message, int durationMs)
{
    // Log notification message
    qDebug() << "Notification: " << message;

    // Start timer to simulate notification auto-hide
    m_notificationTimer.start(durationMs);
}



UIController::UIController(QObject *parent)
    : QObject(parent)
    , m_presentationView(nullptr)
    , m_dashboardView(nullptr)
    , m_studentsView(nullptr)
    , m_qrCodeView(nullptr) // Add QR Code view initialization
    , m_currentView(Dashboard)
{
    // Set up notification timer (without UI elements)
    m_notificationTimer.setSingleShot(true);
    connect(&m_notificationTimer, &QTimer::timeout, []() {
        // Reset notification (without UI components)
        qDebug() << "Notification cleared after timeout";
    });
}

void UIController::setupViews(QWidget *presentationView, QWidget *dashboardView, QWidget *studentsView, QWidget *qrCodeView)
{
    m_presentationView = presentationView;
    m_dashboardView = dashboardView;
    m_studentsView = studentsView;
    m_qrCodeView = qrCodeView; // Assign QR Code view

    // Initial view setup (without UI components)
    setViewVisibility();
}

void UIController::switchToView(ViewType viewType)
{
    m_currentView = viewType;
    setViewVisibility();

    // Log view switch (no UI update)
    switch (m_currentView) {
    case Dashboard:
        qDebug() << "Switched to Dashboard view";
        break;
    case Presentation:
        qDebug() << "Switched to Presentation view";
        break;
    case Students:
        qDebug() << "Switched to Students view";
        break;
    case QRCode: // Handle QR Code view
        qDebug() << "Switched to QR Code view";
        break;
    }
}

void UIController::setViewVisibility()
{
    // Implement view switching logic (without UI components)
    qDebug() << "Setting visibility for current view:" << m_currentView;

    if (m_presentationView) {
        m_presentationView->setVisible(m_currentView == Presentation);
    }

    if (m_dashboardView) {
        m_dashboardView->setVisible(m_currentView == Dashboard);
    }

    if (m_studentsView) {
        m_studentsView->setVisible(m_currentView == Students);
    }

    if (m_qrCodeView) {
        m_qrCodeView->setVisible(m_currentView == QRCode); // Set visibility for QR Code view
    }
}
