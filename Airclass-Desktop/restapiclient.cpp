#include "restapiclient.h"
#include <QJsonDocument>
#include <QDebug>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery> // Eğer GET isteği olsaydı veya form data için


RestApiClient::RestApiClient(QObject *parent)
    : QObject(parent)
    , m_pollTimer(new QTimer(this))
{
    // m_networkManager = new QNetworkAccessManager(this);
    connect(m_pollTimer, &QTimer::timeout, this, &RestApiClient::pollRequests);
}

RestApiClient::~RestApiClient()
{
    if (m_pollTimer->isActive()) {
        m_pollTimer->stop();
    }
}

void RestApiClient::setBaseUrl(const QString &baseUrl)
{
    m_baseUrl = baseUrl;
    if (!m_baseUrl.endsWith('/')) {
        m_baseUrl += '/';
    }
}

void RestApiClient::setAuthToken(const QString &token)
{
    m_authToken = token;
}

QString RestApiClient::getAuthToken() const
{
    return m_authToken;
}

void RestApiClient::sendRequest(const QString &endpointPath, const QString &httpMethod,
                                const QJsonObject &jsonData, const QUrlQuery &urlQuery)
{
    if (endpointPath.isEmpty()) {
        qWarning() << "Empty endpoint provided";
        emit error("Internal error: Empty endpoint path.");
        return;
    }

    QStringList arguments;
    QUrl url(m_baseUrl + endpointPath);

    arguments << "-sS";
    arguments << "-L";
    arguments << "-X" << httpMethod.toUpper();

    arguments << "-H" << "Content-Type: application/json";
    arguments << "-H" << "Accept: application/json";
    if (!m_authToken.isEmpty()) {
        arguments << "-H" << QString("Authorization: Bearer %1").arg(m_authToken);
    }

    if (!urlQuery.isEmpty() && (httpMethod.toUpper() == "GET" || httpMethod.toUpper() == "DELETE")) {
        url.setQuery(urlQuery);
    }

    QByteArray bodyData;
    if (httpMethod.toUpper() == "POST" || httpMethod.toUpper() == "PUT") {
        if (!jsonData.isEmpty()) {
            bodyData = QJsonDocument(jsonData).toJson(QJsonDocument::Compact);
            arguments << "-d" << QString::fromUtf8(bodyData);
        }
    } else if (httpMethod.toUpper() == "DELETE" && !jsonData.isEmpty()) {
        bodyData = QJsonDocument(jsonData).toJson(QJsonDocument::Compact);
        arguments << "-d" << QString::fromUtf8(bodyData);
    }

    arguments << "-w" << "\\n%{http_code}";
    arguments << url.toString();

    QProcess *process = new QProcess(this);
    process->setProperty("endpoint", endpointPath);
    process->setProperty("method", httpMethod);

    connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
            this, &RestApiClient::handleProcessFinished);
    connect(process, &QProcess::errorOccurred, this, &RestApiClient::handleProcessError);

    qDebug() << "Executing curl with arguments:" << arguments.join(" ");
    process->start("curl", arguments);
}

void RestApiClient::handleProcessError(QProcess::ProcessError processError)
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    QString endpoint = process->property("endpoint").toString();
    qDebug() << "QProcess::errorOccurred for endpoint" << endpoint << "Error:" << processError << process->errorString();
    emit error(QString("Process execution error for %1: %2").arg(endpoint, process->errorString()));
    process->deleteLater();
}


