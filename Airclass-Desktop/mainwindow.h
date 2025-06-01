#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QPdfView>
#include <QPdfWriter>
#include <QPainter>
#include <QPageSize>
#include <QStringList>
#include <QJsonObject>
#include <QJsonArray>
#include <QMediaPlayer>
#include <QListWidgetItem>
#include <QTimer>
#include <QAudioOutput>
#include <QLabel>
#include <QInputDialog>
#include <QList>
#include <QApplication>
#include <QVBoxLayout>
#include "websocketclient.h"
#include "presentationmanager.h"
#include "attendancemanager.h"
#include "gestureprocessor.h"
#include "uicontroller.h"
#include "restapiclient.h"
#include "drawinglayer.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(RestApiClient* restApi, WebSocketClient* wsClient, QWidget *parent = nullptr);
    ~MainWindow();

    // Public methods for external access
    void showFullScreen();
    QPdfView* getPdfView() const;
    PresentationManager* getPresentationManager() { return m_presentationManager; }
    void updateSessionInfo(const QString &sessionId, const QString &sessionName, const QString &sessionCode);

    // Handler for gesture commands (public for GestureProcessor)
    void handleGestureCommand(const QJsonObject &data);

protected:
    bool eventFilter(QObject *watched, QEvent *event) override;

private slots:
    // WebSocket connection slots
    void onConnected();
    void onDisconnected();
    void handleServerMessage(const QString &message);

    // REST API slots
    // void onLoginSuccess(const QJsonObject &data);
    void onLoginSuccess(const QJsonObject &data, const QString &token);
    void onLoginFailed(const QString &message);
    void onAttendanceListReceived(const QJsonArray &records);
    void onAttendanceCodeGenerated(const QString &code, const QString &qrBase64, const QString &expiry);
    void onRequestsReceived(const QJsonArray &requests);
    void onNewSpeakRequest(const QString &studentId, const QString &studentName, const QString &requestId);
    void onRequestUpdated(const QString &requestId, const QString &action);
    void onSessionClosed(const QString &sessionId);
    void onQrCodeReceived(const QString &qrBase64);
    void closeSession();

    // UI action slots
    void on_actionOpen_triggered();
    void on_actionExit_triggered();
    void on_actionConnect_triggered();
    void on_actionDisconnect_triggered();
    void on_actionDashboard_triggered();
    void on_actionPresentation_triggered();
    void on_actionStudents_triggered();
    void on_actionGenerate_QR_Code_triggered();
    void on_connectButton_clicked();
    void on_disconnectButton_clicked();
    void on_openPresentationButton_clicked();
    void on_prevButton_clicked();
    void on_nextButton_clicked();
    void on_zoomInButton_clicked();
    void on_zoomOutButton_clicked();
    void on_approveButton_clicked();
    void on_rejectButton_clicked();
    void on_closeSessionButton_clicked();
    void downloadAttendanceReport();

    // QR Code button slots
    void on_saveImageButton_clicked();
    void on_printButton_clicked();
    void on_displayFullScreenButton_clicked();
    void on_generateAttendanceCodeButton_clicked();

    // Attendance button slots
    void on_refreshAttendanceButton_clicked();
    void on_downloadAttendanceButton_clicked();

    // Speak request slots
    void on_approveRequestButton_clicked();
    void on_rejectRequestButton_clicked();

    // Add recent presentation to list
    void addRecentPresentation(const QString &filePath);

    // Handle WebSocket attendance update
    void handleAttendanceUpdate(const QJsonObject &data);
    void handleAttendanceQRCode(const QJsonObject &data);
    void handleSpeakRequest(const QJsonObject &data);
    void handleRequestUpdate(const QJsonObject &data);

    // Timer slots
    void on_timerStartButton_clicked();
    void on_timerStopButton_clicked();
    void on_timerResetButton_clicked();
    void on_timerMinutesSpinBox_valueChanged(int value);
    void on_timerSecondsSpinBox_valueChanged(int value);
    void on_clearDrawingButton_clicked();
    void on_colorButton_clicked();

    // New slots for Gesture Guide and Timeout actions
    void on_actionGestureGuide_triggered();
    void on_actionTimeout_triggered();


    // New slot for break timer
    void onBreakTimerTick();
    void onAttendanceTestSuccess(const QJsonObject &responseData);
    void onAttendanceTestFailed(const QString &errorMsg);

    void onSessionSelected(const QString &sessionId, const QString &sessionName, const QString &sessionCode);
    void onAttendanceCodeReady(const QString &code, const QString &expiresAt, const QString &expiresIn);
    void onAttendanceCodeFailed(const QString &errorMsg);
    void onQrCodeImageDownloaded(QNetworkReply *reply);

private:
    Ui::MainWindow *ui;

    // Core managers
    WebSocketClient *m_webSocketClient;
    PresentationManager *m_presentationManager;
    AttendanceManager *m_attendanceManager;
    GestureProcessor *m_gestureProcessor;
    UIController *m_uiController;
    RestApiClient *m_restApi;

    // Timer variables
    QTimer *m_timer;
    bool m_isTimerRunning;
    int m_remainingSeconds;

    // Audio player for notifications
    QMediaPlayer *m_notificationPlayer;

    // Session info
    QString m_sessionId;
    QString m_sessionName;
    QString m_sessionCode;
    QString m_userName;
    QString m_currentRoomId;
    QString m_currentSessionId;
    QString m_authToken;

    // Current speak request
    QString m_currentRequestId;
    QString m_currentRequestStudentName;

    // Recent presentations list
    QStringList m_recentPresentations;

    // Drawing layer
    DrawingLayer *m_drawingLayer;

    // Break timer variables
    QTimer* m_breakTimer;
    QLabel* m_breakTimerLabel;
    int m_remainingBreakSeconds;

    // Speak requests
    QList<QJsonObject> m_speakRequests;

    // Helper methods
    void initialize();
    void setupConnections();
    void setupRestApiConnections();
    void playNotificationSound();
    void showSpeakRequestNotification(const QString &studentName, const QString &requestId);
    void hideSpeakRequestNotification();
    void showLoginDialog();
    void showSessionDialog();
    void setupDrawingControls();
    void updateTimer();
    void updateTimerDisplay();
    void showTimerDialog();
    void startBreakTimer(int minutes);
    void updateBreakTimerDisplay();

    void addGestureItem(QVBoxLayout *layout, const QString &gesture, const QString &name, 
                       const QString &description, const QString &imagePath);

    QNetworkAccessManager *m_qrNetworkManager; // QR kodunu indirmek için

};

#endif // MAINWINDOW_H
