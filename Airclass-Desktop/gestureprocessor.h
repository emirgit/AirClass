#ifndef GESTUREPROCESSOR_H
#define GESTUREPROCESSOR_H

#include <QObject>
#include <QMap>
#include <QString>
#include <functional> // For std::function

QT_BEGIN_NAMESPACE
class QString;
QT_END_NAMESPACE

class GestureProcessor : public QObject
{
    Q_OBJECT
public:
    explicit GestureProcessor(QObject *parent = nullptr);

    // Registers a command (lambda or function pointer) to be executed for a given gesture name.
    void registerGestureCommand(const QString &gestureName, std::function<void()> callback);

    // Executes a registered command for the given gesture name.
    // Returns true if a command was found and executed, false otherwise.
    bool executeGestureCommand(const QString &gestureName);

private:
    // Stores the mapping from gesture names to their command callbacks.
    QMap<QString, std::function<void()>> m_gestureCommandCallbacks;
};

#endif // GESTUREPROCESSOR_H
