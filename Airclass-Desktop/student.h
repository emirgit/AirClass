#ifndef STUDENT_H
#define STUDENT_H

#include <QString>
#include <QDateTime>

class Student
{
public:
    Student();
    Student(const QString &id, const QString &name);

    QString getId() const;
    QString getName() const;
    QDateTime getJoinTime() const;
    bool hasRaisedHand() const;
    bool hasSpeakingPermission() const;

    void setId(const QString &id);
    void setName(const QString &name);
    void setJoinTime(const QDateTime &time);
    void setRaisedHand(bool raised);
    void setSpeakingPermission(bool permission);

private:
    QString m_id;
    QString m_name;
    QDateTime m_joinTime;
    bool m_raisedHand;
    bool m_speakingPermission;
};

#endif // STUDENT_H
