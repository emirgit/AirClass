#ifndef TESTDATA_H
#define TESTDATA_H

#include <QStringList>

class TestData
{
public:
    static QStringList getTestStudents();
    static QStringList getTestSpeakRequests();
    static QStringList getTestAttendanceData();
};

#endif // TESTDATA_H
