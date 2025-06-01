#include "gestureprocessor.h"
#include <QDebug>

GestureProcessor::GestureProcessor(QObject *parent)
    : QObject(parent)
{
    // Constructor can be empty or used for other initializations if needed in the future.
}

void GestureProcessor::registerGestureCommand(const QString &gestureName, std::function<void()> callback)
{
    if (gestureName.isEmpty()) {
        qWarning() << "GestureProcessor::registerGestureCommand: Attempted to register command for an empty gestureName.";
        return;
    }
    if (!callback) {
        qWarning() << "GestureProcessor::registerGestureCommand: Attempted to register a null callback for gesture:" << gestureName;
        // Optionally, you could remove an existing entry if a null callback is provided:
        // m_gestureCommandCallbacks.remove(gestureName);
        return;
    }
    m_gestureCommandCallbacks[gestureName] = callback;
    qDebug() << "GestureProcessor: Registered command for gesture:" << gestureName;
}

bool GestureProcessor::executeGestureCommand(const QString &gestureName)
{
    if (gestureName.isEmpty()) {
        qWarning() << "GestureProcessor::executeGestureCommand: Attempted to execute command for an empty gestureName.";
        return false;
    }

    if (m_gestureCommandCallbacks.contains(gestureName)) {
        std::function<void()> command = m_gestureCommandCallbacks.value(gestureName);
        if (command) { // Check if the std::function is callable (not empty)
            qDebug() << "GestureProcessor: Executing command for gesture:" << gestureName;
            command();
            return true;
        } else {
            // This case implies that an empty/null std::function was somehow registered.
            // The check in registerGestureCommand should prevent this for new registrations.
            qWarning() << "GestureProcessor: Found gesture" << gestureName << "but its command callback is empty/null.";
        }
    } else {
        qDebug() << "GestureProcessor: No command registered for gesture:" << gestureName;
    }
    return false;
}
