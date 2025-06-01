#include "testdata.h"

QStringList TestData::getTestStudents()
{
    return QStringList()
    << "John Doe (ID: 001)"
    << "Jane Smith (ID: 002)"
    << "Bob Johnson (ID: 003)"
    << "Alice Williams (ID: 004)"
    << "Charlie Brown (ID: 005)";
}

QStringList TestData::getTestSpeakRequests()
{
    return QStringList()
    << "001,John Doe,I have a question about the last slide"
    << "003,Bob Johnson,Can you repeat the explanation?"
    << "004,Alice Williams,I'd like to add something";
}

QStringList TestData::getTestAttendanceData()
{
    return QStringList()
    << "Student ID,Student Name,Time,Status"
    << "001,John Doe,09:00:15,Present"
    << "002,Jane Smith,09:01:30,Present"
    << "003,Bob Johnson,09:02:45,Present"
    << "004,Alice Williams,09:03:20,Present"
    << "005,Charlie Brown,09:05:10,Present";
}