void RestApiClient::handleProcessFinished(int exitCode, QProcess::ExitStatus exitStatus)
{
    QProcess *process = qobject_cast<QProcess*>(sender());
    if (!process) return;

    QString endpoint = process->property("endpoint").toString();
    QString method = process->property("method").toString();

    QByteArray allOutput = process->readAllStandardOutput();
    QByteArray errorOutput = process->readAllStandardError();
    process->deleteLater();

    if (exitStatus == QProcess::CrashExit || exitCode != 0) {
        qDebug() << "Process error for endpoint" << endpoint << "Method:" << method
                 << "Exit code:" << exitCode << "Exit status:" << exitStatus;
        QString stderrStr = QString::fromUtf8(errorOutput).trimmed();
        if (stderrStr.isEmpty() && exitCode !=0) {
            stderrStr = QString("curl exited with code %1").arg(exitCode);
        } else if (stderrStr.isEmpty()) {
            stderrStr = "Unknown curl error";
        }
        qDebug() << "stderr:" << stderrStr;
        emit error(QString("Request failed for %1. Error: %2")
                       .arg(endpoint, stderrStr));
        return;
    }

    QString outputStr = QString::fromUtf8(allOutput).trimmed();
    int lastNewline = outputStr.lastIndexOf('\n');
    QString responseBodyStr;
    int statusCode = 0;

    if (!outputStr.isEmpty()) {
        if (lastNewline != -1) {
            responseBodyStr = outputStr.left(lastNewline);
            QString statusCodeStr = outputStr.mid(lastNewline + 1);
            bool ok;
            statusCode = statusCodeStr.toInt(&ok);
            if (!ok) {
                qDebug() << "Failed to parse status code from curl output:" << statusCodeStr;
                responseBodyStr = outputStr;
                statusCode = 0;
            }
        } else {
            bool ok;
            int potentialCode = outputStr.toInt(&ok);
            if (ok && outputStr.length() == 3) {
                statusCode = potentialCode;
                responseBodyStr = "";
            } else {
                responseBodyStr = outputStr;
                statusCode = 0;
            }
        }
    }

    qDebug() << "Received response for endpoint" << endpoint
             << "Method:" << method
             << "Status code from curl:" << statusCode;
    qDebug() << "Raw response body:" << responseBodyStr;

    if (statusCode < 200 || statusCode >= 300) {
        qDebug() << "HTTP error for endpoint" << endpoint << ":" << statusCode;
        qDebug() << "Response body (error):" << responseBodyStr;
        QJsonDocument errorDoc = QJsonDocument::fromJson(responseBodyStr.toUtf8());
        QString errorMessage = QString("HTTP error %1").arg(statusCode);
        if (!errorDoc.isNull() && errorDoc.isObject()) {
            errorMessage += ": " + errorDoc.object().value("message").toString(errorDoc.object().value("error").toString(responseBodyStr));
        } else if (!responseBodyStr.isEmpty()) {
            errorMessage += ": " + responseBodyStr;
        }
        emit error(errorMessage);
        if(endpoint == "auth/login") emit loginFailed(errorMessage);
        if(endpoint == "auth/register") emit registerFailed(errorMessage);
        return;
    }

    QJsonDocument responseDoc = QJsonDocument::fromJson(responseBodyStr.toUtf8());

    if (responseBodyStr.isEmpty() && (statusCode == 200 || statusCode == 204)) {
        QJsonObject emptySuccessObj;
        emptySuccessObj["status"] = true;
        emptySuccessObj["message"] = "Operation successful (No Content)";
        emptySuccessObj["data"] = QJsonValue::Null;
        qDebug() << "Empty response body for successful request. Passing minimal success object to handleResponse.";
        handleResponse(endpoint, method, emptySuccessObj);
        return;
    }

    if (responseDoc.isNull()) {
        qDebug() << "Invalid JSON response (null or not an object/array) from server for" << endpoint << ":" << responseBodyStr.left(200);
        emit error(QString("Invalid JSON response from server for %1: %2").arg(endpoint, responseBodyStr.left(100)));
        return;
    }

    if (responseDoc.isObject()){
        handleResponse(endpoint, method, responseDoc.object());
    } else if (responseDoc.isArray()) {
        QJsonObject wrapperObj;
        wrapperObj["status"] = true;
        wrapperObj["data"] = responseDoc.array();
        handleResponse(endpoint, method, wrapperObj);
    } else {
        emit error(QString("Unhandled JSON type from server for %1").arg(endpoint));
    }
}


