#ifndef __NETWORK_H__
#define __NETWORK_H__

#include <QThread>
#include <QObject>
#include <QTcpServer>

class NetworkServer: public QTcpServer
{
	public:
		NetworkServer(QObject *parent = 0);
		virtual ~NetworkServer() { };
		virtual void incomingConnection(int handle);
};

class SocketThread: public QThread
{
		Q_OBJECT
		QTcpSocket *m_socket;
		int m_descriptor;

		virtual void run();

	private slots:
		void onReadyRead();
		void onDisconnected();

		void onChanged();

	public:
		SocketThread(QObject *parent, int descriptor);
		virtual ~SocketThread();
};

NetworkServer &network();

#endif /* __NETWORK_H__ */
