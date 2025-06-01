#ifndef GESTUREGUIDE_H
#define GESTUREGUIDE_H

#include <QDialog>
#include <QList>
#include <QMap>
#include <QStringList>
// #include "gestureprocessor.h" // REMOVED

// Forward declarations
QT_BEGIN_NAMESPACE
class QVBoxLayout;
class QHBoxLayout;
class QTabWidget;
class QScrollArea;
class QLabel;
class QPushButton;
class QComboBox;
class QSpinBox;
class QLineEdit; // For gesture name display
QT_END_NAMESPACE

// Structure to hold detailed gesture information
struct GestureInfo {
    QString id;          // e.g., "three_gun", "call" (used for image lookup and internal ID)
    QString name;        // User-friendly name (can be derived from id or set explicitly)
    QString action;      // e.g., "next_slide", "drawing", or empty if customizable
    int repeatCount;     // Default 1, or specified in the file
    QString imagePath;   // Path to the image in resources
    bool isRequired;     // True if it has a pre-defined action, false if customizable
    QString description; // Description for required gestures (could be auto-generated)

    GestureInfo() : repeatCount(1), isRequired(false) {}
};

class GestureGuide : public QDialog
{
    Q_OBJECT

public:
    explicit GestureGuide(QWidget *parent = nullptr); // GestureProcessor* removed
    ~GestureGuide();

    static QString getGestureMapPath();

private slots:
    void saveGestureConfiguration(); // Slot to handle saving

private:
    void initializeCustomizableActions();
    void loadGestureMap();
    void saveGestureMap();
    void setupUI();
    QWidget* createGesturesTab(bool requiredGestures);
    void populateGestureItem(QHBoxLayout* itemLayout, const GestureInfo &gesture, QWidget* parentTab);
    void updateGestureConfig(const QString& gestureId, const QString& newAction, int newRepeatCount);
    void updateGestureList();
    void updateActionComboBox();

    // GestureProcessor* m_gestureProcessor; // REMOVED
    QTabWidget *m_tabWidget;

    QList<GestureInfo> m_allGestures; // Store all gestures loaded from the file
    QStringList m_customizableActionsList; // List of actions for customizable gestures UI

    // For custom gestures tab to map UI elements to gesture IDs for saving
    QMap<QString, QComboBox*> m_customActionCombos;
    QMap<QString, QSpinBox*> m_customRepeatSpins;
};

#endif // GESTUREGUIDE_H