void RestApiClient::handleResponse(const QString &endpoint, const QString &method, const QJsonObject &jsonObj)
{
    qDebug() << "Processing response from endpoint:" << endpoint << "Method:" << method;
    qDebug() << "Parsed JSON response:" << QJsonDocument(jsonObj).toJson(QJsonDocument::Indented);

    bool success = jsonObj.value("status").toBool(true);
    if (endpoint == "attendance" && method == "GET" && jsonObj.contains("total_students")) {
        success = true;
    }
    if (endpoint == "classroom" && method == "DELETE" && jsonObj.value("message").toString() == "Classroom deleted successfully") {
        success = true;
    }

    if (!jsonObj.contains("status") && (endpoint.startsWith("qr/") || endpoint == "attendance" || (endpoint == "classroom" && method == "GET"))) {
        success = true;
    }

    if (!success) {
        QString message = jsonObj.value("message").toString("An unknown error occurred or bad status field.");
        qDebug() << "Request explicitly failed by server logic for endpoint" << endpoint << "Message:" << message;
        emit error(message);
        if(endpoint == "auth/login") emit loginFailed(message);
        if(endpoint == "auth/register") emit registerFailed(message);
        return;
    }

    QJsonValue dataValue = jsonObj.value("data");

    if (endpoint == "auth/login") {
        if (dataValue.isObject()) {
            QJsonObject dataObj = dataValue.toObject();
            m_authToken = dataObj.value("token").toString();
            qDebug() << "Auth token received and stored. Length:" << m_authToken.length();
            if (dataObj.value("user").isObject()) {
                emit loginSuccess(dataObj.value("user").toObject(), m_authToken);
            } else {
                emit error("Login response missing user data.");
                emit loginFailed("Login response missing user data.");
            }
        } else {
            emit error("Invalid data format in login response.");
            emit loginFailed("Invalid data format in login response.");
        }
    }
    else if (endpoint == "auth/register") {
        QJsonObject dataObj = dataValue.toObject();
        m_authToken = dataObj.value("token").toString();
        emit registerSuccess(dataObj);
    }
    else if (endpoint == "attendance") {
        qDebug() << "Received attendance response:" << jsonObj;
        if (dataValue.isObject()) {
            QJsonObject dataObj = dataValue.toObject();
            if (dataObj.contains("attendance_list")) {
                QJsonArray attendanceList = dataObj["attendance_list"].toArray();
                qDebug() << "Attendance list contains" << attendanceList.size() << "records";
                emit attendanceListReceived(attendanceList);
            } else {
                qDebug() << "ERROR: No attendance_list found in data object:" << dataObj;
                emit error("Invalid attendance list response format (no attendance_list found).");
            }
        } else {
            qDebug() << "ERROR: Invalid attendance response format (data is not an object):" << dataValue;
            emit error("Invalid attendance list response format (data is not an object).");
        }
    }
    else if (endpoint == "classroom" && method == "GET") {
        if (dataValue.isArray()) {
            QJsonArray sessionsArray = dataValue.toArray();
            emit sessionsReceived(sessionsArray);
            emit classroomListReceived(sessionsArray);

            QJsonObject activeSessionObj;
            for (const QJsonValue& val : sessionsArray) {
                QJsonObject session = val.toObject();
                if (session.value("status").toInt() == 1 || session.value("status").toBool() == true) {
                    activeSessionObj = session;
                    emit activeSessionReceived(activeSessionObj);
                    break;
                }
            }
        } else {
            emit error("Invalid data format for classroom list (expected data array).");
            emit sessionsReceived(QJsonArray());
            emit activeSessionReceived(QJsonObject());
        }
    }
    else if (endpoint == "classroom" && method == "POST") {
        if (dataValue.isObject()) {
            QJsonObject classroomData = dataValue.toObject();
            // Try both classroom_id and id fields
            qint64 classroomId = classroomData["classroom_id"].toVariant().toLongLong();
            qDebug() << classroomId << "bok";
            if (classroomId == 0) {
                classroomId = classroomData["id"].toVariant().toLongLong();
            }
            QString code = classroomData["code"].toString();
            
            if (classroomId > 0 && !code.isEmpty()) {
                emit classroomCreated(classroomId, code);

                // Automatically activate the created classroom
                QJsonObject activateData;
                activateData["id"] = classroomId;
                activateData["status"] = true;
                sendRequest("classroom", "PUT", activateData);

                // Also emit sessionCreated and activeSessionReceived to trigger UI updates
                QJsonObject sessionData;
                sessionData["id"] = classroomId;  // Use the correct ID field
                sessionData["code"] = code;
                sessionData["status"] = true; // Mark as active
                emit sessionCreated(sessionData);
                emit activeSessionReceived(sessionData);
            } else {
                emit error("Invalid classroom ID or code in server response");
            }
        } else {
            emit error("Invalid data format from server after creating classroom.");
        }
    }
    else if (endpoint == "classroom" && method == "PUT") {
        if (dataValue.isObject()) {
            QJsonObject respDataObj = dataValue.toObject();
            bool status = respDataObj.value("status").toBool();
            if (status) {
                // Session was successfully closed
                emit sessionClosed(QString::number(respDataObj.value("id").toInt(-1)));
                stopPollingRequests();
            } else {
                emit sessionActivated(respDataObj);
                emit activeSessionReceived(respDataObj);
            }
        } else {
            emit error("Invalid data format from server after updating classroom status.");
        }
    }
    else if (endpoint == "classroom" && method == "DELETE") {
        qDebug() << "Classroom deleted successfully.";
    }
    
    else if (endpoint == "attendance/code" && method == "POST") { // YENİ BLOK VEYA MEVCUT BLOK GÜNCELLENİR
        if (jsonObj.value("status").toBool(false)) { // Genellikle API yanıtınızda bir status alanı olur
            if (dataValue.isObject()) {
                QJsonObject dataObj = dataValue.toObject();
                QString code = dataObj.value("code").toString();
                QString expiresAt = dataObj.value("expires_at").toString();
                QString expiresInText = dataObj.value("expires_in").toString(); // API'den "1 minute" gibi gelebilir

                qDebug() << "[RestApiClient] Yoklama kodu başarıyla oluşturuldu (handleResponse):" << code;
                // attendanceCodeGenerated sinyalinin doğru parametrelerle çağrıldığından emin olun.
                // Eğer `qrCode` ve `expiry` yerine `expiresAt` ve `expiresInText` kullanıyorsanız:
                emit attendanceCodeGenerated(code, expiresAt, expiresInText);
            } else {
                qWarning() << "[RestApiClient] /attendance/code yanıtında 'data' nesnesi bulunamadı veya obje değil.";
                emit attendanceCodeGenerationFailed("Invalid data format in attendance code response.");
            }
        } else {
            QString message = jsonObj.value("message").toString("Unknown error during attendance code generation.");
            qWarning() << "[RestApiClient] /attendance/code oluşturma hatası (API - handleResponse):" << message;
            emit attendanceCodeGenerationFailed(message);
        }
    }
    
    // else if (endpoint == "attendance/code") {
    //     if (dataValue.isObject()) {
    //         QJsonObject dataObj = dataValue.toObject();
    //         emit attendanceCodeGenerated(
    //             dataObj["code"].toString(),
    //             dataObj["qr_code"].toString(),
    //             dataObj["expiry"].toString()
    //             );
    //     } else {
    //         emit error("Invalid data format for attendance code generation.");
    //     }
    // }
    
    
    else if (endpoint == "request" && method == "GET") {
        if (dataValue.isArray()) {
            QJsonArray requests = dataValue.toArray();
            emit requestsReceived(requests);
            for (const QJsonValue &val : requests) {
                QJsonObject request = val.toObject();
                QString requestId = request["id"].toString();
                if (requestId.isEmpty()) requestId = request["request_id"].toString();

                if (!m_processedRequestIds.contains(requestId) && !requestId.isEmpty()) {
                    m_processedRequestIds.append(requestId);
                    emit newSpeakRequest(
                        request["student_id"].toString(),
                        request["student_name"].toString(),
                        requestId
                        );
                }
            }
        } else {
            emit error("Invalid data format for list requests (expected data array).");
        }
    }
    else if (endpoint == "request" && method == "POST") {
        if (dataValue.isObject()) {
            emit requestCreated(dataValue.toObject());
        } else {
            emit error("Invalid data format from server after creating request.");
        }
    }
    else if (endpoint == "request" && method == "PUT") {
        if (dataValue.isObject()) {
            QJsonObject updatedRequestData = dataValue.toObject();
            QString reqId = updatedRequestData.value("id").toString();
            if (reqId.isEmpty()) reqId = updatedRequestData.value("request_id").toString();

            QString statusOrAction = updatedRequestData.value("status").toString();
            if (statusOrAction.isEmpty()) statusOrAction = updatedRequestData.value("action").toString();

            if (!reqId.isEmpty()) {
                emit requestUpdated(reqId, statusOrAction);
            } else {
                emit error("Request update response missing ID.");
            }
        } else {
            emit error("Invalid data format from server after updating request.");
        }
    }
    else if (endpoint.startsWith("qr/")) {
        qDebug() << "Processing QR code response";
        if (dataValue.isObject()) {
            QJsonObject qrData = dataValue.toObject();
            QString qrCode = qrData["qr_code"].toString();
            QString code = qrData["code"].toString();
            QString expiry = qrData["expiry"].toString();
            
            if (!qrCode.isEmpty()) {
                qDebug() << "Emitting qrCodeReceived signal with QR code data";
                emit qrCodeReceived(qrCode);
            } else {
                qDebug() << "QR code data is empty in response";
                emit error("QR code data is empty in server response");
            }
        } else {
            qDebug() << "Invalid QR code response format";
            emit error("Invalid QR code response format");
        }
    }
    else if (endpoint == "session/list") {
        if (dataValue.isArray()) {
            emit sessionsReceived(dataValue.toArray());
        } else {
            emit error("Invalid data for sessions list (expected array).");
        }
    }
    else if (endpoint == "session/active") {
        if (dataValue.isNull()) {
            emit activeSessionReceived(QJsonObject());
        } else if (dataValue.isObject()) {
            emit activeSessionReceived(dataValue.toObject());
        } else {
            emit error("Invalid active session data format.");
        }
    }
    else if (endpoint == "session/create") {
        if (dataValue.isObject()) {
            emit sessionCreated(dataValue.toObject());
        } else {
            emit error("Invalid data from server after creating session.");
        }
    }
    else if (endpoint.startsWith("session/activate/")) {
        if (dataValue.isObject()) {
            emit sessionActivated(dataValue.toObject());
        } else {
            emit error("Invalid data from server after activating session.");
        }
    }
    else if (endpoint.startsWith("session/close/")) {
        QString sessionIdFromEndpoint = endpoint.mid(QString("session/close/").length());
        QString sessionIdFromData;
        if(dataValue.isObject()) sessionIdFromData = QString::number(dataValue.toObject().value("id").toInt(-1));

        QString finalSessionId = !sessionIdFromData.isEmpty() && sessionIdFromData != "-1" ? sessionIdFromData : sessionIdFromEndpoint;

        if(!finalSessionId.isEmpty()){
            emit sessionClosed(finalSessionId);
            stopPollingRequests();
        } else {
            emit error("Could not determine session ID for closing.");
        }
    }
}

