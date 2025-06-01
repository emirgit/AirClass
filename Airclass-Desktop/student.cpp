#include "student.h"

Student::Student()
    : m_raisedHand(false)
    , m_speakingPermission(false)
{
    m_joinTime = QDateTime::currentDateTime();
}

Student::Student(const QString &id, const QString &name)
    : m_id(id)
    , m_name(name)
    , m_raisedHand(false)
    , m_speakingPermission(false)
{
    m_joinTime = QDateTime::currentDateTime();
}

QString Student::getId() const
{
    return m_id;
}

QString Student::getName() const
{
    return m_name;
}

QDateTime Student::getJoinTime() const
{
    return m_joinTime;
}

bool Student::hasRaisedHand() const
{
    return m_raisedHand;
}

bool Student::hasSpeakingPermission() const
{
    return m_speakingPermission;
}

void Student::setId(const QString &id)
{
    m_id = id;
}

void Student::setName(const QString &name)
{
    m_name = name;
}

void Student::setJoinTime(const QDateTime &time)
{
    m_joinTime = time;
}

void Student::setRaisedHand(bool raised)
{
    m_raisedHand = raised;
}

void Student::setSpeakingPermission(bool permission)
{
    m_speakingPermission = permission;
}
