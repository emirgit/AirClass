#ifndef ATTENDANCEMANAGER_H
#define ATTENDANCEMANAGER_H

#include <QObject>
#include <QList>
#include <QMap>
#include <QListWidget>
#include "student.h"

class AttendanceManager : public QObject
{
    Q_OBJECT
public:
    explicit AttendanceManager(QObject *parent = nullptr);

    void markAttendance(const Student &student);
    void raiseHand(const QString &studentId);
    void grantSpeakPermission(const QString &studentId);
    void revokeSpeakPermission(const QString &studentId);

    void setStudentListWidget(QListWidget *listWidget);
    void setRequestListWidget(QListWidget *listWidget);

    int getStudentCount() const;
    QList<Student> getStudents() const;
    QList<Student> getStudentsWithRaisedHands() const;
    Student getStudent(const QString &studentId) const;

signals:
    void attendanceUpdated(int totalCount);
    void newSpeakRequest(const QString &studentId, const QString &studentName);
    void speakPermissionGranted(const QString &studentId);
    void speakPermissionRevoked(const QString &studentId);

private:
    QMap<QString, Student> m_students;
    QListWidget *m_studentListWidget;
    QListWidget *m_requestListWidget;

    void updateStudentListWidget();
    void updateRequestListWidget();
};

#endif // ATTENDANCEMANAGER_H
