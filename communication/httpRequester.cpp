#include "httpRequester.h"

HttpRequester* HttpRequester::s_instance = 0;

/** This function is called to create an instance of the class.
    Calling the constructor publicly is not allowed. The constructor
    is private and is only called by this Instance function.
*/
HttpRequester* HttpRequester::instance()
{
    if (!s_instance)   // Only allow one instance of class to be generated.
    {
        s_instance = new HttpRequester();
    }
    return s_instance;
}

/**
 * This method is called when an HTTP request returns. It emits a done signal to be used in MainWindow
 * @brief HttpRequester::on_requestFinished
 * @param reply The HTTP reply
 */
void HttpRequester::on_requestFinished(QNetworkReply* reply)
{
    emit done(reply);
}

/**
 * Send a ping request to the dta backend
 * This method is a SLOT that can be called from MainWindow
 * @brief HttpRequester::sendPingRequest
 */
void HttpRequester::sendPingRequest()
{
    QNetworkRequest req;
    req.setUrl(QUrl("http://"+ backendAddress +"/portal/api/ping"));
    networkManager->get(req);
}

void HttpRequester::sendPostRequest(QString urlPath, QJsonDocument jsonData)
{
    QNetworkRequest req;
    QByteArray postData = jsonData.toJson();
    QByteArray postDataSize = QByteArray::number(postData.size());
    req.setUrl(QUrl("http://"+ backendAddress + urlPath));
    req.setRawHeader("Content-type", "application/json");
    req.setRawHeader("Content-Length", postDataSize);
    networkManager->post(req, postData);
}

/**
 * Set the address of the data backend
 * @brief HttpRequester::setBackendAddress
 * @param address
 */
void HttpRequester::setBackendAddress(QString address)
{
    backendAddress = address;
}