void RestApiClient::login(const QString &email, const QString &password)
{
    QJsonObject data;
    data["email"] = email;
    data["password"] = password;
    sendRequest("auth/login", "POST", data);
}

void RestApiClient::registerUser(const QString &name, const QString &email, const QString &password)
{
    QJsonObject data;
    data["name"] = name;
    data["email"] = email;
    data["password"] = password;
    data["role"] = "teacher";
    sendRequest("auth/register", "POST", data);
}

void RestApiClient::listClassrooms()
{
    sendRequest("classroom", "GET");
}

void RestApiClient::createClassroom(const QString &ip, int port)
{
    QJsonObject data;
    data["ip"] = ip;
    data["port"] = port;
    sendRequest("classroom", "POST", data);
}

void RestApiClient::updateClassroom(const QString &roomId, const QString &name, const QString &description)
{
    QJsonObject data;
    bool ok;
    int id = roomId.toInt(&ok);
    if (!ok) {
        emit error("Invalid classroom ID for update: " + roomId);
        return;
    }
    data["id"] = id; // Assuming API expects "id" for PUT /classroom
    if(!name.isNull()) data["name"] = name;
    if(!description.isNull()) data["description"] = description;
    sendRequest("classroom", "PUT", data);
}

void RestApiClient::deleteClassroom(const QString &roomId)
{
    QJsonObject data; // DELETE typically doesn't use a body, but if your API needs it for ID:
    bool ok;
    int id = roomId.toInt(&ok);
    if (!ok) {
        emit error("Invalid classroom ID for delete: " + roomId);
        return;
    }
    // If your API expects ID in URL for DELETE (e.g. /classroom/{id}), adjust sendRequest or endpointPath
    // sendRequest(QString("classroom/%1").arg(roomId), "DELETE");
    // If it expects ID in body (less common for DELETE):
    data["id"] = id;
    sendRequest("classroom", "DELETE", data); // Or QUrlQuery if it's a query param for DELETE
}

