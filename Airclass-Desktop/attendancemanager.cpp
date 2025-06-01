#include "attendancemanager.h"
#include <QDebug>

AttendanceManager::AttendanceManager(QObject *parent)
    : QObject(parent)
    , m_studentListWidget(nullptr)
    , m_requestListWidget(nullptr)
{
}

void AttendanceManager::markAttendance(const Student &student)
{
    // Add or update student in map
    m_students[student.getId()] = student;

    // Update UI
    updateStudentListWidget();

    // Notify about the update
    emit attendanceUpdated(m_students.size());

    qDebug() << "Student marked present:" << student.getName() << "(" << student.getId() << ")";
}

void AttendanceManager::raiseHand(const QString &studentId)
{
    if (!m_students.contains(studentId)) {
        qWarning() << "Student ID not found:" << studentId;
        return;
    }

    Student &student = m_students[studentId];
    student.setRaisedHand(true);

    // Update UI
    updateRequestListWidget();

    // Notify about the new request
    emit newSpeakRequest(student.getId(), student.getName());

    qDebug() << "Student raised hand:" << student.getName() << "(" << student.getId() << ")";
}

void AttendanceManager::grantSpeakPermission(const QString &studentId)
{
    if (!m_students.contains(studentId)) {
        qWarning() << "Student ID not found:" << studentId;
        return;
    }

    Student &student = m_students[studentId];
    student.setSpeakingPermission(true);
    student.setRaisedHand(false); // Clear raised hand after granting permission

    // Update UI
    updateStudentListWidget();
    updateRequestListWidget();

    // Notify about permission granted
    emit speakPermissionGranted(studentId);

    qDebug() << "Speaking permission granted to:" << student.getName() << "(" << student.getId() << ")";
}

void AttendanceManager::revokeSpeakPermission(const QString &studentId)
{
    if (!m_students.contains(studentId)) {
        qWarning() << "Student ID not found:" << studentId;
        return;
    }

    Student &student = m_students[studentId];
    student.setSpeakingPermission(false);

    // Update UI
    updateStudentListWidget();

    // Notify about permission revoked
    emit speakPermissionRevoked(studentId);

    qDebug() << "Speaking permission revoked from:" << student.getName() << "(" << student.getId() << ")";
}

void AttendanceManager::setStudentListWidget(QListWidget *listWidget)
{
    m_studentListWidget = listWidget;
    updateStudentListWidget();
}

void AttendanceManager::setRequestListWidget(QListWidget *listWidget)
{
    m_requestListWidget = listWidget;
    updateRequestListWidget();
}

int AttendanceManager::getStudentCount() const
{
    return m_students.size();
}

QList<Student> AttendanceManager::getStudents() const
{
    return m_students.values();
}

QList<Student> AttendanceManager::getStudentsWithRaisedHands() const
{
    QList<Student> result;
    for (const Student &student : m_students.values()) {
        if (student.hasRaisedHand()) {
            result.append(student);
        }
    }
    return result;
}

Student AttendanceManager::getStudent(const QString &studentId) const
{
    if (m_students.contains(studentId)) {
        return m_students[studentId];
    }

    // Return empty student if not found
    return Student();
}

void AttendanceManager::updateStudentListWidget()
{
    if (!m_studentListWidget) {
        return;
    }

    m_studentListWidget->clear();

    for (const Student &student : m_students.values()) {
        QString displayText = student.getName();

        // Add indicators for raised hand and speaking permission
        if (student.hasRaisedHand()) {
            displayText += " ✋";
        }

        if (student.hasSpeakingPermission()) {
            displayText += " 🎤";
        }

        QListWidgetItem *item = new QListWidgetItem(displayText);
        item->setData(Qt::UserRole, student.getId());

        m_studentListWidget->addItem(item);
    }
}

void AttendanceManager::updateRequestListWidget()
{
    if (!m_requestListWidget) {
        return;
    }

    m_requestListWidget->clear();

    for (const Student &student : m_students.values()) {
        if (student.hasRaisedHand()) {
            QString displayText = student.getName() + " wants to speak";

            QListWidgetItem *item = new QListWidgetItem(displayText);
            item->setData(Qt::UserRole, student.getId());

            m_requestListWidget->addItem(item);
        }
    }
}
