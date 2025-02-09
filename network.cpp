#include "network.h"

#include <QTcpSocket>

#include "state.h"

SocketThread::SocketThread(QObject *parent, int descriptor)
	:  QThread(parent)
	, m_descriptor(descriptor)
{

}

SocketThread::~SocketThread()
{

}

void SocketThread::run()
{
	qDebug() << Q_FUNC_INFO;
	m_socket = new QTcpSocket;
	m_socket->setSocketDescriptor(m_descriptor);

	connect(m_socket, SIGNAL(readyRead()),	  this, SLOT(onReadyRead()), Qt::DirectConnection);
	connect(m_socket, SIGNAL(disconnected()), this, SLOT(onDisconnected()), Qt::DirectConnection);

	foreach(CtrlVal *c, State::values())
		connect(&(*c)(O_NET), SIGNAL(changedByOthers()), this, SLOT(onChanged));

	exec();
}

void SocketThread::onReadyRead()
{
	QString message = m_socket->readLine();
	qDebug() << message;

	m_socket->disconnectFromHost();
}

void SocketThread::onDisconnected()
{
	m_socket->close();
	// Exit event loop
	quit();
}

void SocketThread::onChanged()
{
}



NetworkServer::NetworkServer(QObject *parent)
	: QTcpServer(parent)
{
	if (this->listen(QHostAddress::Any, 2323)) {
		qDebug() << "Server start at port: " << this->serverPort();
	} else {

		qDebug() << "Start failure";
	}
}

void NetworkServer::incomingConnection(int handle)
{
	qDebug() << Q_FUNC_INFO << " new connection";
	SocketThread *socket = new SocketThread(this, handle);

	connect(socket, SIGNAL(finished()), socket, SLOT(deleteLater()));
	socket->start();
}






NetworkServer &network()
{
	static NetworkServer theNetwork;
	return theNetwork;
}

#if 0
	NetworkThread::NetworkThread(QObject *parent, int socketDescriptor)
	: QThread(parent)
	  , m_socketDescriptor(socketDescriptor)
{
}

NetworkThread::~NetworkThread()
{
}

void NetworkThread::run()
{
	QTcpSocket socket;
	if (!s.setSocketDescriptor(socketDescriptor)) {
		return;
	}

	_server = new QTcpServer();

	connect(&m_server, SIGNAL(newConnection()), this, SLOT(acceptConnection()));

	while (true) {

	}
}

	NetworkServer::NetworkServer()
: QTcpServer()
{
}

NetworkServer::~NetworkServer()
{

}

void NetworkServer::incomingConnection(int socketDescriptor)
{
	NetworkThread *thread = new NetworkThread(this, socketDescriptor);
	connect(this, SIGNAL(newData), thread, SLOT(sendData()));
	connect(thread, SIGNAL(finished()), this, SLOT(deleteLater()));
	thread->start();
};

NetworkServer &network()
{
	static NetworkServer theNetwork;
	return theNetwork;
}
#endif