void RestApiClient::listAttendance(const QString &roomId)
{
    QUrlQuery query;
    query.addQueryItem("classroom_id", roomId);
    sendRequest("attendance", "GET", QJsonObject(), query);
}

void RestApiClient::markAttendance(const QString &roomId, const QString &code,
                                   const QString &studentId, const QString &studentName)
{
    QJsonObject data;
    data["room_id"] = roomId;
    data["code"] = code;
    data["student_id"] = studentId;
    data["student_name"] = studentName;
    sendRequest("attendance", "POST", data);
}

// void RestApiClient::generateAttendanceCode(const QString &roomId, int durationMinutes)
// {
//     QJsonObject data;
//     data["room_id"] = roomId;
//     data["duration_minutes"] = durationMinutes;
//     sendRequest("attendance/code", "POST", data);
// }

void RestApiClient::listRequests(const QString &roomId)
{
    QUrlQuery query;
    query.addQueryItem("classroom_id", roomId);
    sendRequest("request", "GET", QJsonObject(), query);
}

void RestApiClient::createRequest(const QString &roomId, const QString &studentId, const QString &studentName)
{
    QJsonObject data;
    data["classroom_id"] = roomId;
    data["student_id"] = studentId;
    data["student_name"] = studentName;
    sendRequest("request", "POST", data);
}

void RestApiClient::updateRequest(const QString &roomId, const QString &requestId, const QString &action)
{
    QJsonObject data;
    data["classroom_id"] = roomId;
    data["request_id"] = requestId;
    data["action"] = action;
    sendRequest("request", "PUT", data);
}

void RestApiClient::startPollingRequests(const QString &roomId, int intervalMs)
{
    m_currentRoomId = roomId;
    m_processedRequestIds.clear();
    m_pollTimer->start(intervalMs);
    pollRequests();
}

void RestApiClient::stopPollingRequests()
{
    m_pollTimer->stop();
}

void RestApiClient::pollRequests()
{
    if (!m_currentRoomId.isEmpty() && !m_currentRoomId.isNull()) {
        listRequests(m_currentRoomId);
    }
}

void RestApiClient::listSessions()
{
    sendRequest("classroom", "GET");
}

void RestApiClient::getActiveSession()
{
    sendRequest("classroom", "GET");
}

void RestApiClient::createSession(const QString &name, const QString &ip, int port)
{
    QJsonObject data;
    data["name"] = name;
    data["ip"] = ip;
    data["port"] = port;
    sendRequest("classroom", "POST", data);
}

void RestApiClient::activateSession(const QString &sessionId)
{
    bool ok;
    int sId = sessionId.toInt(&ok);
    if (!ok) {
        emit error("Invalid session ID for activation: " + sessionId);
        return;
    }
    QJsonObject data;
    data["id"] = sId;
    data["status"] = true;
    sendRequest("classroom", "PUT", data);
}

void RestApiClient::closeSession(const QString &sessionId)
{
    if (sessionId.isEmpty()) {
        qDebug() << "Cannot close session: empty session ID";
        return;
    }

    bool ok;
    int id = sessionId.toInt(&ok);
    if (!ok) {
        qDebug() << "Invalid session ID format:" << sessionId;
        return;
    }

    QJsonObject data;
    data["id"] = id;
    data["status"] = false;  // This will deactivate the session

    // First deactivate the session
    sendRequest("classroom", "PUT", data);
}

void RestApiClient::getQrCode(const QString &roomId)
{
    QString endpoint = QString("qr/%1").arg(roomId);
    sendRequest(endpoint, "GET");
}



// YALNIZCA BU VERSİYON KALMALI (sendRequest kullanan)
void RestApiClient::generateAttendanceCode(const QString &classroomId, int expiresInSeconds) {
    if (m_authToken.isEmpty()) {
        qWarning() << "[RestApiClient] Yetkilendirme tokeni yok. Yoklama kodu oluşturulamıyor.";
        emit attendanceCodeGenerationFailed("Authorization token is missing.");
        return;
    }

    QJsonObject jsonPayload;
    //jsonPayload["classroom_id"] = 48; // API int64 bekliyor
    jsonPayload["classroom_id"] = classroomId.toLongLong(); // API int64 bekliyor
    jsonPayload["expires_in"] = expiresInSeconds;

    qDebug() << "[RestApiClient] Sending POST request to /attendance/code via sendRequest";
    qDebug() << "[RestApiClient] Payload:" << QJsonDocument(jsonPayload).toJson(QJsonDocument::Compact);

    // sendRequest metodunu kullanarak isteği gönderin
    sendRequest("attendance/code", "POST", jsonPayload);
}
